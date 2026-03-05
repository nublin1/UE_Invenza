// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UInstancedStaticMeshComponent* StaticMeshVisual;

	//
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UItemBase* ItemBase;
	
	UFUNCTION()
	void UpdateVisual();
};
