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

struct FItemMetaData;
class FIteract;
enum class EInteractableType : uint8;
class UItemCollection;
struct FItemMoveData;
struct FInputActionInstance;
class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializationComplete);
#pragma endregion Delegates

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UIInventoryManager : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// Delegates
	//====================================================================
	/** Событие по завершению инициализации */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory|Events")
	FOnInitializationComplete OnInitializationCompleteDelegate;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UIInventoryManager();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	void OpenTradeModal(bool bIsSaleOperation, UItemBase* OperationalItem);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void OnQuickTransferItem(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void ItemTransferRequest(FItemMoveData ItemMoveData);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Query")
	UInvBaseContainerWidget* GetMainInventory() const { return CoreHUDWidget ? CoreHUDWidget->GetMainInvWidget() : nullptr; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FUISettings GetUISettings() const { return UISettings; }
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	UCoreHUDWidget* GetCoreHUDWidget() const { return CoreHUDWidget; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	UInvBaseContainerWidget* GetCurrentInteractInvWidget() const { return CurrentInteractInvWidget.Get(); }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FInventoryModifierState GetInventoryModifierStates() const { return InventoryModifierState; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Widgets")
	TObjectPtr<UCoreHUDWidget> CoreHUDWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TObjectPtr<UInvBaseContainerWidget> CurrentInteractInvWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FInventoryModifierState InventoryModifierState;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	TObjectPtr<UDataTable> ItemDataTable;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TObjectPtr<UModalTradeWidget> ModalTradeWidget;

	//====================================================================
	// Lifecycle
	//====================================================================
	virtual void BeginPlay() override;
	
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
	void InitializeInvSlotsBindings();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction);
};
