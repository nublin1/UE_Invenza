//  Nublin Studio 2025 All Rights Reserved.

#include "ActorComponents/Interactable/VendorComponent.h"

#include "ActorComponents/ItemCollection.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Container/InvBaseContainerWidget.h"

UVendorComponent::UVendorComponent()
{
}

void UVendorComponent::BeginFocus()
{
	Super::BeginFocus();
}

void UVendorComponent::EndFocus()
{
	Super::EndFocus();
}

void UVendorComponent::Interact(UInteractionComponent* InteractionComponent)
{
	Super::Interact(InteractionComponent);
}

void UVendorComponent::OnRegister()
{
	Super::OnRegister();
}

void UVendorComponent::InitializeInteractionComponent()
{
	Super::InitializeInteractionComponent();
	ItemCollection = GetOwner()->FindComponentByClass<UItemCollection>();
	UpdateInteractableData();
}

void UVendorComponent::UpdateInteractableData()
{
	Super::UpdateInteractableData();
	InteractableData.DefaultInteractableType = EInteractableType::Vendor;
	InteractableData.Action = FText::FromString(TEXT("Trade"));
	InteractableData.Quantity = -1;
}

UUInventoryWidgetBase* UVendorComponent::FindContainerWidget()
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UInvBaseContainerWidget::StaticClass(), false);

	for (UUserWidget* Widget : FoundWidgets)
	{
		if (Widget->GetFName() == ContainerWidgetName)
		{
			auto InvBaseContainerWidget = Cast<UInvBaseContainerWidget>(Widget);
			ContainerWidget = InvBaseContainerWidget;
			return InvBaseContainerWidget->GetInventoryFromContainerSlot();
		}
	}

	return nullptr;
}
