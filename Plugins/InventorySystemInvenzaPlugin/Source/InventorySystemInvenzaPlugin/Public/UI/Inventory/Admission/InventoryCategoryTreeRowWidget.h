// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "Styling/SlateBrush.h"
#include "InventoryCategoryTreeRowWidget.generated.h"

class UImage;
class ULabelBaseText;
class UInventoryCategoryTreeEntry;
class UUIButton;
class USizeBox;

UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryCategoryTreeRowWidget : public UInvenzaBaseWidget,
	public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	virtual void NativeOnItemExpansionChanged(bool bIsExpanded) override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UUIButton> ExpandButton;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<USizeBox> Indentation;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Admission", meta = (ClampMin = "0"))
	float IndentWidth = 18.f;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> CategoryLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UImage> StateIcon;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UUIButton> StateButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Admission|Style")
	FSlateBrush AllowedStateBrush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Admission|Style")
	FSlateBrush BlockedStateBrush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Admission|Style")
	FSlateBrush PartialStateBrush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Admission|Style")
	FSlateBrush CategoryExpansionBrush;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryCategoryTreeEntry> Entry;

	// Functions
	void RefreshRow();
	void ReleaseEntry();

	UFUNCTION()
	void StateClicked(UUIButton* Button);

	UFUNCTION()
	void ExpandClicked(UUIButton* Button);
};
