// Nublin Studio 2026 All Rights Reserved.

#include "UI/Game/GameLayer.h"

#include "ActorComponents/UIInventoryManager.h"
#include "Components/Border.h"
#include "Interface/Inventory/InvUIProvider.h"

UGameLayer::UGameLayer()
{
}

void UGameLayer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("OwnerPawn is null"));
		return;
	}

	UIInventoryManager* InvManager = OwnerPawn->FindComponentByClass<UIInventoryManager>();
	if (!InvManager)
	{
		UE_LOG(LogTemp, Error, TEXT("UIInventoryManager not found"));
		return;
	}

	InvManager->SetInteractionUIProvider(
		TScriptInterface<IInteractionUIProvider>(this)
	);
}

UInteractionWidget* UGameLayer::GetPawnInteractionWidget() const
{
	if (!MainBorder ||! InteractionPanel)
		return nullptr;
	
	for (int32 i = 0; i < InteractionPanel->GetChildrenCount(); i++)
	{
		if (auto* Widget = Cast<UInteractionWidget>(InteractionPanel->GetChildAt(i)))
		{
			return Widget;
		}
	}

	return nullptr;
}
