// Nublin Studio 2026 All Rights Reserved.

#include "UI/Game/GameLayer.h"

#include "Components/Border.h"

UGameLayer::UGameLayer()
{
}

void UGameLayer::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UGameLayer::NativeConstruct()
{
	Super::NativeConstruct();
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
