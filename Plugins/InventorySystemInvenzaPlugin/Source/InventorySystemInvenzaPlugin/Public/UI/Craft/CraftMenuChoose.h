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

public:
	UCraftMenuChoose();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftRequested, FItemRecipeRow, RecipeRow, int32, Amount,
												   TArray<int>, Options);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(Category = "Invenza|Crafting", BlueprintAssignable)
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

	// Functions
	UFUNCTION()
	UCraftingComponent* GetCraftComponentPtr() { return CraftComponentPtr; }

	UFUNCTION()
	URecipeListEntryObject* GetSelectedObj() { return SelectedObj; }

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void SetAvailableRecipes(const TArray<FItemRecipeRow>& Recipes);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent);

protected:
	// Runtime
	// Properties and variables
	// Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "UI|Refs")
	TObjectPtr<UCraftingComponent> CraftComponentPtr;

	// Data
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI|Data")
	TObjectPtr<URecipeListEntryObject> SelectedObj = nullptr;

	UPROPERTY()
	int32 AmountToCraft = 0;

	UPROPERTY()
	TArray<int32> SelectedOptions;

	bool bRefreshingDetails = false;

	bool bRefreshMaximum = true;

	// Functions
	void ResetSelection();

	UFUNCTION()
	void HandleOptionsChanged();

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
