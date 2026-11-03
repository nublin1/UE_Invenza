// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Data/Inventory/InventoryAdmissionTypes.h"
#include "InventoryCategoryTreeEntry.generated.h"

class UInventoryAdmissionWidget;
class UTexture2D;

/** Local presentation data; never stored in or replicated with an inventory. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryCategoryTreeEntry : public UObject
{
	GENERATED_BODY()

public:
#pragma region Delegates
	DECLARE_MULTICAST_DELEGATE(FOnAdmissionNodeChanged);
#pragma endregion Delegates

	// Properties and variables
	FOnAdmissionNodeChanged OnChanged;
	FGameplayTag Category;
	FText Label;

	UPROPERTY(Transient)
	FDataTableRowHandle ItemDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> ItemIcon;
	bool bMatchesSearch = true;
	int32 Depth = 0;
	EInventoryAdmissionSelection Selection = EInventoryAdmissionSelection::Blocked;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryAdmissionWidget> Window;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryCategoryTreeEntry>> Children;
};
