//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Layout/UILayer.h"

#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "UI/Layout/PrimaryLayout.h"

UUILayer::UUILayer()
{
}

void UUILayer::SetPrimaryLayout(UPrimaryLayout* Layout)
{
	PrimaryLayout = Layout;
}

TArray<UInvenzaBaseWidget*> UUILayer::GetStack() const
{
	TArray<UInvenzaBaseWidget*> Result;
	for (const auto& Entry : Stack)
	{
		auto* Widget = Entry.Get();
		if (IsValid(Widget))
		{
			Result.Add(Widget);
		}
	}
	return Result;
}

bool UUILayer::ContainsContent(const UInvenzaBaseWidget* Widget) const
{
	return IsValid(Widget) && Stack.Contains(Widget);
}

UUserWidget* UUILayer::Peek_Implementation()
{
	for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
	{
		if (IsValid(Stack[Index]))
		{
			return Stack[Index];
		}
	}
	return nullptr;
}

UInvenzaBaseWidget* UUILayer::PushContent_Implementation(const TSoftClassPtr<UUserWidget>& WidgetClass)
{
	if (bUpdatingStack || !MainBorder || WidgetClass.IsNull() || !GetOwningPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("UI layer %s cannot create content: invalid host, player, class or nested operation."), *GetName());
		return nullptr;
	}
	UClass* LoadedClass = WidgetClass.LoadSynchronous();
	if (!LoadedClass || !LoadedClass->IsChildOf(UInvenzaBaseWidget::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("UI layer %s requires an InvenzaBaseWidget content class."), *GetName());
		return nullptr;
	}
	auto* Widget = CreateWidget<UInvenzaBaseWidget>(GetOwningPlayer(), LoadedClass);
	return Widget && PushWidget(Widget) ? Widget : nullptr;
}

bool UUILayer::PushWidget(UInvenzaBaseWidget* Widget)
{
	if (bUpdatingStack || !IsValid(Widget) || !MainBorder || Widget == this ||
		Widget->GetOwningPlayer() != GetOwningPlayer() ||
		(Widget->GetOwningLayer() && Widget->GetOwningLayer() != this))
	{
		UE_LOG(LogTemp, Warning, TEXT("UI layer %s rejected a content instance or nested stack operation."), *GetName());
		return false;
	}
	if (Peek() == Widget)
	{
		return true;
	}
	TGuardValue<bool> StackGuard(bUpdatingStack, true);
	if (auto* Previous = Cast<UInvenzaBaseWidget>(Peek()))
	{
		CollapseTop();
		SuspendScreenInput(Previous);
	}
	Stack.Remove(Widget);
	Stack.RemoveAll([](const auto& Entry)
	{
		return !IsValid(Entry.Get());
	});
	Stack.Add(Widget);
	// Preserve the existing PushContent policy; only the empty Pop policy is extended.
	SetVisibility(ESlateVisibility::Visible);
	RefreshStackPresentation();
	OnStackChanged.Broadcast(this);
	return true;
}

void UUILayer::ShowTop_Implementation()
{
	if (auto* Widget = Cast<UInvenzaBaseWidget>(Peek()))
	{
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Widget->SetLayerScreenState(this, true);
		RestoreScreenInput(Widget);
	}
	NotifyDiagnosticsChanged();
}

void UUILayer::CollapseTop_Implementation()
{
	if (auto* Widget = Cast<UInvenzaBaseWidget>(Peek()))
	{
		Widget->SetLayerScreenState(this, false);
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
	NotifyDiagnosticsChanged();
}

void UUILayer::PopContent_Implementation()
{
	if (bUpdatingStack || Stack.IsEmpty())
	{
		return;
	}
	TGuardValue<bool> StackGuard(bUpdatingStack, true);
	RemoveContentInternal(Stack.Num() - 1);
	RefreshStackPresentation();
	OnStackChanged.Broadcast(this);
}

bool UUILayer::RemoveContent(UInvenzaBaseWidget* Widget)
{
	const int32 Index = Stack.IndexOfByKey(Widget);
	if (bUpdatingStack || Index == INDEX_NONE)
	{
		return false;
	}
	TGuardValue<bool> StackGuard(bUpdatingStack, true);
	const bool bWasTop = Index == Stack.Num() - 1;
	RemoveContentInternal(Index);
	if (bWasTop)
	{
		RefreshStackPresentation();
	}
	OnStackChanged.Broadcast(this);
	return true;
}

void UUILayer::ClearStack_Implementation()
{
	if (bUpdatingStack)
	{
		return;
	}
	TGuardValue<bool> StackGuard(bUpdatingStack, true);
	while (!Stack.IsEmpty())
	{
		RemoveContentInternal(Stack.Num() - 1);
	}
	RefreshStackPresentation();
	OnStackChanged.Broadcast(this);
}

void UUILayer::RemoveContentInternal(int32 Index)
{
	UInvenzaBaseWidget* Widget = Stack[Index];
	Stack.RemoveAt(Index);
	if (IsValid(Widget))
	{
		Widget->SetLayerScreenState(this, false);
		ReleaseScreenInput(Widget);
		SuspendedInputRequests.Remove(Widget);
		Widget->RemoveFromParent();
		Widget->SetLayerScreenState(nullptr, false);
		Widget->OnRemovedFromLayer();
		ReleaseScreenInput(Widget);
		OnContentRemoved.Broadcast(Widget);
	}
}

void UUILayer::RefreshStackPresentation()
{
	Stack.RemoveAll([](const auto& Entry)
	{
		return !IsValid(Entry.Get());
	});
	PushedWidget = Cast<UInvenzaBaseWidget>(Peek());
	if (!MainBorder)
	{
		return;
	}
	if (MainBorder->GetContent() != PushedWidget)
	{
		MainBorder->ClearChildren();
		if (PushedWidget)
		{
			auto* ContentSlot = Cast<UBorderSlot>(MainBorder->AddChild(PushedWidget));
			if (ContentSlot)
			{
				ContentSlot->SetHorizontalAlignment(DefaultHorizontalAlignment);
				ContentSlot->SetVerticalAlignment(DefaultVerticalAlignment);
			}
		}
	}
	if (PushedWidget)
	{
		ShowTop();
	}
	else if (bCollapseWhenStackIsEmpty)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

static void CollectUILayerScreenObjects(UWidget* Widget, TSet<UObject*>& Objects)
{
	if (!Widget || Objects.Contains(Widget))
	{
		return;
	}
	Objects.Add(Widget);
	auto* UserWidget = Cast<UUserWidget>(Widget);
	if (UserWidget && UserWidget->WidgetTree)
	{
		UserWidget->WidgetTree->ForEachWidget([&Objects](UWidget* Child)
		{
			CollectUILayerScreenObjects(Child, Objects);
		});
	}
}

static TSet<UObject*> GetUILayerScreenObjects(UInvenzaBaseWidget* Widget)
{
	TSet<UObject*> Objects;
	CollectUILayerScreenObjects(Widget, Objects);
	return Objects;
}

void UUILayer::ReleaseScreenInput(UInvenzaBaseWidget* Widget)
{
	ULocalPlayer* Player = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
	auto* Input = Player ? Player->GetSubsystem<UUIInputModeSubsystem>() : nullptr;
	if (!Input)
	{
		return;
	}
	const auto Objects = GetUILayerScreenObjects(Widget);
	for (UObject* Owner : Input->GetUIInputRequestOwners())
	{
		if (Objects.Contains(Owner))
		{
			Input->ReleaseUIInput(Owner);
		}
	}
}

void UUILayer::SuspendScreenInput(UInvenzaBaseWidget* Widget)
{
	ULocalPlayer* Player = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
	auto* Input = Player ? Player->GetSubsystem<UUIInputModeSubsystem>() : nullptr;
	if (!Input)
	{
		return;
	}
	const auto Objects = GetUILayerScreenObjects(Widget);
	auto& Requests = SuspendedInputRequests.FindOrAdd(Widget);
	for (UObject* Owner : Input->GetUIInputRequestOwners())
	{
		if (Objects.Contains(Owner))
		{
			Requests.AddUnique(Owner);
			Input->ReleaseUIInput(Owner);
		}
	}
}

void UUILayer::RestoreScreenInput(UInvenzaBaseWidget* Widget)
{
	TArray<TWeakObjectPtr<UObject>> Requests;
	if (!SuspendedInputRequests.RemoveAndCopyValue(Widget, Requests))
	{
		return;
	}
	ULocalPlayer* Player = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
	auto* Input = Player ? Player->GetSubsystem<UUIInputModeSubsystem>() : nullptr;
	if (Input)
	{
		for (const auto& Owner : Requests)
		{
			if (Owner.IsValid())
			{
				Input->RequestUIInput(Owner.Get());
			}
		}
	}
}

void UUILayer::NotifyDiagnosticsChanged() const
{
	if (PrimaryLayout.IsValid())
	{
		PrimaryLayout->NotifyLayoutChanged();
	}
	else if (auto* Layer = GetOwningLayer(); Layer && Layer != this)
	{
		Layer->NotifyDiagnosticsChanged();
	}
}

FReply UUILayer::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (MainBorder && MainBorder->IsHovered())
	{
		OnBackgroundClicked.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
