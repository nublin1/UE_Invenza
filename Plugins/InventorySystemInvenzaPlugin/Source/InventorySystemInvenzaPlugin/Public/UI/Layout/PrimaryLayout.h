//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "PrimaryLayout.generated.h"

class UUILayer;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UPrimaryLayout : public UBaseUserWidget
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
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBaseUserWidget> LayersDebugger;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UBaseUserWidget> GameMenuLayer_Obs;
};
