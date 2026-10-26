//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStructures.h"
#include "Interface/Interaction/Pickupableass.h"
#include "PickupComponent.generated.h"

struct FInitItemsEntry;
class UObject;
class UBoxComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UPickupComponent : public UInteractableComponent, public IPickupableass
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

	// Functions
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginFocus() override;

	virtual void EndFocus() override;

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Pickup | Initialization")
	virtual void InitializeDrop(FInitItemsEntry ItemToDrop);

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	FInitItemsEntry GetInitialItem() { return InitialItem; }

	UFUNCTION(BlueprintCallable, Category = "Pickup | Getters")
	virtual UObject* GetItemData() override;

	UObject* PeekItemData() const { return ItemBase; }

	void RestorePersistentItem(UObject* Item);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup | Item Initialization")
	FInitItemsEntry InitialItem;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_ItemBase, VisibleAnywhere, BlueprintReadOnly,
			  Category = "Pickup | Item Reference")
	TObjectPtr<UObject> ItemBase;

	FString PersistentSaveKey;

	bool bRuntimeSaveActor = false;

	bool bIsPendingDestruction = false;

	// Functions
	UFUNCTION()
	void InitializePickupComponent();

	UFUNCTION()
	void OnRep_ItemBase();

	virtual void UpdateInteractableData() override;
};
