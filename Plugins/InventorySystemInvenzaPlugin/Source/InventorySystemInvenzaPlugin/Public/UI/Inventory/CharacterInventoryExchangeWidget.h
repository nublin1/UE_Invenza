//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/InvenzaBaseWidget.h"
#include "CharacterInventoryExchangeWidget.generated.h"

class UIInventoryManager;
class UInventoryContainerWidget;
class UUIButton;
class ULabelBaseText;
class UVerticalBox;
class UHorizontalBox;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCharacterInventoryExchangeWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UCharacterInventoryExchangeWidget();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

public:
	// Functions
	void SetManager(UIInventoryManager* InManager) { Manager = InManager; }

	UFUNCTION(BlueprintCallable, Category = "Character Inventory")
	void SetInventories(const FText& CharacterName, const TArray<UInventoryContainerWidget*>& Other,
						const TArray<UInventoryContainerWidget*>& Player);

	UFUNCTION(BlueprintCallable, Category = "Character Inventory")
	void ClearInventories();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditDefaultsOnly, Category = "Character Inventory|Layout")
	FMargin InventoryPadding = FMargin(0, 0, 12, 0);

	UPROPERTY(Transient)
	TObjectPtr<UIInventoryManager> Manager;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUIButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULabelBaseText> CharacterLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> OtherInventories;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> PlayerInventories;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UInventoryContainerWidget>, bool> OriginalCloseOptions;

	// Functions
	UFUNCTION()
	void CloseClicked(UUIButton* Button);
};
