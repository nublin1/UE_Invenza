//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStructures.h"
#include "Data/Interactable/PickupTypes.h"
#include "Engine/EngineTypes.h"
#include "Interface/Interaction/Pickupableass.h"
#include "Interface/Interaction/LootContainerProvider.h"
#include "PickupComponent.generated.h"

struct FInitItemsEntry;
class UObject;
class UBoxComponent;
class UIInventoryManager;
class UStaticMesh;
class UMaterialInterface;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UPickupComponent : public UInteractableComponent, public IPickupableass, public ILootContainerProvider
{
	GENERATED_BODY()

public:
	UPickupComponent();

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

public:
	// Properties and variables
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pickup|Save")
	FGuid PersistentPickupID;

	/** Nonempty list enables bundle pickup. InitialItem remains the legacy single-item definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Contents")
	TArray<FInitItemsEntry> InitialItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Appearance")
	EPickupMeshSource MeshSource = EPickupMeshSource::FirstItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Appearance", meta = (EditCondition = "MeshSource == EPickupMeshSource::CustomMesh"))
	TObjectPtr<UStaticMesh> CustomMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Appearance", meta = (EditCondition = "MeshSource == EPickupMeshSource::CustomMesh"))
	TArray<TObjectPtr<UMaterialInterface>> CustomMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Appearance", meta = (EditCondition = "MeshSource == EPickupMeshSource::CustomMesh"))
	FVector CustomMeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Appearance", meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.StaticMeshComponent"))
	FComponentReference VisualMeshComponent;

	/** Local offset from the source actor for resources that do not fit into storage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Contents")
	FVector OverflowDropOffset = FVector(80.f, 0.f, 40.f);

	// Functions
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginFocus() override;

	virtual void EndFocus() override;

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Pickup | Initialization")
	virtual void InitializeDrop(FInitItemsEntry ItemToDrop);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Pickup|Initialization")
	void InitializeItems(const TArray<FInitItemsEntry>& Items);

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	FInitItemsEntry GetInitialItem() { return InitialItem; }

	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	virtual UObject* GetItemData() override;

	UObject* PeekItemData() const { return ItemBase; }
	virtual const TObjectPtr<UInventoryBase>& GetMainLootContainer() const override { return PortableContainer; }
	void RefreshPortableContainer();

	void RestorePersistentItem(UObject* Item);
	void RestorePersistentItems(const TArray<UObject*>& Items, bool bBundle);

	UFUNCTION(BlueprintPure, Category = "Pickup|Contents")
	TArray<UObject*> GetPickupItems() const;

	UFUNCTION(BlueprintPure, Category = "Pickup|Contents")
	bool IsBundlePickup() const { return bBundlePickup; }

	bool IsCollecting() const { return bCollecting; }
	bool HasInitializedContents() const { return bContentsInitialized || IsValid(ItemBase); }
	bool CollectItems(UIInventoryManager* Manager);

	UFUNCTION(BlueprintCallable, Category = "Pickup|Appearance")
	void RefreshAppearance();

	FString GetPersistentSaveKey() const;

	void SetPersistentSaveKey(const FString& Key, bool bRuntime)
	{
		PersistentSaveKey = Key;
		bRuntimeSaveActor = bRuntime;
	}

	bool IsRuntimeSaveActor() const { return bRuntimeSaveActor; }

	bool HasAssignedSaveKey() const { return !PersistentSaveKey.IsEmpty(); }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void OnPickedUp() override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;

	FVector AuthoredMeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Item Initialization")
	FInitItemsEntry InitialItem;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_ItemBase, VisibleAnywhere, BlueprintReadOnly,
			  Category = "Pickup | Item Reference")
	TObjectPtr<UObject> ItemBase;

	UPROPERTY(ReplicatedUsing = OnRep_ItemBase)
	TObjectPtr<UInventoryBase> PortableContainer;

	FString PersistentSaveKey;

	bool bRuntimeSaveActor = false;

	bool bIsPendingDestruction = false;

	UPROPERTY(ReplicatedUsing = OnRep_ItemBase)
	TArray<TObjectPtr<UObject>> PickupItems;

	UPROPERTY(ReplicatedUsing = OnRep_ItemBase)
	bool bBundlePickup = false;

	bool bContentsInitialized = false;
	bool bCollecting = false;
	bool bAuthoredInteractionCached = false;
	FInteractableData AuthoredPrimaryData;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> AuthoredMaterials;

	// Functions
	void CacheVisualMesh();
	void ContentsChanged();

	UFUNCTION()
	void InitializePickupComponent();

	UFUNCTION()
	void OnRep_ItemBase();

	UFUNCTION()
	void ItemVisualDataChanged(UObject* Item);

	virtual void UpdateInteractableData() override;
};
