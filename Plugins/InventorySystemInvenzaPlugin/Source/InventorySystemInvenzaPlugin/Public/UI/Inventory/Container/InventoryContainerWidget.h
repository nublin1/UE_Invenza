//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "InventoryContainerWidget.generated.h"

class UUIButton;
class ULabelBaseText;
enum class EInventoryType : uint8;
class UMovableTitleBar;
class UUInventoryBaseWidget;
class UAUIManagerActor;
class UInvWeightWidget;

/**
 * Base container widget for displaying an inventory together with optional title, money,
 * weight and inventory operations.
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryContainerWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UInventoryContainerWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetClose, UInvenzaBaseWidget*, Widget);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Container|Events")
	FWidgetClose OnClose;

	//Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UMovableTitleBar> TitleBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<ULabelBaseText> InvMoney;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> OperationsSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta = (BindWidgetOptional))
	TObjectPtr<UInvWeightWidget> InvWeight;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Container|Inventory")
	void InitializeInventoryContainer();

	UFUNCTION(BlueprintCallable, Category = "Container|Inventory")
	virtual void ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory);

	UFUNCTION(BlueprintPure, Category = "Container|Inventory")
	virtual UUInventoryBaseWidget* GetInventoryWidgetFromContainerSlot();

	UFUNCTION(BlueprintCallable, Category = "Container|Inventory")
	virtual void ReDrawRequest();

	/** Disconnect a discarded panel before releasing its inventory. */
	void ReleaseInventory();

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void SetShowCloseButton(bool bShow);

	UFUNCTION(BlueprintPure, Category = "Container|UI")
	bool GetShowCloseButton() const { return bIsShowCloseButton; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowTotalMoney = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowCloseButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	FText Title;

	//

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Runtime")
	TObjectPtr<UInventoryBase> InventoryRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Runtime")
	TObjectPtr<UUInventoryBaseWidget> InventoryWidgetRef;

	// Functions
	// UI
	void UpdateCloseButtonVisibility();

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	virtual void CloseButtonClicked(UUIButton* Btn);

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	virtual void UpdateWeightInfo(float InventoryTotalWeight);

	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	virtual void UpdateMoneyInfo(int32 TotalMoney);

	UFUNCTION(BlueprintCallable, Category = "Container|Operations")
	virtual void TakeAll();

	UFUNCTION(BlueprintCallable, Category = "Container|Operations")
	virtual void PlaceAll();

	UFUNCTION()
	void PlaceMatching();

	UFUNCTION(BlueprintCallable, Category = "Container|Operations")
	virtual void SortItems();

	UFUNCTION(Category = "Invenza|UI|Inventory", BlueprintCallable)
	void InitializeTitleBar();

	UFUNCTION(Category = "Invenza|UI|Inventory", BlueprintCallable)
	void InitializeInventoryInfo();

	UFUNCTION(Category = "Invenza|UI|Inventory", BlueprintCallable)
	void InitializeOperations();

	UFUNCTION()
	virtual void UpdateTitle(FText NewText);
};
