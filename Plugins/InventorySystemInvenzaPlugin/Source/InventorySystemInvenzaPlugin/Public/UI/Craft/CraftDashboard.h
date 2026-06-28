// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftDashboard.generated.h"

class UCraftControlPanel;
struct FQueuedRecipe;
class UQueueCraftList;
class UGenericProgress;
class UCraftMenuChoose;
class UCraftingComponent;
class UUIButton;
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

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UQueueCraftList> QueueCraftList;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UCraftControlPanel> CraftControlPanel;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION(BlueprintCallable)
	void InitializeCraftComponentBindings();

	UFUNCTION(BlueprintCallable)
	void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent);

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
	void HandleQueueOrderChangeRequested(FName RecipeID, const int32 QueueIndex, bool bMoveUp);

	UFUNCTION()
	void HandleQueueItemDeleteRequested(int32 QueueIndex);
};
