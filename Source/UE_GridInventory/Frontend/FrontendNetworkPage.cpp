//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendNetworkPage.h"
#include "Frontend/InvenzaSessionsSubsystem.h"
#include "Frontend/FrontendMenuSettings.h"
#include "UI/Core/Buttons/UIButton.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void UFrontendNetworkPage::NativeConstruct()
{
	Super::NativeConstruct();
	Network = GetGameInstance() ? GetGameInstance()->GetSubsystem<UInvenzaSessionsSubsystem>() : nullptr;
	if (Network)
	{
		Network->OnChanged.AddUniqueDynamic(this, &ThisClass::RefreshNetwork);
	}
	for (auto* Button : {HostGame.Get(), ConnectIP.Get(), FindGames.Get(), CancelSearch.Get(), BackHome.Get()})
	{
		Button->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Clicked);
	}
	PlayerCount->ClearOptions();
	for (int32 Count = MinimumPlayers; Count <= MaximumPlayers; ++Count)
	{
		PlayerCount->AddOption(FString::FromInt(Count));
	}
	PlayerCount->SetSelectedOption(FString::FromInt(DefaultPlayers));
	Activate();
}

void UFrontendNetworkPage::NativeDestruct()
{
	if (Network)
	{
		Network->OnChanged.RemoveDynamic(this, &ThisClass::RefreshNetwork);
	}
	for (auto* Button : {HostGame.Get(), ConnectIP.Get(), FindGames.Get(), CancelSearch.Get(), BackHome.Get()})
	{
		if (Button)
		{
			Button->OnButtonClicked.RemoveDynamic(this, &ThisClass::Clicked);
		}
	}
	Super::NativeDestruct();
}

void UFrontendNetworkPage::Activate()
{
	RefreshNetwork();
}

void UFrontendNetworkPage::Clicked(UUIButton* Button)
{
	if (!Network)
	{
		return;
	}
	if (const auto* Index = ServerButtons.Find(Button))
	{
		Network->Join(*Index);
		return;
	}
	if (Button == HostGame)
	{
		Network->Host(ServerName->GetText().ToString(), FCString::Atoi(*PlayerCount->GetSelectedOption()));
	}
	else if (Button == FindGames)
	{
		Network->Find();
	}
	else if (Button == CancelSearch)
	{
		Network->CancelSearch();
	}
	else if (Button == ConnectIP)
	{
		Network->ConnectIP(IPAddress->GetText().ToString());
	}
	else if (Button == BackHome)
	{
		Network->CancelSearch();
		OnBackRequested.Broadcast();
	}
}

void UFrontendNetworkPage::RefreshNetwork()
{
	if (!Network || !NetworkStatus)
	{
		return;
	}
	const auto* Texts = GetDefault<UFrontendMenuSettings>();
	const bool bPlaying = GetOwningPlayer() && GetOwningPlayer()->GetPawn();
	NetworkStatus->SetText(bPlaying ? Texts->GetText(TEXT("NetworkWhilePlaying")) : FText::FromString(Network->Status));
	BackendLabel->SetText(Texts->GetText(Network->IsSteam() ? TEXT("SteamBackend") : TEXT("LANBackend")));
	for (auto* Button : {HostGame.Get(), FindGames.Get(), ConnectIP.Get()})
	{
		Button->SetIsEnabled(!bPlaying && !Network->bBusy);
	}
	ServerList->ClearChildren();
	ServerButtons.Reset();
	if (!ServerButtonClass || bPlaying)
	{
		return;
	}
	for (int32 Index = 0; Index < Network->Servers.Num(); ++Index)
	{
		const auto& Server = Network->Servers[Index];
		auto* Button = CreateWidget<UUIButton>(GetOwningPlayer(), ServerButtonClass);
		ServerList->AddChildToVerticalBox(Button);
		if (Button->MainLabel)
		{
			Button->MainLabel->SetText(
			    FText::Format(Texts->GetText(TEXT("ServerEntry")), FText::AsCultureInvariant(Server.Name),
			        FText::AsCultureInvariant(Server.Map), Server.Players, Server.Capacity));
		}
		Button->SetIsEnabled(!Network->bBusy && Server.Players < Server.Capacity);
		Button->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::Clicked);
		ServerButtons.Add(Button, Index);
	}
}
