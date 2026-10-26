// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "ReceptDetailRequiredListSimple.generated.h"

class URecipeRequiredIListEntryObject;
struct FRecipeItemRequirementCheck;
struct FItemRecipeRow;
class UListView;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UReceptDetailRequiredListSimple : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UReceptDetailRequiredListSimple();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOptionsChanged);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnOptionsChanged OnOptionsChanged;

	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> RequiredList;

	// Functions
	void ClearRequirements();

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void RefreshRequiredList(const FItemRecipeRow& RecipeRow, const TArray<FRecipeItemRequirementCheck>& Requirements);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	void UpdateRequirementsCheck(const FItemRecipeRow& UpdateRecipeRow,
								 const TArray<FRecipeItemRequirementCheck>& NewRequirements);

	UFUNCTION(Category = "Invenza|Crafting", BlueprintCallable)
	TArray<int32> GetAllSelectedOptions();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Settings")
	TSubclassOf<URecipeRequiredIListEntryObject> RequiredListEntryObjectClass;

	// Functions
	void HandleOptionChanged();
};
