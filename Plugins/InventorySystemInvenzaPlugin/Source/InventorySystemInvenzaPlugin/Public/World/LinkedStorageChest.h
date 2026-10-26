//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LinkedStorageChest.generated.h"

class UStaticMeshComponent;
class ULinkedStorageComponent;

UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API ALinkedStorageChest : public AActor
{
	GENERATED_BODY()

public:
	ALinkedStorageChest();

	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Linked Storage")
	TObjectPtr<ULinkedStorageComponent> LinkedStorage;
};
