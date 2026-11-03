// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PortableInventoryState.generated.h"

class UInventoryCollectionComponent;
class UInventoryBase;
class UItemBase;

/** Stable network/save owner for a portable container, independent of the item actor's lifetime. */
UCLASS(NotBlueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API APortableInventoryState : public AActor
{
	GENERATED_BODY()

public:
	APortableInventoryState();

protected:
	// Unreal overrides
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
		const FVector& SrcLocation) const override;

public:
	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portable Inventory")
	TObjectPtr<UInventoryCollectionComponent> Collection;

	// Functions
	bool InitializeForItem(UItemBase* Item);
	UInventoryBase* GetInventory() const;
	bool HasContents() const;
	bool HasCraftObligations() const;
	static APortableInventoryState* FindForItem(const UItemBase* Item);
	bool SetItemIdentity(const FGuid& ID);
	void SetCarrier(AActor* Actor);
	bool CanReplicateTo(const AActor* RealViewer) const;
	bool DestroyPermanently();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(ReplicatedUsing = OnRep_ItemIdentity)
	FGuid ItemIdentity;

	TWeakObjectPtr<AActor> Carrier;

	// Functions
	UFUNCTION()
	void OnRep_ItemIdentity();
};
