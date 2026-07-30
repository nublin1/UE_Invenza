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
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UUILayer> GameLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UUILayer> GameMenuLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UUILayer> MenuLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UUILayer> ModalLayer;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent )
	UUserWidget* PushContentToLayer(FGameplayTag LayerName, const TSoftClassPtr<UUserWidget>& WidgetClass);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> LayersDebugger;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> GameMenuLayer_Obs;
};
