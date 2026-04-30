//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryReplicationTypes.generated.h"

class UIInventoryManager;
class UItemBase;

USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemBase> Item = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FItemMappingArrayWrapper Locations;
	
	void PostReplicatedAdd(const struct FInventoryArray& InArraySerializer);
	void PostReplicatedChange(const struct FInventoryArray& InArraySerializer);
	void PreReplicatedRemove(const struct FInventoryArray& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FInventoryEntry> Items;
	UPROPERTY(NotReplicated)
	TObjectPtr<UIInventoryManager> OwningManager;
	UPROPERTY(NotReplicated)
	TObjectPtr<UItemCollection> OwningCollection;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryArray>(Items, DeltaParms, *this);
	}

	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};