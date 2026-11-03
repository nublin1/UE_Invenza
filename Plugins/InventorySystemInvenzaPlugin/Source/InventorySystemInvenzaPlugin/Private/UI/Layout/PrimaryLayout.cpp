//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/LayersDebuggerWidget.h"
#include "UI/Layout/UILayer.h"

UPrimaryLayout::UPrimaryLayout()
{
}

void UPrimaryLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	for (const auto& Definition : LayerDefinitions)
	{
		RegisterLayer(Definition.LayerTag, Cast<UUILayer>(GetWidgetFromName(Definition.WidgetName)));
	}
	if (WBP_LayersDebugger)
	{
		LayersDebugger = WBP_LayersDebugger;
		WBP_LayersDebugger->SetLayout(this);
	}
}

void UPrimaryLayout::NativeConstruct()
{
	Super::NativeConstruct();
	bDestroyingLayout = false;
	if (RegisteredLayers.IsEmpty())
	{
		for (const auto& Definition : LayerDefinitions)
		{
			RegisterLayer(Definition.LayerTag, Cast<UUILayer>(GetWidgetFromName(Definition.WidgetName)));
		}
	}
	if (WBP_LayersDebugger)
	{
		WBP_LayersDebugger->SetLayout(this);
	}
	PushInitialScreens();
	NotifyLayoutChanged();
}

void UPrimaryLayout::NativeDestruct()
{
	bDestroyingLayout = true;
	if (WBP_LayersDebugger)
	{
		WBP_LayersDebugger->SetLayout(nullptr);
	}
	const auto Tags = RegisteredLayerOrder;
	for (FGameplayTag Tag : Tags)
	{
		if (auto* Layer = FindLayer(Tag))
		{
			Layer->ClearStack();
			Layer->OnStackChanged.RemoveDynamic(this, &ThisClass::LayerStackChanged);
			Layer->SetPrimaryLayout(nullptr);
		}
	}
	RegisteredLayers.Reset();
	RegisteredLayerOrder.Reset();
	bPushedInitialScreens = false;
	Super::NativeDestruct();
}

bool UPrimaryLayout::RegisterLayer(FGameplayTag LayerName, UUILayer* LayerWidget)
{
	if (!LayerName.IsValid() || !IsValid(LayerWidget) || RegisteredLayers.Contains(LayerName) ||
		RegisteredLayers.FindKey(LayerWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Primary layout %s rejected layer registration %s: invalid or duplicate tag/instance."),
			*GetName(), *LayerName.ToString());
		return false;
	}
	RegisteredLayers.Add(LayerName, LayerWidget);
	RegisteredLayerOrder.Add(LayerName);
	LayerWidget->SetPrimaryLayout(this);
	LayerWidget->OnStackChanged.AddUniqueDynamic(this, &ThisClass::LayerStackChanged);
	NotifyLayoutChanged();
	return true;
}

bool UPrimaryLayout::UnRegisterLayer(FGameplayTag LayerName)
{
	UUILayer* Layer = FindLayer(LayerName);
	if (!Layer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot unregister unknown layer %s."), *GetName(), *LayerName.ToString());
		return false;
	}
	Layer->ClearStack();
	Layer->OnStackChanged.RemoveDynamic(this, &ThisClass::LayerStackChanged);
	Layer->SetPrimaryLayout(nullptr);
	RegisteredLayers.Remove(LayerName);
	RegisteredLayerOrder.Remove(LayerName);
	Layer->RemoveFromParent();
	NotifyLayoutChanged();
	return true;
}

UUILayer* UPrimaryLayout::FindLayer(FGameplayTag LayerName) const
{
	const auto* Found = RegisteredLayers.Find(LayerName);
	return Found && IsValid(Found->Get()) ? Found->Get() : nullptr;
}

TArray<FGameplayTag> UPrimaryLayout::GetRegisteredLayerTags() const
{
	return RegisteredLayerOrder;
}

FGameplayTag UPrimaryLayout::GetLayerTag(const UUILayer* Layer) const
{
	for (const auto& Pair : RegisteredLayers)
	{
		if (Pair.Value == Layer)
		{
			return Pair.Key;
		}
	}
	return FGameplayTag();
}

UUserWidget* UPrimaryLayout::PushContentToLayer_Implementation(FGameplayTag LayerName,
	const TSoftClassPtr<UUserWidget>& WidgetClass)
{
	if (auto* Layer = FindLayer(LayerName))
	{
		return Layer->PushContent(WidgetClass);
	}
	UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot push content to unknown layer %s."), *GetName(), *LayerName.ToString());
	return nullptr;
}

void UPrimaryLayout::ClearLayer(FGameplayTag LayerName)
{
	if (auto* Layer = FindLayer(LayerName))
	{
		Layer->ClearStack();
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot clear unknown layer %s."), *GetName(), *LayerName.ToString());
}

void UPrimaryLayout::ClearAllLayers()
{
	const auto Tags = RegisteredLayerOrder;
	for (FGameplayTag Tag : Tags)
	{
		ClearLayer(Tag);
	}
}

void UPrimaryLayout::ClearAllLayersExcept(FGameplayTag ExceptionLayerName)
{
	if (!FindLayer(ExceptionLayerName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot preserve unknown layer %s; no layers were cleared."),
			*GetName(), *ExceptionLayerName.ToString());
		return;
	}
	const auto Tags = RegisteredLayerOrder;
	for (FGameplayTag Tag : Tags)
	{
		if (Tag != ExceptionLayerName)
		{
			ClearLayer(Tag);
		}
	}
}

void UPrimaryLayout::PopContentFromLayer(FGameplayTag LayerName)
{
	if (auto* Layer = FindLayer(LayerName))
	{
		Layer->PopContent();
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot pop unknown layer %s."), *GetName(), *LayerName.ToString());
}

void UPrimaryLayout::ChangeLayerVisbility(FGameplayTag LayerTag, ESlateVisibility State)
{
	if (auto* Layer = FindLayer(LayerTag))
	{
		Layer->SetVisibility(State);
		NotifyLayoutChanged();
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Primary layout %s cannot change unknown layer %s."), *GetName(), *LayerTag.ToString());
}

void UPrimaryLayout::PushInitialScreens()
{
	if (bPushedInitialScreens)
	{
		return;
	}
	bPushedInitialScreens = true;
	for (FGameplayTag Tag : RegisteredLayerOrder)
	{
		if (const auto* WidgetClass = InitialScreens.Find(Tag))
		{
			PushContentToLayer(Tag, *WidgetClass);
		}
	}
	for (const auto& Pair : InitialScreens)
	{
		if (!FindLayer(Pair.Key))
		{
			UE_LOG(LogTemp, Warning, TEXT("Primary layout %s has an initial screen for unknown layer %s."), *GetName(), *Pair.Key.ToString());
		}
	}
	for (FGameplayTag Tag : InitialCollapsedScreens)
	{
		ChangeLayerVisbility(Tag, ESlateVisibility::Collapsed);
	}
}

void UPrimaryLayout::LayerStackChanged(UUILayer* Layer)
{
	NotifyLayoutChanged();
}

void UPrimaryLayout::NotifyLayoutChanged()
{
	if (!bDestroyingLayout)
	{
		OnLayoutChanged.Broadcast();
	}
}
