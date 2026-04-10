//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "InventoryContainerWidget.generated.h"

class ULabelBaseText;
enum class EInventoryType : uint8;
class UMovableTitleBar;
class UUInventoryBaseWidget;
class UAUIManagerActor;
class UInvWeightWidget;
class USlotbasedInventoryWidget;


/**
 * 
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
	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category = "Container|Events" )
	FWidgetClose OnClose;

	//Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta=(BindWidgetOptional))
	TObjectPtr<UMovableTitleBar> TitleBar;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional), Category = "Container|Widgets")
	TObjectPtr<ULabelBaseText> InvMoney;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container|Widgets", meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> OperationsSlot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional), Category = "Container|Widgets")
	TObjectPtr<UInvWeightWidget> InvWeight;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void InitializeInventoryBindings();
	
	UFUNCTION(BlueprintCallable, Category = "Container")
	virtual void ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory);
	
	UFUNCTION(BlueprintCallable, Category = "Container")
	virtual UUInventoryBaseWidget* GetInventoryWidgetFromContainerSlot();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Container|Config")
	bool bIsShowTotalMoney = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Container|Config")
	bool bIsShowWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Container|Config")
	bool bIsShowCloseButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Container|Config")
	FText Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Inventory Container|Runtime")
	TObjectPtr<UInventoryBase> InventoryRef;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	virtual void CloseButtonClicked();
	
	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight);
	UFUNCTION()
	virtual void UpdateMoneyInfo(int32 TotalMoney);

	UFUNCTION()
	virtual void TakeAll();
	UFUNCTION()
	virtual void PlaceAll();
	UFUNCTION()
	static void TransferAllItems(UInventoryContainerWidget* SourceContainer, UInventoryContainerWidget* TargetContainer);
	UFUNCTION()
	virtual void SortItems();
	
};
