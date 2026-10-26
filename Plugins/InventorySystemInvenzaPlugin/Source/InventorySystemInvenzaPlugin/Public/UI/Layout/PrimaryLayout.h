//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "PrimaryLayout.generated.h"

class UUILayer;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UPrimaryLayout : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UPrimaryLayout();

public:
	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> GameLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> GameMenuLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> MenuLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> ModalLayer;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintImplementableEvent)
	UUserWidget* PushContentToLayer(FGameplayTag LayerName, const TSoftClassPtr<UUserWidget>& WidgetClass);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	TArray<FGameplayTag> InitialCollapsedScreens;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> LayersDebugger;

	UPROPERTY(Category = "Invenza|UI|Layout", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> GameMenuLayer_Obs;
};
