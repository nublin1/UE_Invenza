// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "CraftDashboard.generated.h"

class UCraftControlPanel;
struct FQueuedRecipe;
class UQueueCraftList;
class UGenericProgress;
class UCraftMenuChoose;
class UCraftingComponent;
class UUIButton;
class UPanelWidget;
class ULabelBaseText;
class USimpleUserObjectListEntry;
struct FBlockReasonData;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftDashboard : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UCraftDashboard();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UQueueCraftList> QueueCraftList;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UCraftControlPanel> CraftControlPanel;

	/** Rows are populated from the crafting component's block-change event. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> BlockReasonsPanel;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BlockReasonsSection;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> EmptyQueueLabel;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> InputSlot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> FuelSlot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> OutputSlot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> InteractorSlot;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION(BlueprintCallable)
	void InitializeCraftComponentBindings();

	UFUNCTION(BlueprintCallable)
	void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent);
	
	UFUNCTION(BlueprintCallable)
	void SetInventoryWidgets(UInventoryContainerWidget* InputWidget, UInventoryContainerWidget* FuelWidget, UInventoryContainerWidget* OutputWidget);
	
	UFUNCTION(BlueprintCallable)
	void SetInteractorWidget(UInventoryContainerWidget* InteractorWidget);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "UI|Refs")
	TObjectPtr<UCraftingComponent> CraftComponentPtr;
	
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|Config")
	FGameplayTag AddTaskBtnTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI|Config")
	FGameplayTag PauseBtnTag;

	/** Uses the Core list-entry widget, including its Core label and image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Config")
	TSubclassOf<USimpleUserObjectListEntry> BlockReasonWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Config")
	FText PauseButtonText = NSLOCTEXT("InvenzaCraft", "Pause", "Pause");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Config")
	FText ResumeButtonText = NSLOCTEXT("InvenzaCraft", "Resume", "Resume");
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void AddTaskBtnPressed(UUIButton* Btn);
	
	UFUNCTION()
	void PauseBtnPressed(UUIButton* Btn);

	UFUNCTION()
	void UpdateCurrentCraftProgress(const FQueuedRecipe& Recipe);

	UFUNCTION()
	void UpdateQueueCraftList(const TArray<FQueuedRecipe>& NewRecipeQueue);

	UFUNCTION()
	void HandleBlocksUpdated(TArray<FBlockReasonData> Blocks);

	void UnbindCraftComponent();
	void RefreshCraftState();

	UFUNCTION()
	void HandleQueueOrderChangeRequested(FName RecipeID, const int32 QueueIndex, bool bMoveUp);

	UFUNCTION()
	void HandleQueueItemDeleteRequested(int32 QueueIndex);
};
