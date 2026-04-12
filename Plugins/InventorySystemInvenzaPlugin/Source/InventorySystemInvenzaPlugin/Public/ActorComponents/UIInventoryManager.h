//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/InvenzaSettings.h"
#include "UI/Inventory/Container/InventoryContainerWidget.h"
#include "Data/Inventory/InventoryTypes.h"
#include "UIInventoryManager.generated.h"

class ILootContainerProvider;
class UVendorComponent;
class IVendorProvider;
class IInteractionUIProvider;
class UInteractionComponent;
class IInvUIProvider;
class UEquipmentComponent;
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
	void InitializeInventoryManager();
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateInventories();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateWidgetsForInventories(); 
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CreateWidget(UInventoryBase* InvToLink);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitWidgets();
	
public:

	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupStartingResources();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void OnQuickTransferItem(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void ItemTransferRequest(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void ItemDropRequest(UItemBase* ItemToDrop);
		
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FUISettings GetUISettings() const { return UISettings; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FInventoryModifierState GetInventoryModifierStates() const { return InventoryModifierState; }

	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable)
	UInventoryBase* GetInventoryByID(FString ContainerID);

	UFUNCTION(Category = "Inventory|UI")
	void SetUIProvider(const TScriptInterface<IInvUIProvider>& NewUIProvider) { UIInvProvider = NewUIProvider; }
	UFUNCTION(Category = "Inventory|UI")
	void SetInteractionUIProvider(const TScriptInterface<IInteractionUIProvider>& NewUIProvider) { InteractionUIProvider = NewUIProvider; }

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Startup
	UPROPERTY(EditAnywhere, Category="Inventory")
	TArray<FInventoryStartupData> StartupInventories;
	
	TMap<UInventoryBase*, TArray<FInitItemsEntry>> StartingItems;

	TArray<UInventoryBase*> InventoryWidgetInitMap;

	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FInventoryModifierState InventoryModifierState;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<IInvUIProvider> UIInvProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<IInteractionUIProvider> InteractionUIProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<ILootContainerProvider> LootContainerProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UVendorComponent> VendorProviderCurrent;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> ExternalInventory;

	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> VendorInventory;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryContainerWidget> VendorInventoryContainerWidget;
	
	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TMap<FString, TObjectPtr<UInventoryBase>> Inventories;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>> InventorContainerWidgetMap;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainPawnInventory;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollectionRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnItemAddedToInventory(FItemMapping& ItemSlots, UItemBase* Item);
	UFUNCTION()
	void OnItemRemovedFromInventory(FItemMapping ItemSlots, UItemBase* Item);

	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual void VendorRequest(FItemMoveData ItemMoveData);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleInteract(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleClearInteraction(UInteractableComponent* TargetInteractableComponent = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenExternalInventory(UInventoryBase* Inv);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseExternalInventory(UInventoryBase* Inv);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInteractionWidget();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindEvents();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInventoryEvents(UInventoryBase* Inventory);
		
	
	UFUNCTION()
	void OnQuickGrabPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnQuickGrabReleased(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnGrabAllPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnGrabAllReleased(const FInputActionInstance& Instance);
	UFUNCTION()
	void RotateDraggedItem();

	UFUNCTION()
	void HandleToggleInventory();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void InitializeBindings();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void BindInputActions();
	
};
