//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "InvBaseContainerWidget.generated.h"

class ULabelBaseText;
enum class EInventoryType : uint8;
class UMovableTitleBar;
class UUInventoryWidgetBase;
class UAUIManagerActor;
class UInvWeightWidget;
class USlotbasedInventoryWidget;


/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvBaseContainerWidget : public UInvenzaBaseWidget
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetClose, UInvenzaBaseWidget*, Widget);
#pragma endregion Delegates
	
public:
	UInvBaseContainerWidget();

protected:
	virtual void NativeConstruct() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category = "Container|Events" )
	FWidgetClose OnClose;

	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container", meta=(BindWidgetOptional))
	TObjectPtr<UMovableTitleBar> TitleBar;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<ULabelBaseText> InvMoney;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container", meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container", meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> OperationsSlot;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UInvWeightWidget> InvWeight;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void InitializeInventoryBindings();
	
	UFUNCTION(BlueprintCallable, Category = "Container")
	virtual void ChangeInventoryInContainerSlot(TSubclassOf<UInvenzaBaseWidget> NewInventory);
	
	UFUNCTION(BlueprintCallable, Category = "Container")
	virtual UUInventoryWidgetBase* GetInventoryWidgetFromContainerSlot();

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsShowTotalMoney = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool bIsShowWeight = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool bIsShowCloseButton = true;

	// Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FText Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Container")
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
	static void TransferAllItems(UInvBaseContainerWidget* SourceContainer, UInvBaseContainerWidget* TargetContainer);
	UFUNCTION()
	virtual void SortItems();
	
};
