//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "UI/BaseUserWidget.h"
#include "SlotbasedInventorySlot.generated.h"

class UTextBlock;
class UCoreCellWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API USlotbasedInventorySlot : public UInventorySlot
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCoreCellWidget> CoreCellWidget;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemUseKey;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	USlotbasedInventorySlot();

	virtual void SetItemUseKeyText(FString InUseKeyText)override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InUseKeyTextByDefault;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void NativeConstruct() override;
};
