//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageAccessPoint.generated.h"

class UStaticMeshComponent;
class UStorageAccessComponent;

UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API AStorageAccessPoint : public AActor
{
	GENERATED_BODY()

public:
	AStorageAccessPoint();

public:
	// Properties and variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Storage Access")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Storage Access")
	TObjectPtr<UStorageAccessComponent> StorageAccess;
};
