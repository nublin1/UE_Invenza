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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UGenericProgress> CraftProgressSimple;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UQueueCraftList> QueueCraftList;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
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
	void UpdateCurrentCraftProgress(float NewValue);

	UFUNCTION()
	void UpdateQueueCraftList(TArray<FQueuedRecipe>& NewRecipeQueue);
};
