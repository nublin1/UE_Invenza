// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "Styling/SlateBrush.h"
#include "InventoryWorkspaceRowWidget.generated.h"

class UUIButton;
class UImage;
class ULabelBaseText;
class UNamedSlot;
class UInventoryContainerWidget;

/** A storage header and body whose geometry is authored in Widget Blueprint. */
UCLASS(Abstract)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryWorkspaceRowWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Workspace", meta = (BindWidget))
	TObjectPtr<UUIButton> ToggleButton;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Workspace", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TitleLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Workspace", meta = (BindWidget))
	TObjectPtr<UImage> Arrow;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Workspace", meta = (BindWidget))
	TObjectPtr<UNamedSlot> InventoryContent;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Workspace|Appearance")
	FSlateBrush ExpandedBrush;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Workspace|Appearance")
	FSlateBrush CollapsedBrush;

	// Functions
	void ConfigureRow(const FText& Title, UInventoryContainerWidget* Inventory, bool bAllowCollapse, bool bExpanded);

	void SetExpanded(bool bExpanded);
};
