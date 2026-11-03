// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "Data/Inventory/InventoryAdmissionTypes.h"
#include "InventoryAdmissionWidget.generated.h"

class UTreeView;
class UEditableTextBox;
class ULabelBaseText;
class UUIButton;
class UInventoryBase;
class UIInventoryManager;
class UInventoryCategoryTreeEntry;

UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryAdmissionWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override;

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|Admission")
	void SetInventory(UInventoryBase* Inventory);
	void ReleaseInput();
	void CompleteSubmission(FGuid RequestID, bool bSuccess);
	void ToggleNode(UInventoryCategoryTreeEntry* Node);
	void SetNodeAllowed(UInventoryCategoryTreeEntry* Node, bool bAllowed);
	EInventoryAdmissionSelection GetNodeSelection(const UInventoryCategoryTreeEntry* Node) const;
	bool CanEdit() const;
	const FInventoryAdmissionRules& GetDraftRules() const { return DraftRules; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UTreeView> CategoryTree;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UEditableTextBox> SearchInput;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TitleLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<ULabelBaseText> StatusLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UUIButton> ApplyButton;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Admission", meta = (BindWidget))
	TObjectPtr<UUIButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryBase> InventoryRef;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryCategoryTreeEntry>> Nodes;

	UPROPERTY(Transient)
	FInventoryAdmissionRules DraftRules;

	FGuid PendingRequest;
	FString Search;
	bool bDirty = false;

	// Functions
	UIInventoryManager* GetManager() const;
	void BuildTree();
	void SyncSelectionFromRules();
	void UpdateSelectionStates();
	void RebuildDraftRules();
	void RefreshTree();
	void UpdateStatus(FName Message = NAME_None);
	void GetChildren(UObject* Item, TArray<UObject*>& Children);

	UFUNCTION()
	void RulesChanged();

	UFUNCTION()
	void SearchChanged(const FText& Text);

	UFUNCTION()
	void ApplyClicked(UUIButton* Button);

	UFUNCTION()
	void CloseClicked(UUIButton* Button);
};
