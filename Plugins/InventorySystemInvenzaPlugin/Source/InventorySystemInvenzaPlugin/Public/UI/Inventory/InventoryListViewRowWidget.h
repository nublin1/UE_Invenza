// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "InventoryListViewRowWidget.generated.h"

class UImage;
class UInventoryViewEntry;
class UItemTooltipWidget;
class ULabelBaseText;

/** A display row with selection and a tooltip, without inventory-slot operations. */
UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryListViewRowWidget : public UInvenzaBaseWidget,
	public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;
	virtual void NativeOnDragDetected(const FGeometry& Geometry, const FPointerEvent& Event,
		UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& Geometry, const FDragDropEvent& Event,
		UDragDropOperation* Operation) override;

public:
	// Functions
	UFUNCTION(BlueprintPure, Category = "Inventory View")
	UInventoryViewEntry* GetViewEntry() const { return ViewEntry; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> ItemName;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory View", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> ItemQuantity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory View")
	TSubclassOf<UItemTooltipWidget> ItemTooltipWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryViewEntry> ViewEntry;

	UPROPERTY(Transient)
	TObjectPtr<UItemTooltipWidget> ItemTooltip;

	// Functions
	UFUNCTION()
	void RefreshRow();

	void ReleaseEntry();
	void ReleaseTooltip();
};
