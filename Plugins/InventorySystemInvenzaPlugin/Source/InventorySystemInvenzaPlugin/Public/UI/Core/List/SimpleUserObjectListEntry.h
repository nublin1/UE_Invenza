// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "SimpleUserObjectListEntry.generated.h"

class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API USimpleUserObjectListEntry : public UInvenzaBaseWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
		
public:
	USimpleUserObjectListEntry() {}
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> ListEntry_Image;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> ListEntry_Text;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	void UpdateImage(const TSoftObjectPtr<UTexture2D>& NewIcon);
	UFUNCTION()
	void UpdateText(const FText& Text);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
};
