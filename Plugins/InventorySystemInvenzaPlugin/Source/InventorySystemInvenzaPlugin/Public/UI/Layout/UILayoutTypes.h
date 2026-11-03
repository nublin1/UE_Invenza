// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UILayoutTypes.generated.h"

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FUILayerRegistration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Layout")
	FGameplayTag LayerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Layout")
	FName WidgetName;
};

