//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/InvenzaSessionsSubsystem.h"
#include "Subsystems/InvenzaSaveSubsystem.h"
#include "Frontend/InvenzaGameUserSettings.h"
#include "Frontend/FrontendMenuWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "TimerManager.h"
#include "Misc/ConfigCacheIni.h"

static const FName InvenzaSessionProjectKey(TEXT("INVENZA_PROJECT"));
static const FString InvenzaSessionProjectValue(TEXT("UE_Invenza_Frontend_v1"));
static const FName InvenzaSessionTitleKey(TEXT("INVENZA_TITLE"));
const TCHAR* UInvenzaSessionsSubsystem::MenuMap()
{
	return TEXT("/Game/Frontend/Maps/L_MainMenu");
}
const TCHAR* UInvenzaSessionsSubsystem::PlayMap()
{
	return TEXT("/InventorySystemInvenzaPlugin/Maps/Map_Ex");
}

void UInvenzaSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	NetworkHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::NetworkFailed);
	TravelHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::TravelFailed);
	MapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::Loaded);
	Status = TEXT("Готово к подключению");
}

void UInvenzaSessionsSubsystem::Deinitialize()
{
	ClearOperations();
	GEngine->OnNetworkFailure().Remove(NetworkHandle);
	GEngine->OnTravelFailure().Remove(TravelHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapHandle);
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		Sessions->DestroySession(NAME_GameSession);
	}
	Sessions.Reset();
	Super::Deinitialize();
}

bool UInvenzaSessionsSubsystem::IsSteam() const
{
	const IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	return OSS && OSS->GetSubsystemName() == FName(TEXT("STEAM"));
}

bool UInvenzaSessionsSubsystem::Ready()
{
	if (bBusy)
	{
		return false;
	}
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	// A requested Steam launch must not silently advertise a LAN game.
	FString Requested;
	GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), Requested, GEngineIni);
	if (Requested.Equals(TEXT("Steam"), ESearchCase::IgnoreCase) && !IsSteam())
	{
		SetStatus(TEXT("Steam недоступен. Запустите Steam и войдите в аккаунт, затем перезапустите игру."));
		return false;
	}
	Sessions = OSS ? OSS->GetSessionInterface() : nullptr;
	if (!Sessions.IsValid())
	{
		SetStatus(TEXT("Сетевой сервис недоступен."));
		return false;
	}
	FailureReason.Reset();
	return true;
}

void UInvenzaSessionsSubsystem::SetStatus(const FString& Message, bool Busy)
{
	Status = Message;
	bBusy = Busy;
	OnChanged.Broadcast();
}

void UInvenzaSessionsSubsystem::ArmTimeout()
{
	GetWorld()->GetTimerManager().SetTimer(Timeout, this, &ThisClass::TimedOut, 35.f);
}

void UInvenzaSessionsSubsystem::ClearOperations()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timeout);
	}
	if (Sessions.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);
	}
	Operation = EInvenzaSessionOperation::None;
}

void UInvenzaSessionsSubsystem::TimedOut()
{
	const EInvenzaSessionOperation Previous = Operation;
	ClearOperations();
	if (Sessions.IsValid())
	{
		if (Previous == EInvenzaSessionOperation::Find)
		{
			Sessions->CancelFindSessions();
		}
		else if (Sessions->GetNamedSession(NAME_GameSession))
		{
			Sessions->DestroySession(NAME_GameSession);
		}
	}
	SetStatus(TEXT("Время ожидания истекло. Попробуйте ещё раз."));
	if (Previous == EInvenzaSessionOperation::Destroy || Previous == EInvenzaSessionOperation::Travel)
	{
		ReturnToMenu();
	}
}

void UInvenzaSessionsSubsystem::Host(const FString& Name, int32 Capacity)
{
	if (!Ready())
	{
		return;
	}
	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		SetStatus(TEXT("Сначала завершите текущую сессию."));
		return;
	}
	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = !IsSteam();
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowJoinViaPresence = IsSteam();
	Settings.bUsesPresence = IsSteam();
	Settings.bUseLobbiesIfAvailable = IsSteam();
	Settings.NumPublicConnections = FMath::Clamp(Capacity, 2, 8);
	Settings.Set(
	    InvenzaSessionProjectKey, InvenzaSessionProjectValue, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	const FString Title = Name.TrimStartAndEnd().IsEmpty() ? TEXT("Invenza") : Name.TrimStartAndEnd().Left(48);
	Settings.Set(InvenzaSessionTitleKey, Title, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(SETTING_MAPNAME, FString(TEXT("Map_Ex")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Operation = EInvenzaSessionOperation::Create;
	CreateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
	    FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::Created));
	SetStatus(TEXT("Создание игры…"), true);
	ArmTimeout();
	if (!Sessions->CreateSession(0, NAME_GameSession, Settings))
	{
		Created(NAME_GameSession, false);
	}
}

void UInvenzaSessionsSubsystem::Created(FName, bool Success)
{
	ClearOperations();
	if (!Success)
	{
		SetStatus(TEXT("Не удалось создать игру."));
		return;
	}
	Operation = EInvenzaSessionOperation::Travel;
	SetStatus(TEXT("Загрузка игровой карты…"), true);
	ArmTimeout();
	UGameplayStatics::OpenLevel(this, FName(PlayMap()), true, TEXT("listen"));
}

void UInvenzaSessionsSubsystem::Find()
{
	if (!Ready())
	{
		return;
	}
	Results.Reset();
	Servers.Reset();
	Search = MakeShared<FOnlineSessionSearch>();
	Search->bIsLanQuery = !IsSteam();
	Search->MaxSearchResults = 1000;
	Search->TimeoutInSeconds = 15.f;
	if (IsSteam())
	{
		Search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		Search->QuerySettings.Set(InvenzaSessionProjectKey, InvenzaSessionProjectValue, EOnlineComparisonOp::Equals);
	}
	Operation = EInvenzaSessionOperation::Find;
	FindHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
	    FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::Found));
	SetStatus(TEXT("Поиск игр…"), true);
	ArmTimeout();
	if (!Sessions->FindSessions(0, Search.ToSharedRef()))
	{
		Found(false);
	}
}

void UInvenzaSessionsSubsystem::Found(bool Success)
{
	ClearOperations();
	if (!Success)
	{
		SetStatus(TEXT("Поиск не удался. Проверьте соединение."));
		return;
	}
	for (const auto& Result : Search->SearchResults)
	{
		FString Project;
		if (!Result.IsValid() || !Result.Session.SessionSettings.Get(InvenzaSessionProjectKey, Project) ||
		    Project != InvenzaSessionProjectValue)
		{
			continue;
		}
		FInvenzaServerInfo Info;
		Result.Session.SessionSettings.Get(InvenzaSessionTitleKey, Info.Name);
		Result.Session.SessionSettings.Get(SETTING_MAPNAME, Info.Map);
		Info.Capacity = Result.Session.SessionSettings.NumPublicConnections;
		Info.Players = Info.Capacity - Result.Session.NumOpenPublicConnections;
		Results.Add(Result);
		Servers.Add(Info);
	}
	SetStatus(Servers.IsEmpty() ? TEXT("Игры не найдены. Создайте игру во втором окне или на другом компьютере.")
	                            : FString::Printf(TEXT("Найдено игр: %d"), Servers.Num()));
}

void UInvenzaSessionsSubsystem::CancelSearch()
{
	if (Operation != EInvenzaSessionOperation::Find)
	{
		return;
	}
	ClearOperations();
	if (Sessions.IsValid())
	{
		Sessions->CancelFindSessions();
	}
	Search.Reset();
	SetStatus(TEXT("Поиск отменён."));
}

void UInvenzaSessionsSubsystem::Join(int32 Index)
{
	if (!Ready() || !Results.IsValidIndex(Index))
	{
		return;
	}
	if (Sessions->GetNamedSession(NAME_GameSession))
	{
		SetStatus(TEXT("Сначала завершите текущую сессию."));
		return;
	}
	if (Results[Index].Session.NumOpenPublicConnections <= 0)
	{
		SetStatus(TEXT("В игре нет свободных мест. Обновите список."));
		return;
	}
	Operation = EInvenzaSessionOperation::Join;
	JoinHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
	    FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::Joined));
	SetStatus(TEXT("Подключение…"), true);
	ArmTimeout();
	if (!Sessions->JoinSession(0, NAME_GameSession, Results[Index]))
	{
		Joined(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UInvenzaSessionsSubsystem::Joined(FName, EOnJoinSessionCompleteResult::Type Result)
{
	ClearOperations();
	FString Address;
	if (Result != EOnJoinSessionCompleteResult::Success ||
	    !Sessions->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (Sessions->GetNamedSession(NAME_GameSession))
		{
			Sessions->DestroySession(NAME_GameSession);
		}
		SetStatus(TEXT("Подключение не удалось. Игра могла закрыться или заполниться."));
		return;
	}
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC)
	{
		SetStatus(TEXT("Локальный игрок недоступен."));
		return;
	}
	Operation = EInvenzaSessionOperation::Travel;
	SetStatus(TEXT("Загрузка игры…"), true);
	ArmTimeout();
	PC->ClientTravel(Address, TRAVEL_Absolute);
}

void UInvenzaSessionsSubsystem::ConnectIP(const FString& Address)
{
	if (!Ready())
	{
		return;
	}
	if (IsSteam())
	{
		SetStatus(TEXT("Подключение по IP доступно в профиле LAN. В Steam выберите игру из списка."));
		return;
	}
	const FString Target = Address.TrimStartAndEnd();
	FString Host, Port;
	Target.Split(TEXT(":"), &Host, &Port);
	if (Host.IsEmpty())
	{
		Host = Target;
	}
	TArray<FString> Parts;
	Host.ParseIntoArray(Parts, TEXT("."), false);
	bool Valid = Parts.Num() == 4;
	for (const FString& Part : Parts)
	{
		Valid &= !Part.IsEmpty() && Part.Len() <= 3 && FCString::Atoi(*Part) <= 255;
		for (TCHAR C : Part)
		{
			Valid &= C >= TEXT('0') && C <= TEXT('9');
		}
	}
	if (Target.Contains(TEXT(":")))
	{
		Valid &= !Port.IsEmpty() && Port.Len() <= 5 && FCString::Atoi(*Port) > 0 && FCString::Atoi(*Port) <= 65535;
		for (TCHAR C : Port)
		{
			Valid &= C >= TEXT('0') && C <= TEXT('9');
		}
	}
	if (!Valid)
	{
		SetStatus(TEXT("Введите IPv4-адрес, например 127.0.0.1:7777."));
		return;
	}
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		Operation = EInvenzaSessionOperation::Travel;
		SetStatus(TEXT("Подключение по IP…"), true);
		ArmTimeout();
		PC->ClientTravel(Target, TRAVEL_Absolute);
	}
}

void UInvenzaSessionsSubsystem::Leave()
{
	if (!GetGameInstance()->GetSubsystem<UInvenzaSaveSubsystem>()->SaveBeforeExit())
	{
		return;
	}
	if (Operation == EInvenzaSessionOperation::Destroy)
	{
		return;
	}
	const bool WasSearching = Operation == EInvenzaSessionOperation::Find;
	ClearOperations();
	if (Sessions.IsValid() && WasSearching)
	{
		Sessions->CancelFindSessions();
	}
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession))
	{
		Operation = EInvenzaSessionOperation::Destroy;
		DestroyHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
		    FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::Destroyed));
		SetStatus(TEXT("Выход из игры…"), true);
		ArmTimeout();
		if (!Sessions->DestroySession(NAME_GameSession))
		{
			Destroyed(NAME_GameSession, false);
		}
	}
	else
	{
		SetStatus(FailureReason.IsEmpty() ? TEXT("Сессия завершена.") : FailureReason);
		ReturnToMenu();
	}
}

void UInvenzaSessionsSubsystem::Destroyed(FName, bool Success)
{
	ClearOperations();
	SetStatus(!FailureReason.IsEmpty() ? FailureReason
	        : Success                  ? TEXT("Сессия завершена.")
	                                   : TEXT("Сессия закрыта локально. Сервис не подтвердил завершение."));
	ReturnToMenu();
}

void UInvenzaSessionsSubsystem::ReturnToMenu()
{
	UGameplayStatics::OpenLevel(this, FName(MenuMap()));
}

void UInvenzaSessionsSubsystem::Loaded(UWorld* World)
{
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}
	ClearOperations();
	bBusy = false;
	if (auto* Settings = Cast<UInvenzaGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		Settings->ApplyAudio(World);
	}
	OnChanged.Broadcast();
}

void UInvenzaSessionsSubsystem::NetworkFailed(UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString& Message)
{
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}
	FailureReason = TEXT("Сетевая ошибка: ") + Message;
	Leave();
}

void UInvenzaSessionsSubsystem::TravelFailed(UWorld* World, ETravelFailure::Type, const FString& Message)
{
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}
	FailureReason = TEXT("Ошибка загрузки карты: ") + Message;
	Leave();
}
