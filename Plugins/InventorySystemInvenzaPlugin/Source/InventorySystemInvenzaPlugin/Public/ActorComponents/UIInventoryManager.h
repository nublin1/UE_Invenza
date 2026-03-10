//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Service/TradeService.h"
#include "Settings/InvnzaSettings.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "UIInventoryManager.generated.h"

enum class EInteractableType : uint8;
struct FItemMoveData;
struct FInputActionInstance;
struct FItemMetaData;
class FIteract;
class UItemCollection;
class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UIInventoryManager : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializationComplete);
#pragma endregion Delegates

public:
	UIInventoryManager();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	
	//====================================================================
	// Delegates
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory|Events")
	FOnInitializationComplete OnInitializationCompleteDelegate;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateInventories();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitWidgets();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	void OpenTradeModal(bool bIsSaleOperation, UItemBase* OperationalItem);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void OnQuickTransferItem(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void ItemTransferRequest(FItemMoveData ItemMoveData);
	
	/*UFUNCTION(BlueprintCallable, Category = "Inventory|Query")
	UInvBaseContainerWidget* GetMainInventory() const { return CoreHUDWidget ? CoreHUDWidget->GetMainInvWidget() : nullptr; }*/
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FUISettings GetUISettings() const { return UISettings; }
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	UCoreHUDWidget* GetCoreHUDWidget() const { return CoreHUDWidget; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	UInvBaseContainerWidget* GetCurrentInteractInvWidget() const { return CurrentInteractInvWidget.Get(); }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FInventoryModifierState GetInventoryModifierStates() const { return InventoryModifierState; }

	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByID(FString ContainerID);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Widgets")
	TObjectPtr<UCoreHUDWidget> CoreHUDWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TObjectPtr<UInvBaseContainerWidget> CurrentInteractInvWidget;

	//
	UPROPERTY(EditAnywhere, Category="Inventory")
	TArray<FInventoryStartupData> StartupInventories;

	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FInventoryModifierState InventoryModifierState;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	TObjectPtr<UDataTable> ItemDataTable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TObjectPtr<UModalTradeWidget> ModalTradeWidget;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, TObjectPtr<UInventoryBase>> Inventories;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual ETradeResult VendorRequest(FTradeRequest TradeRequest);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void SetInteractableType(UInteractableComponent* InteractData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void ClearInteractableType(UInteractableComponent* InteractData = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindEvents(AActor* TargetActor);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void UIIteract(UInteractableComponent* TargetInteractableComponent);
	
	UFUNCTION()
	void OnQuickGrabPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnQuickGrabReleased(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnGrabAllPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnGrabAllReleased(const FInputActionInstance& Instance);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void InitializeBindings();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void BindInputActions();
	
	
};
