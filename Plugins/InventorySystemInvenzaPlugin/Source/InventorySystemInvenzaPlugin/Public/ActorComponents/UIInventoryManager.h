//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/InvenzaSettings.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "UIInventoryManager.generated.h"

struct FModalResult;
enum class EObjectInteractionType : uint8;
struct FModalActionConfig;
class UCraftingComponent;
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
class INVENTORYSYSTEMINVENZAPLUGIN_API UIInventoryManager : public UActorComponent, public IInventoryInteractionHandler
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	void InitInvWidgets();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitCraftWidgets();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BindWorldDropZoneEvents();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupStartingResources();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupAdditionalComponents();
	
	// ItemMenu
	virtual void ItemContextMenuRequest_Implementation(const FString& FromInventory, FGuid SlotGuid, UItemBase* Item) override;

	// Quick Transfer
	virtual void OnQuickTransferItem_Implementation(FItemMoveData InData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnQuickTransferItem(FItemMoveData InData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_QuickTransfer_Internal(FItemMoveData InData);
	
	virtual void OnQuickTransferAllSameItems_Implementation(FItemMoveData ItemMoveData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnQuickTransferAllSameItems(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_QuickTransferAllSameItems_Internal(FItemMoveData InData);

	// Transfer
	virtual void ItemTransferRequest_Implementation(FItemMoveData ItemMoveData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemTransferRequest(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_ItemTransferRequest(FItemMoveData ItemMoveData);

	// Split
	virtual void ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UItemBase* ItemToSplit, int32 SplitAmount) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemSplitRequest(UInventoryBase* TargetInventory, UItemBase* ItemToSplit, int32 SplitAmount);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_SplitItem(UInventoryBase* TargetInventory, UItemBase* ItemToSplit, int32 SplitAmount);

	// Drop
	virtual void ItemDropRequest_Implementation(FItemDropData DropData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnItemDrop(FItemDropData DropData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void HandleItemDrop(FItemDropData DropData );
	
	// Delete
	virtual void ItemDeleteRequest_Implementation(const FString& FromInventory, UItemBase* Item) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnItemDelete(const FString& FromInventory, UItemBase* Item);
	
	// Use
	virtual void RequestUseSlot_Implementation(const FString& InvID, FGuid SlotID) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_RequestUseSlot(const FString& InvID, FGuid SlotID);
	UFUNCTION()
	void OnUseSlotInput(FString InvID, FGuid SlotID);

	//
	virtual void RebuildInventoryRequest_Implementation(const FString& InvID) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
	void Server_RebuildInventory(const FString& InvID);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HandleRebuildInventory(const FString& InvID);
	
	//	
	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FUISettings GetUISettings() const { return UISettings; }

	virtual FInventoryModifierState GetInventoryModifierStates_Implementation() const override { return InventoryModifierState; }

	//
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

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Inventory")
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<ILootContainerProvider> LootContainerProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<IVendorProvider> VendorProviderCurrent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainPawnInventory;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UCraftingComponent> CraftingComponentRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollectionRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UItemBase> PendingContextItem = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> PendingContextInv = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FGuid PendingContextSlotGuid;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void OnItemAddedToInventory(FItemMapping& ItemSlots, UItemBase* Item);
	UFUNCTION()
	void OnItemRemovedFromInventory(FItemMapping ItemSlots, UItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryBase* ResolveTargetInventory(UInventoryBase* SourceInventory);	
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual void VendorRequest(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractRequest(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleInteract(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleInteract(UInteractableComponent* TargetInteractableComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractClearRequest(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleClearInteract(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleClearInteraction(UInteractableComponent* TargetInteractableComponent = nullptr);

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenVendorInventory(UInventoryBase* Inv);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseVendorInventory(UInventoryBase* Inv);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenExternalInventory(UInventoryBase* Inv);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseExternalInventory(UInventoryBase* Inv);

protected:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInteractionWidget();

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindEvents();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInventoryEvents(UInventoryBase* Inventory);
	
protected:
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
	void HandleToggleInventory() ;
	UFUNCTION()
	void HandleToggleCraftMenu() ;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void InitializeBindings();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void BindInputActions();

	UFUNCTION(BlueprintCallable)
	virtual TMap<EObjectInteractionType, FModalActionConfig> CollectAccessibleItemActions(UItemBase* InItem);
	
	UFUNCTION(BlueprintCallable)
	void OnInventoryModalResponse(FModalResult Result);
};
