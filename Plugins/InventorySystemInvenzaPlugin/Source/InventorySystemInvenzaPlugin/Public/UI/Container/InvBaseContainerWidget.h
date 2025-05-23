//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/BaseUserWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "InvBaseContainerWidget.generated.h"

enum class EInventoryType : uint8;
class UMovableTitleBar;
class UUInventoryWidgetBase;
class UAUIManagerActor;
class UInvWeightWidget;
class USlotbasedInventoryWidget;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetClose, UBaseUserWidget*, Widget);
#pragma endregion Delegates
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvBaseContainerWidget : public UBaseUserWidget
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FWidgetClose OnClose;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UInvBaseContainerWidget();

	UFUNCTION(BlueprintCallable)
	virtual void ChangeInventoryInContainerSlot(TSubclassOf<UBaseUserWidget> NewInventory);

	UFUNCTION()
	virtual UMovableTitleBar* GetTitleBar() const{return TitleBar;}
	UFUNCTION()
	EInventoryType GetInventoryType() const {return InventoryType;}

	UFUNCTION(BlueprintCallable)
	virtual UUInventoryWidgetBase* GetInventoryFromContainerSlot();

	UFUNCTION(BlueprintCallable, Category = "Container|Comparison")
	bool EqualsByNameAndType(FName InName, EInventoryType InType) const
	{
		return GetFName() == InName && InventoryType == InType;
	}
	
	bool operator==(const UInvBaseContainerWidget& other) const
	{
		return (GetFName() == other.GetFName() && InventoryType == other.InventoryType);
	}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	//Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UMovableTitleBar> TitleBar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> ContainerSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta=(BindWidgetOptional))
	TObjectPtr<UNamedSlot> OperationsSlot;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UInvWeightWidget> InvWeight;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInventoryType InventoryType = EInventoryType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	bool bIsShowTotalMoney = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	bool bIsShowWeight = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	bool bIsShowCloseButton = true;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void CloseButtonClicked();
	
	UFUNCTION()
	virtual void UpdateWeightInfo(float InventoryTotalWeight, float InventoryWeightCapacity);
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
