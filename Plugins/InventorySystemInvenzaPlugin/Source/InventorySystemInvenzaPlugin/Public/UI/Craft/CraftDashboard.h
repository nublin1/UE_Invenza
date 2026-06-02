// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftDashboard.generated.h"

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
	TObjectPtr<UUIButton> Btn_AddTask;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void BindCraftMenu(UCraftMenuChoose* NewCraftMenuChooseRef);

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Refs")
	TObjectPtr<UCraftMenuChoose> CraftMenuRef;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void AddTaskBtnPressed(UUIButton* Btn);

	UFUNCTION()
	void SetNewCurrentCraftRecipe(FQueuedRecipe NewQueuedRecipe);
	UFUNCTION()
	void UpdateCurrentCraftProgress(FQueuedRecipe& Recipe);

	UFUNCTION()
	void UpdateQueueCraftList(TArray<FQueuedRecipe>& NewRecipeQueue);

	UFUNCTION()
	void HandleQueueOrderChangeRequested(FName RecipeID, bool bMoveUp);
};
