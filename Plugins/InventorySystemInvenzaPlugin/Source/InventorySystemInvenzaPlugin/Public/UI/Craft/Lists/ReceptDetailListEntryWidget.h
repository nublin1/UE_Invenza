// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "ReceptDetailListEntryWidget.generated.h"

class UCurrentMaxDisplay;
class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UReceptDetailListEntryWidget : public UInvenzaBaseWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

		
public:
	UReceptDetailListEntryWidget();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* DetailItemObject) override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> IngredientIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> RequiredItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<UCurrentMaxDisplay> RemainingCounter;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
