// Nublin Studio 2025 All Rights Reserve

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UI/InvenzaBaseWidget.h"
#include "WorldDropZoneWidget.generated.h"

struct FItemMoveData;
class UItemBase;
class UBorder;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UWorldDropZoneWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDroppedToWorld, FItemDropData, DropData);
#pragma endregion Delegates

public:
	UWorldDropZoneWidget();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Drop Zone")
	FOnItemDroppedToWorld OnItemDroppedToWorld;

	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
