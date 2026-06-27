// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "QueueCraftListEntryWidget.generated.h"

class UCurrentMaxDisplay;
class UProductionQueueListEntryObject;
class UCraftingQuantitySelector;
class ULabelBaseText;
class UImageBaseWidget;
class UUIButton;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UQueueCraftListEntryWidget : public UInvenzaBaseWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQueueEntryMoveRequested, UObject*, Item, bool, bMoveUp);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueEntryDeleteRequested, UObject*, Item);
#pragma endregion

public:
	UQueueCraftListEntryWidget();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* DetailItemObject) override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//
	UPROPERTY(BlueprintAssignable, Category = "Queue")
	FOnQueueEntryMoveRequested OnMoveRequested;
	UPROPERTY(BlueprintAssignable, Category = "Queue")
	FOnQueueEntryDeleteRequested OnDeleteRequested;
	
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UImageBaseWidget> QueueIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> QueueItemName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_QueueUp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_QueueDown;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_QueueDelete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UCraftingQuantitySelector> CraftingQuantitySelectorMini;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UCurrentMaxDisplay> RemainingCount ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UCurrentMaxDisplay> CraftProgress ;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	// Data
	UPROPERTY()
	TObjectPtr<UProductionQueueListEntryObject> QueueListEntryRef;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	void HandleDataChanged();
	
	UFUNCTION()
	void OnBtnUpClicked(UUIButton* Btn);
	UFUNCTION()
	void OnBtnDownClicked(UUIButton* Btn);

	UFUNCTION()
	void OnBtnDeleteClicked(UUIButton* Btn);
};
