//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Data/Inventory/InventorySlotData.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UI/InvenzaBaseWidget.h"
#include "InventorySlot.generated.h"

class UCoreCellWidget;
class UInputAction;
class UItemBase;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventorySlot : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	UInventorySlot();
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EItemCategory AllowedSlotCategory = EItemCategory::All;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FGameplayTag LinkedEquipmentSlotTag;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	bool operator==(const UInventorySlot& Other) const
	{
		return SlotData == Other.SlotData;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void UpdateVisualWithItemInfo(UItemBase* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void UpdateVisualWithTexture(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void ResetVisual();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Visual")
	virtual void ClearVisual();
	
	//Getters
	UInventorySlotData* GetSlotData() {return SlotData;}
	FIntPoint GetSlotPosition() const { return SlotData->InventorySlotInfo.CellPosition; }
	UInputAction* GetUseAction() const;	
	
	//Setters
	void SetSlotData(UInventorySlotData* NewSlotData) { this->SlotData = NewSlotData; }
	void SetSlotPosition(FIntPoint InSlotPosition) const { this->SlotData->InventorySlotInfo.CellPosition = InSlotPosition; }
	virtual void SetSlotNameText(FString InUseKeyText);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory Data")
	TObjectPtr<UInventorySlotData> SlotData;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
