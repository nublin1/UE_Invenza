//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/InvenzaSettings.h"
#include "Data/Inventory/InventoryTypes.h"
#include "Data/Trade/TradeTypes.h"
#include "Data/Inventory/StorageAccessTypes.h"
#include "Data/Interactable/InteractableData.h"
#include "UI/Inventory/InventoryPanelPresentationTypes.h"
#include "Interface/Inventory/InventoryInteractionHandler.h"
#include "SaveLoad/InventorySaveTypes.h"
#include "UIInventoryManager.generated.h"

enum class EInteractionType : uint8;
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
class UInventoryCollectionComponent;
class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;
class UStorageAccessWidget;
class UCharacterInventoryAccessComponent;
class UCharacterInventoryExchangeWidget;
struct FEquipmentInventoryContext;

UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UIInventoryManager : public UActorComponent, public IInventoryInteractionHandler
{
	GENERATED_BODY()

public:
	UIInventoryManager();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializationComplete);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeSessionItemsChanged);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Trade")
	FOnTradeSessionItemsChanged OnTradeSessionItemsChanged;

	// Delegates

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory|Events")
	FOnInitializationComplete OnInitializationCompleteDelegate;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Save")
	EInventoryClientTrust SessionClientTrust = EInventoryClientTrust::ServerValidated;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void ShowControlsHelp();

	/** Applies only to the station currently opened by this player's interaction. */
	UFUNCTION(Server, Reliable, Category = "Inventory|Crafting")
	void Server_SetStationAutomaticCrafting(UCraftingComponent* Station, bool bAutomatic);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Storage Access")
	void CloseStorageAccess();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Storage Access")
	void RefreshStorageAccess();

	bool IsInventoryAccessible(UInventoryBase* Inventory) const;

	bool IsCharacterInventoryAccessible(UInventoryBase* Inventory) const;

	bool TryResolveEquipmentContext(UInventoryBase* Inventory, FGuid SlotID,
									FEquipmentInventoryContext& OutContext) const;

	/** Returns whether this is an equipment operation; OutCanTransfer is its read-only validation result. */
	bool CheckEquipmentTransfer(FItemMoveData ItemMoveData, bool& OutCanTransfer);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Character Exchange")
	void CloseCharacterInventory();

	UFUNCTION(Server, Reliable, Category = "Inventory|Equipment")
	void Server_EquipInventoryItem(UInventoryBase* Inventory, UObject* Item, bool bUnequip);

	bool IsStorageInventoryAccessible(const UInventoryBase* Inventory) const;

	UInventoryBase* FindStorageInventory(const FString& ID) const;

	bool IsTradeSessionInventory(const UInventoryBase* Inventory) const;

	bool WasSoldThisSession(const UInventoryBase* Inventory, const UObject* Item) const;

	uint32 GetTradeSessionId() const { return TradeSessionItems.SessionId; }

	/** Called by the trade window when it closes. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	void CloseTradeSession();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeInventoryManager();

	/** Trusted integrations can submit a complete own-character state through the owned manager. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void SubmitTrustedInventoryState(const FInventorySavedOwner& State);

	EInventoryClientTrust GetSessionClientTrust() const { return SessionClientTrust; }

	void SetSessionClientTrust(EInventoryClientTrust Mode) { SessionClientTrust = Mode; }

	UFUNCTION(Server, Reliable)
	void Server_SubmitTrustedInventoryState(const FInventorySavedOwner& State);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupStartingResources();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	virtual void SetupAdditionalComponents();

	// ItemMenu
	virtual void ItemContextMenuRequest_Implementation(const FString& FromInventory, FGuid SlotGuid,
													   UObject* Item) override;

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
	virtual void TransferItemArray_Implementation(UInventoryBase* SourceInventory,
												  UInventoryBase* TargetInventory) override;

	/** Move all stacks whose item IDs were present in the destination at the start. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void TransferMatchingItems(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory);

	UFUNCTION(BlueprintPure, Category = "Inventory|Transfer")
	bool CanTransferMatchingItems(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory) const;

	UFUNCTION(Server, Reliable)
	void Server_TransferMatchingItems(UInventoryBase* SourceInventory, UInventoryBase* TargetInventory);

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
	virtual void ItemSplitRequest_Implementation(UInventoryBase* TargetInventory, UObject* ItemToSplit,
												 int32 SplitAmount) override;

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_ItemSplitRequest(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void Handle_SplitItem(UInventoryBase* TargetInventory, UObject* ItemToSplit, int32 SplitAmount);

	// Drop
	virtual void ItemDropRequest_Implementation(FItemDropData DropData) override;

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Transfer")
	void Server_OnItemDrop(FItemDropData DropData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Transfer")
	void HandleItemDrop(FItemDropData DropData);

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
	UInventoryBase* GetPawnMainInventory() const { return MainPawnInventoryRef; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Settings")
	FUISettings GetUISettings() const { return UISettings; }

	virtual FInventoryModifierState GetInventoryModifierStates_Implementation() const override
	{
		return InventoryModifierState;
	}

	//
	UFUNCTION(Category = "Inventory|UI")
	void SetUIProvider(const TScriptInterface<IInvUIProvider>& NewUIProvider)
	{
		UIInvProvider = NewUIProvider;
		OnRep_StorageSession();
		QueueInventoryUIRefresh();
	}

	UFUNCTION(Category = "Inventory|UI")
	void SetInteractionUIProvider(const TScriptInterface<IInteractionUIProvider>& NewUIProvider)
	{
		InteractionUIProvider = NewUIProvider;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenSecondaryInventory(UInventoryBase* Inv, EInteractableType InteractableType);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseSecondaryInventory(EInteractableType InteractableType);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HandleCraftInventoriesChanged(const FLinkedCraftInventories& Current);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindEvents();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInventoryEvents(UInventoryBase* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BindWorldDropZoneEvents();

protected:
	// Runtime
	// Properties and variables
	TWeakObjectPtr<class UEnhancedInputComponent> ControlsHelpInput;

	uint32 ControlsHelpBindingHandle = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UInteractableComponent> TrackedInteractionTarget;

	EInteractionType TrackedInteractionType = EInteractionType::Primary;

	FTimerHandle FallbackInteractionRangeTimer;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_StorageSession)
	FStorageAccessSession StorageSession;

	UPROPERTY(Transient)
	TObjectPtr<UStorageAccessWidget> StorageWindow;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UInventoryBase>, TObjectPtr<UInventoryContainerWidget>> StorageWidgets;

	TSet<TWeakObjectPtr<AActor>> StorageRelevantActors;

	TArray<TWeakObjectPtr<UInteractableComponent>> StorageInteractionLocks;

	TSet<TWeakObjectPtr<UInventoryBase>> ObservedStorageInventories;

	bool bUpdatingStorageUI = false;

	bool bEndingPlay = false;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CharacterInventoryAccess)
	TObjectPtr<UCharacterInventoryAccessComponent> CharacterInventoryAccess;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterInventoryExchangeWidget> CharacterInventoryWindow;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventoryContainerWidget>> CharacterInventoryWidgets;

	UPROPERTY(Transient)
	TArray<FInventoryPanelCanvasPlacement> CharacterInventoryOriginalPlacements;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CharacterInventoryAccess)
	uint32 CharacterInventoryRevision = 0;

	uint32 ClosedCharacterInventoryRevision = 0;

	bool bApplyingEquipmentTransfer = false;

	FTimerHandle CharacterInventoryUITimer;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryCollectionComponent> ObservedCharacterCollection;

	FTimerHandle StorageUIRefreshTimer;

	FTimerHandle InventoryInitTimer;

	uint32 ClosedStorageRevision = 0;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_TradeSessionItems)
	FTradeSessionItems TradeSessionItems;

	bool bClosingSecondaryInventory = false;

	FTimerHandle ReplicatedInventoryUITimer;

	// Startup
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<FInventoryStartupData> StartupInventories;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TMap<TObjectPtr<UInventoryBase>, FInitItemsList> StartingItems;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory")
	TArray<UInventoryBase*> InventoryWidgetInitMap;

	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FUISettings UISettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Settings")
	FInventoryModifierState InventoryModifierState;

	//
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<IInvUIProvider> UIInvProvider;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TScriptInterface<IInteractionUIProvider> InteractionUIProvider;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<ILootContainerProvider> LootContainerProvider;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<ICraftProvider> CraftProvider;

	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite, Replicated)
	TScriptInterface<IVendorProvider> VendorProviderCurrent;

	// Refs
	UPROPERTY(Category = "Invenza|Inventory", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<APawn> OwnerPawnRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	TObjectPtr<UInventoryBase> MainPawnInventoryRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UCraftingComponent> PawnCraftingComponentRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite, Replicated)
	TObjectPtr<UCraftingComponent> ActiveCraftComponentRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryCollectionComponent> ItemCollectionRef;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadOnly)
	TObjectPtr<UInvenzaInventorySettingsAsset> GlobalSettings;

	//
	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UObject> PendingContextItem = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> PendingContextInv = nullptr;

	UPROPERTY(Category = "Invenza|Inventory", VisibleInstanceOnly, BlueprintReadWrite)
	FGuid PendingContextSlotGuid;

	// Functions
	void TrackInventoryInteraction(UInteractableComponent* Target, EInteractionType Type);

	bool TryApplyLinkedStorageTransfer(FItemMoveData Move);
	void NormalizeLinkedStorageTransferOwnership(const FItemMoveData& Move);

	bool TryApplyEquipmentTransfer(FItemMoveData ItemMoveData, bool bOnlyCheck = false, bool* OutCanTransfer = nullptr);

	UFUNCTION()
	void OnRep_CharacterInventoryAccess();

	void RefreshCharacterInventoryUI();

	void ClearCharacterInventoryUI();

	UFUNCTION(Server, Reliable)
	void Server_CloseCharacterInventory(UCharacterInventoryAccessComponent* ExpectedAccess, uint32 Revision);

	void CheckInventoryInteractionRange();

	void RefreshStorageUI();

	void OpenStorageAccess(UStorageAccessComponent* AccessPoint);

	void EndStorageAccess();

	void ClearStorageUI();

	void UpdateStorageRelevance();

	UFUNCTION()
	void OnRep_StorageSession();

	void ReleaseStorageInteractionLocks();

	UFUNCTION()
	void HandleStorageListChanged();

	UFUNCTION(Server, Reliable)
	void Server_CloseStorageAccess(uint32 Revision);

	UFUNCTION(Server, Reliable)
	void Server_RefreshStorageAccess(uint32 Revision);

	UFUNCTION()
	void OnRep_TradeSessionItems();

	void ResetTradeSession(UInventoryBase* Inventory = nullptr);

	void EndTradeSession();

	UFUNCTION(Server, Reliable)
	void Server_CloseTradeSession(uint32 SessionId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateInventories();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateWidgetsForInventories();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryContainerWidget* CreateInventoryWidget(UInventoryBase* InvToLink);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitInvWidgets();

	UFUNCTION()
	void QueueInventoryUIRefresh();

	void RefreshReplicatedInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitCraftWidgets();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryBase* ResolveTargetInventory(UInventoryBase* SourceInventory);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	bool FindSuitableEquipmentSlot(UObject* Item, UInventoryBase*& OutEquipmentInventory,
								   UInventorySlotData*& OutSuitableSlot, UInventorySlotData*& OutFreeSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Trade")
	virtual void VendorRequest(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractRequest(UInteractableComponent* TargetInteractableComponent, EInteractionType Type);

	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleInteract(UInteractableComponent* Target, EInteractionType Type);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandlePickupInteraction(UInteractableComponent* Target);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleContainerInteraction(UInteractableComponent* Target);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleTradeInteraction(UInteractableComponent* Target);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleCraftStationInteraction(UInteractableComponent* Target, EInteractionType Type);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void InteractClearRequest(UInteractableComponent* TargetInteractableComponent, EInteractionType Type);

	UFUNCTION(Server, Reliable, Category = "Inventory|Interaction")
	void Server_HandleClearInteract(UInteractableComponent* Target, EInteractionType Type);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void HandleClearInteraction(UInteractableComponent* Target, EInteractionType Type);

	UFUNCTION(Server, Reliable)
	void Server_ClientClosedUI(EInteractableType InteractableType);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Interaction")
	void BindInteractionWidget();

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

	UFUNCTION()
	void HandleToggleCraftMenu();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void InitializeBindings();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Initialization")
	void BindInputActions();

	UFUNCTION()
	void SetInteractionOwnership(AActor* TargetActor, bool bTake);

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	TMap<EObjectInteractionType, FModalActionConfig> CollectAccessibleInventoryActions();

	UFUNCTION(Category = "Invenza|Inventory", BlueprintCallable)
	void OnInventoryModalResponse(FModalResult Result);
};
