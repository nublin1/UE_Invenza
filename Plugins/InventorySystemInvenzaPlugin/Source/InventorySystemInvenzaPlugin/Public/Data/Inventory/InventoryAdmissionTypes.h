// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "InventoryAdmissionTypes.generated.h"

UENUM(BlueprintType)
enum class EInventoryAdmissionSelection : uint8
{
	Blocked,
	Allowed,
	Partial
};

/** Admission is separate from the category requirements of individual cells. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInventoryAdmissionRules
{
	GENERATED_BODY()

	/** An empty allow list accepts every category unless excluded below. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Admission")
	FGameplayTagContainer AllowedCategories;

	/** Exclusions include descendants and always take precedence over allowances. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Admission")
	FGameplayTagContainer ExcludedCategories;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Admission", meta = (RowType = "/Script/InventorySystemInvenzaPlugin.ItemData"))
	TArray<FDataTableRowHandle> AllowedItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Admission", meta = (RowType = "/Script/InventorySystemInvenzaPlugin.ItemData"))
	TArray<FDataTableRowHandle> ExcludedItems;

	bool Allows(FGameplayTag Category) const
	{
		return !Category.MatchesAny(ExcludedCategories) &&
			((AllowedCategories.IsEmpty() && AllowedItems.IsEmpty()) || Category.MatchesAny(AllowedCategories));
	}

	bool AllowsItem(FGameplayTag Category, const FDataTableRowHandle& Definition) const
	{
		return !Category.MatchesAny(ExcludedCategories) && !ExcludedItems.Contains(Definition) &&
			((AllowedCategories.IsEmpty() && AllowedItems.IsEmpty()) ||
			 Category.MatchesAny(AllowedCategories) || AllowedItems.Contains(Definition));
	}

	bool operator==(const FInventoryAdmissionRules& Other) const
	{
		return AllowedCategories == Other.AllowedCategories && ExcludedCategories == Other.ExcludedCategories &&
			AllowedItems == Other.AllowedItems && ExcludedItems == Other.ExcludedItems;
	}
};
