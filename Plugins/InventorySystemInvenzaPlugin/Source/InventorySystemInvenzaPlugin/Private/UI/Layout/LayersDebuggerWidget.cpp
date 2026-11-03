// Nublin Studio 2026 All Rights Reserved.

#include "UI/Layout/LayersDebuggerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/DynamicEntryBox.h"
#include "Components/PanelWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/Core/LabelBaseText.h"
#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/UILayer.h"

static UWidget* GetLayoutDebuggerParent(UWidget* Widget)
{
	if (!Widget)
	{
		return nullptr;
	}
	if (auto* Parent = Widget->GetParent())
	{
		return Parent;
	}
	if (auto* Tree = Cast<UWidgetTree>(Widget->GetOuter()))
	{
		return Tree->GetTypedOuter<UUserWidget>();
	}
	return nullptr;
}

static bool IsLayoutDebuggerWidgetShown(UWidget* Widget)
{
	TSet<UWidget*> Visited;
	for (auto* Current = Widget; Current; Current = GetLayoutDebuggerParent(Current))
	{
		if (Visited.Contains(Current) || !Current->IsVisible())
		{
			return false;
		}
		Visited.Add(Current);
	}
	return Widget != nullptr;
}

static FText GetLayerDebuggerScreenName(const UUserWidget* Widget)
{
	FString Name = Widget->GetClass()->GetName();
	Name.RemoveFromEnd(TEXT("_C"));
	return FText::FromString(Name);
}

void ULayersDebuggerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnNativeVisibilityChanged.RemoveAll(this);
	OnNativeVisibilityChanged.AddUObject(this, &ThisClass::VisibilityChanged);
	QueueRefresh();
}

void ULayersDebuggerWidget::NativeDestruct()
{
	SetLayout(nullptr);
	OnNativeVisibilityChanged.RemoveAll(this);
	Super::NativeDestruct();
}

void ULayersDebuggerWidget::SetLayout(UPrimaryLayout* Layout)
{
	if (PrimaryLayout.Get() != Layout)
	{
		if (PrimaryLayout.IsValid())
		{
			PrimaryLayout->OnLayoutChanged.RemoveDynamic(this, &ThisClass::QueueRefresh);
		}
		ClearObservedWidgets();
		PrimaryLayout = Layout;
		if (Layout)
		{
			Layout->OnLayoutChanged.AddUniqueDynamic(this, &ThisClass::QueueRefresh);
		}
	}
	if (!Layout)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
		}
		bRefreshQueued = false;
		return;
	}
	QueueRefresh();
}

void ULayersDebuggerWidget::QueueRefresh()
{
	if (bRefreshQueued || !PrimaryLayout.IsValid() || !GetWorld() || GetWorld()->bIsTearingDown ||
		!IsLayoutDebuggerWidgetShown(this))
	{
		return;
	}
	bRefreshQueued = true;
	RefreshTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshDiagnostics);
}

void ULayersDebuggerWidget::RefreshDiagnostics()
{
	bRefreshQueued = false;
	if (!PrimaryLayout.IsValid())
	{
		return;
	}
	ClearObservedWidgets();
	for (FGameplayTag Tag : PrimaryLayout->GetRegisteredLayerTags())
	{
		auto* Layer = PrimaryLayout->FindLayer(Tag);
		auto* Entries = Layer ? Cast<UDynamicEntryBox>(GetWidgetFromName(Layer->GetFName())) : nullptr;
		if (!Entries)
		{
			continue;
		}
		ObserveWidget(Layer);
		Entries->Reset();
		const auto Stack = Layer->GetStack();
		for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
		{
			auto* Screen = Stack[Index];
			ObserveWidget(Screen);
			if (auto* Entry = Entries->CreateEntry<ULabelBaseText>())
			{
				FLabelStyle Style = ResolveThemeStyle(TEXT("EntryStyle"), EntryStyle);
				Style.Text = GetLayerDebuggerScreenName(Screen);
				if (Screen == Layer->Peek() && Screen->IsLayerScreenActive() && IsLayoutDebuggerWidgetShown(Screen))
				{
					Style.Color = ResolveThemeSlateColor(TEXT("ActiveColor"), ActiveColor);
				}
				Entry->UpdateTextAndFont(Style);
			}
		}
	}
}

void ULayersDebuggerWidget::ObserveWidget(UUserWidget* Widget)
{
	TSet<UWidget*> Visited;
	for (UWidget* Current = Widget; IsValid(Current) && !Visited.Contains(Current);
		Current = GetLayoutDebuggerParent(Current))
	{
		Visited.Add(Current);
		if (auto* UserWidget = Cast<UUserWidget>(Current); UserWidget && UserWidget != this &&
			!ObservedWidgets.Contains(UserWidget))
		{
			ObservedWidgets.Add(UserWidget);
			UserWidget->OnNativeVisibilityChanged.AddUObject(this, &ThisClass::VisibilityChanged);
		}
	}
}

void ULayersDebuggerWidget::ClearObservedWidgets()
{
	for (const auto& Widget : ObservedWidgets)
	{
		if (Widget.IsValid())
		{
			Widget->OnNativeVisibilityChanged.RemoveAll(this);
		}
	}
	ObservedWidgets.Reset();
}

void ULayersDebuggerWidget::VisibilityChanged(ESlateVisibility NewVisibility)
{
	QueueRefresh();
}

FText ULayersDebuggerWidget::GetLayerDiagnosticText(FGameplayTag LayerTag) const
{
	auto* Layer = PrimaryLayout.IsValid() ? PrimaryLayout->FindLayer(LayerTag) : nullptr;
	TArray<FText> Names;
	if (Layer)
	{
		const auto Stack = Layer->GetStack();
		for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
		{
			Names.Add(GetLayerDebuggerScreenName(Stack[Index]));
		}
	}
	return FText::Join(FText::FromString(TEXT("\n")), Names);
}
