// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/Crafting/CraftingComponent.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftMenuChoose.generated.h"

class UUIButton;
class UMovableTitleBar;
class UCraftRecipesList;
class UCraftMenuDetail;
class ULabelBaseText;
class UWidgetSwitcher;
class URecipeListEntryObject;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftMenuChoose : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftRequested, FItemRecipeRow, RecipeRow, int32, Amount, TArray<int>, Options );
#pragma endregion Delegates
	
public:
	UCraftMenuChoose();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable)
	FOnCraftRequested OnCraftRequested;
	
	// Widgets
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UMovableTitleBar> MovableTitleBar;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UCraftRecipesList> CraftRecipesList;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UCraftMenuDetail> CraftMenuDetail;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> EmptySelectionText;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	UCraftingComponent* GetCraftComponentPtr() {return CraftComponentPtr;}
	UFUNCTION()
	URecipeListEntryObject* GetSelectedObj() {return SelectedObj;}
	
	UFUNCTION(BlueprintCallable)
	void SetAvailableRecipes(const TArray<FItemRecipeRow>& Recipes);
	UFUNCTION(BlueprintCallable)
	void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "UI|Refs")
	TObjectPtr<UCraftingComponent> CraftComponentPtr;

	// Data
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI|Data")
	TObjectPtr<URecipeListEntryObject> SelectedObj = nullptr;
	UPROPERTY()
	float AmountToCraft = 0;
	UPROPERTY()
	TArray<int32> SelectedOptions;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	void HandleAvailableRecipesChanged();

	UFUNCTION()
	void RefreshCurrentSelectionDetails();
	
	UFUNCTION()
	void HandleItemSelectionChanged(UObject* Item);
	UFUNCTION()
	void HandleOnCraftAmountChanged(int32 NewAmount);

	UFUNCTION()
	void OnBtnClosePressed(UUIButton* Btn);

	UFUNCTION()
	void CraftBtnPressed(UUIButton* Btn);
};
