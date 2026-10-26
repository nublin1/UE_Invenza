//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "UI/Inventory/InventoryWorkspaceTypes.h"
#include "StorageAccessWidget.generated.h"

class UInventoryWorkspaceWidget;
class UIInventoryManager;
class UUIButton;
class ULabelBaseText;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UStorageAccessWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UStorageAccessWidget();

protected:
	// Unreal overrides
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

public:
	// Properties and variables
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UInventoryWorkspaceWidget> Workspace;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<ULabelBaseText> TitleLabel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UUIButton> CloseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UUIButton> RefreshButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Storage Access|Workspace")
	FInventoryWorkspaceSettings WorkspaceSettings;

	// Functions
	void SetManager(UIInventoryManager* InManager) { Manager = InManager; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UIInventoryManager> Manager;

	// Functions
	UFUNCTION()
	void CloseClicked(UUIButton* Button);

	UFUNCTION()
	void RefreshClicked(UUIButton* Button);
};
