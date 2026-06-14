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
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> RequiredList;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void RefreshRequiredList(const FItemRecipeRow& RecipeRow, const TArray<FRecipeItemRequirementCheck>& Requirements);
	UFUNCTION(BlueprintCallable)
	void UpdateRequirementsCheck(const FItemRecipeRow& UpdateRecipeRow, const TArray<FRecipeItemRequirementCheck>& NewRequirements);
	
	UFUNCTION(BlueprintCallable)
	TArray<int32> GetAllSelectedOptions();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Settings")
	TSubclassOf<URecipeRequiredIListEntryObject> RequiredListEntryObjectClass;

	//====================================================================
	// FUNCTIONS
	//====================================================================
};
