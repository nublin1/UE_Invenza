// Nublin Studio 2026 All Rights Reserved.

#include "InvenzaTakeAmountValidationLibrary.h"

#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/SceneComponent.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/SlotBasedInv/SlotbasedInventory.h"
#include "Data/Items/itemBase.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "Factory/ItemFactory.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Subsystems/ModalWindowManager.h"
#include "UI/Core/Modal/ModalDialogBase.h"
#include "UI/Core/Modal/InventoryTakeAmountHeaderWidget.h"
#include "UObject/Script.h"
#include "UObject/UnrealType.h"

static UIInventoryManager* FindTakeProbeManager(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() && ((World->GetNetMode() == NM_Client && PC->IsLocalController()) ||
			(World->GetNetMode() != NM_Client && !PC->IsLocalController())))
		{
			return PC->GetPawn()->FindComponentByClass<UIInventoryManager>();
		}
	}
	return nullptr;
}

UWidget* UInvenzaTakeAmountValidationLibrary::FindPreviewWidget(UUserWidget* Owner, FName Name)
{
	return Owner ? Owner->GetWidgetFromName(Name) : nullptr;
}

static UInventoryBase* FindTakeProbeBag(UIInventoryManager* Manager)
{
	const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), TEXT("MainPawnInventoryRef"));
	return Manager ? Cast<UInventoryBase>(Property->GetObjectPropertyValue_InContainer(Manager)) : nullptr;
}

static UInventoryBase* FindTakeProbeSource(UIInventoryManager* Manager)
{
	UInventoryCollectionComponent* Collection = Manager ? Manager->GetOwner()->FindComponentByClass<UInventoryCollectionComponent>() : nullptr;
	return Collection ? Collection->GetLinkedInventories().ExternalInventory.Get() : nullptr;
}

static int32 CountTakeProbeVines(UInventoryBase* Inventory, bool bFree)
{
	int32 Quantity = 0;
	if (Inventory && Inventory->GetItemCollectionLinked())
	{
		for (UObject* Item : Inventory->GetItemCollectionLinked()->GetAllItemsByContainer(Inventory->GetInventoryContainerID()))
		{
			if (IsValid(Item) && Item->Implements<UObjectDataProvider>() && IObjectDataProvider::Execute_GetItemID(Item) == TEXT("Vine"))
			{
				Quantity += bFree ? IObjectDataProvider::Execute_GetFreeAmount(Item) : IObjectDataProvider::Execute_GetQuantity(Item);
			}
		}
	}
	return Quantity;
}

static UModalDialogBase* FindTakeProbeDialog(UModalWindowManager* Modal)
{
	const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(UModalWindowManager::StaticClass(), TEXT("ModalLayoutRef"));
	UUILayer* Layer = Modal ? Cast<UUILayer>(Property->GetObjectPropertyValue_InContainer(Modal)) : nullptr;
	return Layer ? Cast<UModalDialogBase>(Layer->Peek()) : nullptr;
}

static bool PressTakeProbeAction(UModalDialogBase* Dialog, EObjectInteractionType Action)
{
	FMapProperty* Property = FindFProperty<FMapProperty>(UModalDialogBase::StaticClass(), TEXT("ButtonToResultMap"));
	if (!Dialog || !Property)
	{
		return false;
	}
	FScriptMapHelper Map(Property, Property->ContainerPtrToValuePtr<void>(Dialog));
	FObjectPropertyBase* KeyProperty = CastFieldChecked<FObjectPropertyBase>(Property->KeyProp);
	for (int32 Index = 0; Index < Map.GetMaxIndex(); ++Index)
	{
		if (!Map.IsValidIndex(Index))
		{
			continue;
		}
		const FModalResult* Result = reinterpret_cast<const FModalResult*>(Map.GetValuePtr(Index));
		UUIButton* Button = Cast<UUIButton>(KeyProperty->GetObjectPropertyValue(Map.GetKeyPtr(Index)));
		if (Result->ResultInteractionType == Action && Button && Button->MainButton)
		{
			Button->MainButton->OnPressed.Broadcast();
			return true;
		}
	}
	return false;
}

UModalDialogBase* UInvenzaTakeAmountValidationLibrary::OpenProbeContextMenu(UWorld* World, bool bPlayerInventory)
{
	UIInventoryManager* Manager = FindTakeProbeManager(World);
	UInventoryBase* Inventory = bPlayerInventory ? FindTakeProbeBag(Manager) : FindTakeProbeSource(Manager);
	APawn* Pawn = Manager ? Cast<APawn>(Manager->GetOwner()) : nullptr;
	APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	UModalWindowManager* Modal = Controller && Controller->GetLocalPlayer() ?
		Controller->GetLocalPlayer()->GetSubsystem<UModalWindowManager>() : nullptr;
	if (!Inventory || !Inventory->GetItemCollectionLinked() || !Modal)
	{
		return nullptr;
	}
	for (UObject* Item : Inventory->GetItemCollectionLinked()->GetAllItemsByContainer(Inventory->GetInventoryContainerID()))
	{
		FItemMapping* Mapping = Inventory->GetItemCollectionLinked()->FindItemMappingByContainerName(Item, Inventory->GetInventoryContainerID());
		if (!Mapping || Mapping->OccupiedSlots.IsEmpty())
		{
			continue;
		}
		TGuardValue<bool> RPCGuard(GAllowActorScriptExecutionInEditor, false);
		Modal->InitializeUI();
		Manager->ItemContextMenuRequest_Implementation(Inventory->GetInventoryContainerID(),
			Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid, Item);
		return FindTakeProbeDialog(Modal);
	}
	return nullptr;
}

FString UInvenzaTakeAmountValidationLibrary::GetContextMenuReport(UModalDialogBase* Dialog)
{
	FMapProperty* Property = FindFProperty<FMapProperty>(UModalDialogBase::StaticClass(), TEXT("ButtonToResultMap"));
	if (!Dialog || !Property)
	{
		return TEXT("{}");
	}
	FScriptMapHelper Map(Property, Property->ContainerPtrToValuePtr<void>(Dialog));
	FObjectPropertyBase* KeyProperty = CastFieldChecked<FObjectPropertyBase>(Property->KeyProp);
	FString Report = TEXT("{");
	for (int32 Index = 0; Index < Map.GetMaxIndex(); ++Index)
	{
		if (!Map.IsValidIndex(Index))
		{
			continue;
		}
		const FModalResult* Result = reinterpret_cast<const FModalResult*>(Map.GetValuePtr(Index));
		UUIButton* Button = Cast<UUIButton>(KeyProperty->GetObjectPropertyValue(Map.GetKeyPtr(Index)));
		if (Button && Button->MainLabel)
		{
			if (Report.Len() > 1)
			{
				Report += TEXT(",");
			}
			Report += FString::Printf(TEXT("\"%s\":{\"text\":\"%s\",\"visible\":%s}"),
				*StaticEnum<EObjectInteractionType>()->GetNameStringByValue(static_cast<int64>(Result->ResultInteractionType)),
				*Button->MainLabel->GetText().ToString().ReplaceCharWithEscapedChar(),
				Button->IsVisible() && Button->MainLabel->IsVisible() ? TEXT("true") : TEXT("false"));
		}
	}
	return Report + TEXT("}");
}

bool UInvenzaTakeAmountValidationLibrary::StartNetworkPreview()
{
	if (!GEditor || GEditor->PlayWorld)
	{
		return false;
	}
	ULevelEditorPlaySettings* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
	Settings->SetPlayNetMode(EPlayNetMode::PIE_ListenServer);
	Settings->SetPlayNumberOfClients(2);
	Settings->SetRunUnderOneProcess(true);
	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = Settings;
	Params.bAllowOnlineSubsystem = false;
	GEditor->RequestPlaySession(Params);
	return true;
}

bool UInvenzaTakeAmountValidationLibrary::PrepareNetworkProbe(UWorld* World)
{
	UIInventoryManager* Manager = FindTakeProbeManager(World);
	UInventoryBase* Bag = FindTakeProbeBag(Manager);
	if (!World || World->GetNetMode() == NM_Client || !Manager || !Bag)
	{
		return false;
	}
	if (FindTakeProbeSource(Manager))
	{
		return true;
	}
	UInventoryCollectionComponent* BagCollection = Bag->GetItemCollectionLinked();
	for (UObject* Existing : BagCollection->GetAllItemsByContainer(Bag->GetInventoryContainerID()))
	{
		BagCollection->RemoveItemFromAllContainers(Existing);
	}
	Bag->UpdateWeightInfo();
	Bag->UpdateMoneyInfo();
	Bag->OnInventoryRedrawRequested.Broadcast();
	AActor* Owner = World->SpawnActor<AActor>();
	Owner->SetReplicates(true);
	APawn* Pawn = CastChecked<APawn>(Manager->GetOwner());
	Owner->SetOwner(Pawn->GetController());
	USceneComponent* Root = NewObject<USceneComponent>(Owner);
	Owner->AddInstanceComponent(Root);
	Owner->SetRootComponent(Root);
	Root->RegisterComponent();
	Owner->SetActorLocation(Pawn->GetActorLocation());
	Owner->AttachToActor(Pawn, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	UInventoryCollectionComponent* Collection = NewObject<UInventoryCollectionComponent>(Owner);
	Collection->bPersistInventories = false;
	Owner->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = USlotbasedInventory::StaticClass();
	Startup.Settings.bCollectInvDataFromWidget = false;
	Startup.Settings.InventorySlotBasedSettings.bIgnoreItemSize = true;
	Startup.Settings.InventorySlotBasedSettings.InitNumberRows = 1;
	Startup.Settings.InventorySlotBasedSettings.InitNumColumns = 1;
	USlotbasedInventory* Inventory = CastChecked<USlotbasedInventory>(UInventoryBase::CreateInventoryAdvanced(Collection, Startup, Owner, Collection));
	Collection->AddPawnInventory_Internal(Inventory);
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/InventorySystemInvenzaPlugin/Data/DT_ItemData.DT_ItemData"));
	FDataTableRowHandle Handle;
	Handle.DataTable = Table;
	Handle.RowName = TEXT("Vine");
	UItemBase* Item = UItemFactory::CreateItemByHandle(Owner, Handle, 10);
	if (!Item)
	{
		return false;
	}
	FItemMapping Mapping;
	Mapping.InventoryID = Inventory->GetInventoryContainerID();
	Mapping.OccupiedSlots.Add(Inventory->GetInventorySlots()[0]);
	Collection->AddItem(Item, Mapping);
	Item->ReserveAmount_Implementation(Owner, 2);
	UInteractableComponent* Interaction = NewObject<UInteractableComponent>(Owner);
	Owner->AddInstanceComponent(Interaction);
	Interaction->RegisterComponent();
	Interaction->TryAcquireInteraction(Pawn);
	FindFProperty<FObjectPropertyBase>(UIInventoryManager::StaticClass(), TEXT("TrackedInteractionTarget"))->SetObjectPropertyValue_InContainer(Manager, Interaction);
	Pawn->FindComponentByClass<UInventoryCollectionComponent>()->SetExternalInventory(Inventory);
	Owner->ForceNetUpdate();
	Pawn->ForceNetUpdate();
	return true;
}

FString UInvenzaTakeAmountValidationLibrary::GetNetworkSnapshot(UWorld* World)
{
	UIInventoryManager* Manager = FindTakeProbeManager(World);
	UInventoryBase* Source = FindTakeProbeSource(Manager);
	UInventoryBase* Bag = FindTakeProbeBag(Manager);
	const bool bReady = Source && Source->GetItemCollectionLinked() && Bag && Bag->GetItemCollectionLinked();
	return FString::Printf(TEXT("{\"ready\":%s,\"netMode\":%d,\"source\":%d,\"available\":%d,\"bag\":%d,\"allowed\":%s}"),
		bReady ? TEXT("true") : TEXT("false"), World ? static_cast<int32>(World->GetNetMode()) : -1,
		CountTakeProbeVines(Source, false), CountTakeProbeVines(Source, true), CountTakeProbeVines(Bag, false),
		Manager && Manager->CanTakeAmountFromInventory(Source) ? TEXT("true") : TEXT("false"));
}

bool UInvenzaTakeAmountValidationLibrary::SendClientWithdrawal(UWorld* World, int32 Amount, bool bStaleSnapshot)
{
	UIInventoryManager* Manager = FindTakeProbeManager(World);
	UInventoryBase* Inventory = FindTakeProbeSource(Manager);
	if (!World || World->GetNetMode() != NM_Client || !Manager || !Inventory || !Inventory->GetItemCollectionLinked())
	{
		return false;
	}
	UInventoryCollectionComponent* Collection = Inventory->GetItemCollectionLinked();
	for (UObject* Item : Collection->GetAllItemsByContainer(Inventory->GetInventoryContainerID()))
	{
		if (!IsValid(Item) || !Item->Implements<UObjectDataProvider>() || IObjectDataProvider::Execute_GetItemID(Item) != TEXT("Vine"))
		{
			continue;
		}
		FItemMapping* Mapping = Collection->FindItemMappingByContainerName(Item, Inventory->GetInventoryContainerID());
		if (!Mapping || Mapping->OccupiedSlots.IsEmpty())
		{
			return false;
		}
		FInventoryTakeAmountRequest Request;
		Request.RequestID = FGuid::NewGuid();
		Request.Move.SourceInventory = Inventory;
		Request.Move.SourceItem = Item;
		Request.Move.SourceSlotID = Mapping->OccupiedSlots[0]->InventorySlotInfo.SlotGuid;
		Request.Move.RequestedAmount = Amount;
		Request.ExpectedSourceQuantity = IObjectDataProvider::Execute_GetQuantity(Item) + (bStaleSnapshot ? 1 : 0);
		Request.ExpectedAvailableAmount = IObjectDataProvider::Execute_GetFreeAmount(Item);
		// Python normally requests editor-local function execution. This probe must exercise the real owned RPC.
		TGuardValue<bool> RPCGuard(GAllowActorScriptExecutionInEditor, false);
		if (!bStaleSnapshot && Amount <= IObjectDataProvider::Execute_GetFreeAmount(Item))
		{
			APawn* Pawn = CastChecked<APawn>(Manager->GetOwner());
			APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
			UModalWindowManager* Modal = Controller && Controller->GetLocalPlayer() ?
				Controller->GetLocalPlayer()->GetSubsystem<UModalWindowManager>() : nullptr;
			if (!Modal)
			{
				return false;
			}
			Modal->InitializeUI();
			Manager->ItemContextMenuRequest_Implementation(Inventory->GetInventoryContainerID(), Request.Move.SourceSlotID, Item);
			if (!PressTakeProbeAction(FindTakeProbeDialog(Modal), EObjectInteractionType::TakeAmount))
			{
				return false;
			}
			UModalDialogBase* Dialog = FindTakeProbeDialog(Modal);
			// NamedSlot content is read through its panel API, without modifying the Blueprint.
			UInventoryTakeAmountHeaderWidget* Header = Dialog && Dialog->Upper_Slot ?
				Cast<UInventoryTakeAmountHeaderWidget>(Dialog->Upper_Slot->GetContent()) : nullptr;
			USpinBox* Input = Header ? Cast<USpinBox>(Header->GetWidgetFromName(TEXT("AmountInput"))) : nullptr;
			if (!Input)
			{
				return false;
			}
			Input->SetValue(static_cast<float>(Amount));
			Input->OnValueChanged.Broadcast(static_cast<float>(Amount));
			return PressTakeProbeAction(Dialog, EObjectInteractionType::Yes);
		}
		Manager->Server_TakeItemAmount(Request);
		return true;
	}
	return false;
}

void UInvenzaTakeAmountValidationLibrary::ReleaseProbeReservation(UWorld* World)
{
	UInventoryBase* Source = FindTakeProbeSource(FindTakeProbeManager(World));
	if (World && World->GetNetMode() != NM_Client && Source && Source->GetItemCollectionLinked())
	{
		for (UObject* Item : Source->GetItemCollectionLinked()->GetAllItemsByContainer(Source->GetInventoryContainerID()))
		{
			if (UItemBase* Base = Cast<UItemBase>(Item))
			{
				Base->ReleaseReservation_Implementation(Source->GetInventoryOwnerActor(), MAX_int32);
			}
		}
		Source->GetInventoryOwnerActor()->ForceNetUpdate();
	}
}
