// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "StorageVisualRepresentation.generated.h"

class UItemBase;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API AStorageVisualRepresentation : public AActor
{
	GENERATED_BODY()

public:
	AStorageVisualRepresentation();

protected:
	// Unreal overrides
	virtual void BeginPlay() override;

public:
	// Properties and variables
	UPROPERTY(Category = "Invenza|Storage", EditAnywhere, BlueprintReadWrite)
	UInstancedStaticMeshComponent* StaticMeshVisual;

	//
	UPROPERTY(Category = "Invenza|Storage", EditAnywhere, BlueprintReadWrite)
	UItemBase* ItemBase;

	// Functions
	UFUNCTION()
	void UpdateVisual();
};
