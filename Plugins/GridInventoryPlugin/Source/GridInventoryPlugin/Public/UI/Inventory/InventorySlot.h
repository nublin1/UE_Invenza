//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "UI/BaseUserWidget.h"
#include "InventorySlot.generated.h"

class UCoreCellWidget;
class UInputAction;
class UItemBase;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UInventorySlot : public UBaseUserWidget
{
	GENERATED_BODY()
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInventorySlot();

	bool operator==(const UInventorySlot& other) const
	{
		return SlotPosition == other.GetSlotPosition();
	}
	
	UFUNCTION(BlueprintCallable)
	virtual void UpdateVisual(UItemBase* Item);
	
	//Getters
	FORCEINLINE FIntVector2 GetSlotPosition() const { return SlotPosition; }
	FORCEINLINE UInputAction* GetUseAction() const { return UseAction; }	
	
	//Setters	
	FORCEINLINE void SetSlotPosition(const FIntVector2 InSlotPosition) { this->SlotPosition = InSlotPosition; }
	FORCEINLINE virtual void SetItemUseKeyText(FString InUseKeyText);
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(VisibleAnywhere)
	FIntVector2 SlotPosition{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> UseAction;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
};
