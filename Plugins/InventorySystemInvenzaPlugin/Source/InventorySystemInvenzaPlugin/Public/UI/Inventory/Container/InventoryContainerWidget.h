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

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetClose, UInvenzaBaseWidget*, Widget);
#pragma endregion Delegates
	
public:
	UInventoryContainerWidget();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Container|Events" )
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
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Container|Inventory")
	void InitializeInventoryBindings();
	
	UFUNCTION(BlueprintCallable, Category = "Container|Inventory")
	virtual void ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory);
	
	UFUNCTION(BlueprintPure, Category = "Container|Inventory")
	virtual UUInventoryBaseWidget* GetInventoryWidgetFromContainerSlot();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowTotalMoney = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	bool bIsShowCloseButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Config")
	FText Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Runtime")
	TObjectPtr<UInventoryBase> InventoryRef;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	// UI
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
	UFUNCTION(BlueprintCallable, Category = "Container|Operations")
	virtual void SortItems();
	
};
