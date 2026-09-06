//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/InvenzaSettings.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "UIInventoryManager.generated.h"

class ICraftProvider;
class UInvenzaInventorySettingsAsset;
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
	UInventoryContainerWidget* CreateInventoryWidget(UInventoryBase* InvToLink);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitInvWidgets();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitCraftWidgets();
	

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupStartingResources();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupAdditionalComponents();
	
	// ItemMenu
	virtual void ItemContextMenuRequest_Implementation(const FString& FromInventory, FGuid SlotGuid, UObject* Item) override;

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
	virtual void TransferItemArray_Implementation(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory) override;
	
	virtual void ItemTransferRequest_Implementation(FItemMoveData ItemMoveData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemTransferRequest(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_ItemTransferRequest(FItemMoveData ItemMoveData);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void ApplyItemMove(FItemMoveData ItemMoveData);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	EInventoryContextActionResult ValidateAndEquipTransferredItem(FItemMoveData& ItemMoveData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")	
	void UnequipItemIfNeeded(UInventoryBase* SourceInventory, FGuid SourceSlotID);

	// Split
	virtual void ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemSplitRequest(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_SplitItem(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount);

	// Drop
	virtual void ItemDropRequest_Implementation(FItemDropData DropData) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnItemDrop(FItemDropData DropData);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void HandleItemDrop(FItemDropData DropData );
	
	// Delete
	virtual void ItemDeleteRequest_Implementation(const FString& FromInventory, UObject* Item) override;
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnItemDelete(const FString& FromInventory, UObject* Item);
	
	// Equip
	virtual void ItemEquipRequest_Implementation(UObject* Item);
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemEquipRequest(UObject* Item);
	
	virtual void ItemUnequipRequest_Implementation(UObject* Item);
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemUnequipRequest(UObject* Item);
	
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
	UFUNCTION()
	UInventoryBase* GetPawnMainInventory() const{ return MainPawnInventoryRef;}
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TMap<TObjectPtr<UInventoryBase>, FInitItemsList> StartingItems;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<ILootContainerProvider> LootContainerProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<ICraftProvider> CraftProvider;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<IVendorProvider> VendorProviderCurrent;
	
	// Refs
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<APawn> OwnerPawnRef; 
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	TObjectPtr<UInventoryBase> MainPawnInventoryRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UCraftingComponent> PawnCraftingComponentRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_ActiveCraftComponentRef)
	TObjectPtr<UCraftingComponent> ActiveCraftComponentRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollectionRef;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UInvenzaInventorySettingsAsset> GlobalSettings;
	
	
	//
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UObject> PendingContextItem = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> PendingContextInv = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FGuid PendingContextSlotGuid;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryBase* ResolveTargetInventory(UInventoryBase* SourceInventory);
	
	UFUNCTION(BlueprintCallable)
	bool FindSuitableEquipmentSlot(UObject* Item,UInventoryBase*& OutEquipmentInventory,UInventorySlotData*& OutSuitableSlot,UInventorySlotData*& OutFreeSlot);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual void VendorRequest(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractRequest(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleInteract(UInteractableComponent* Target);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandlePickupInteraction(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleContainerInteraction(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleTradeInteraction(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleCraftStationInteraction(UInteractableComponent* Target);

	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractClearRequest(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleClearInteract(UInteractableComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleClearInteraction(UInteractableComponent* TargetInteractableComponent = nullptr);

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenSecondaryInventory(UInventoryBase* Inv, EInteractableType InteractableType);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseSecondaryInventory(EInteractableType InteractableType);

protected:
	UFUNCTION()
	void OnRep_ActiveCraftComponentRef(UCraftingComponent* PreviousComponent);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInteractionWidget();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
	void BindCraftComponentToWidgets(UCraftingComponent* Component);

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindEvents();
	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInventoryEvents(UInventoryBase* Inventory);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BindWorldDropZoneEvents();
	
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
	
	UFUNCTION()
	void SetInteractionOwnership(AActor* TargetActor, bool bTake);
	
	UFUNCTION(BlueprintCallable)
	TMap<EObjectInteractionType, FModalActionConfig> CollectAccessibleInventoryActions();
	UFUNCTION(BlueprintCallable)
	void OnInventoryModalResponse(FModalResult Result);
};
