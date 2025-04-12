//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI/BaseUserWidget.h"
#include "ListInventorySlotWidget.generated.h"

class UTextBlock;
class UScrollBox;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UListInventorySlotWidget : public UInventorySlot, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemName;
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
