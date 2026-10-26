// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "GameHUD_Inz.generated.h"

class UInvenzaBaseWidget;
class UPrimaryLayout;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API AGameHUD_Inz : public AHUD
{
	GENERATED_BODY()

public:
	AGameHUD_Inz();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|GameHUD_Inz", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimaryLayout> PrimaryLayout;

	UPROPERTY(Category = "Invenza|GameHUD_Inz", EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, TSoftClassPtr<UInvenzaBaseWidget>> InitialScreens;
};
