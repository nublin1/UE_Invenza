// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Interface/UI/ModalInterface.h"
#include "UI/InvenzaBaseWidget.h"
#include "InventoryTakeAmountHeaderWidget.generated.h"

class UInventoryBase;
class UInventoryCollectionComponent;
class ULabelBaseText;
class USpinBox;

/** Designer-authored amount selector. Only the server manager can move items. */
UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryTakeAmountHeaderWidget : public UInvenzaBaseWidget,
	public IModalInterface
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativeDestruct() override;

public:
	// Functions
	virtual void ConfigureHeader_Implementation(FModalHeaderData HeaderData) override;
	virtual FModalHeaderResult GetHeaderResult_Implementation() override;
	void SetServerFeedback(const FText& Message, int32 SourceQuantity, int32 AvailableAmount);
	void SetSubmitting();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TitleLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULabelBaseText> TakeLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULabelBaseText> RemainingLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULabelBaseText> FeedbackLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpinBox> AmountInput;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Take Amount|Text")
	FText TakeFormat;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Take Amount|Text")
	FText RemainingFormat;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Take Amount|Text")
	FText NoAvailableAmountText;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Take Amount|Text")
	FText SubmittingText;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryBase> SourceInventory;

	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> SourceItem;

	TWeakObjectPtr<UInventoryCollectionComponent> SourceCollection;
	int32 TotalQuantity = 0;
	int32 AvailableQuantity = 0;
	int32 SelectedQuantity = 1;
	bool bUpdatingInput = false;

	// Functions
	void UnbindSource();
	void UpdateLabels();

	UFUNCTION()
	void RefreshSource();

	UFUNCTION()
	void HandleSourceItemChanged(UObject* Item);

	UFUNCTION()
	void HandleQuantityChanged(int32 Amount, UObject* Item);

	UFUNCTION()
	void HandleCollectionChanged(const FString& InventoryID);

	UFUNCTION()
	void HandleInputChanged(float Value);
};
