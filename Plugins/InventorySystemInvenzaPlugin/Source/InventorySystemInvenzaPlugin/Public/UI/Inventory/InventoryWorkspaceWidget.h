//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/InventoryWorkspaceTypes.h"
#include "InventoryWorkspaceWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UUIButton;
class ULabelBaseText;
class UInventoryContainerWidget;
class UNamedSlot;
class UImage;
class UInventoryWorkspaceRowWidget;

/** Shared inventory layout. Filtering changes visibility, never inventory access. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryWorkspaceWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UInventoryWorkspaceWidget();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UScrollBox> Inventories;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UNamedSlot> PlayerSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UEditableTextBox> SearchText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UUIButton> ClearSearch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<ULabelBaseText> EmptyLabel;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULabelBaseText> LabelClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Workspace|Appearance")
	TSubclassOf<UUIButton> HeaderButtonClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Workspace")
	FInventoryWorkspaceSettings DisplaySettings;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Workspace|Layout")
	FMargin InventoryRowPadding = FMargin(4.f, 4.f, 12.f, 16.f);

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Workspace|Layout", meta = (ClampMin = "0"))
	float PlayerColumnExtraWidth = 20.f;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory Workspace")
	void ConfigureWorkspace(const FInventoryWorkspaceSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Inventory Workspace")
	void SetInventoryExpanded(UInventoryContainerWidget* Widget, bool bExpanded);

	UFUNCTION(BlueprintPure, Category = "Inventory Workspace")
	bool IsInventoryExpanded(UInventoryContainerWidget* Widget) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Workspace")
	void SetInventoryWidgets(const TArray<UInventoryContainerWidget*>& Widgets, const TArray<FText>& Titles);

	UFUNCTION(BlueprintCallable, Category = "Inventory Workspace")
	void SetPlayerWidget(UInventoryContainerWidget* Widget);

	UInventoryContainerWidget* ReleasePlayerWidget();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidget>> Rows;

	UPROPERTY(Transient)
	TArray<FText> RowTitles;

	TArray<TWeakObjectPtr<UInventoryContainerWidget>> InstalledWidgets;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryContainerWidget> PlayerWidget;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UUIButton>, TObjectPtr<UInventoryContainerWidget>> CollapseButtons;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UUIButton>, TObjectPtr<UInventoryWorkspaceRowWidget>> CollapseArrows;

	TSet<TWeakObjectPtr<UInventoryContainerWidget>> CollapsedInventories;

	TMap<TWeakObjectPtr<UInventoryContainerWidget>, bool> OriginalCloseButtonSettings;

	// Functions
	UFUNCTION()
	void FilterChanged(const FText& Text);

	UFUNCTION()
	void ClearClicked(UUIButton* Button);

	UFUNCTION()
	void CollapseClicked(UUIButton* Button);

	void UpdatePlayerColumnWidth();

	void ApplyContainerOptions(UInventoryContainerWidget* Widget);

	void RestoreContainerOptions(UInventoryContainerWidget* Widget);

	void RestoreAllContainerOptions();

	void UpdateCollapseArrow(UUIButton* Button, bool bExpanded);
};
