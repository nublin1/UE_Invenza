// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "InvenzaExamplePackagingLibrary.generated.h"

class USkeletalMesh;
class UBlueprint;
class UAnimInstance;

/** Packaging-only repairs on migrated copies, never original /Game assets. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaExamplePackagingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Example Packaging")
	static bool RefreshMigratedPreview(UObject* Asset, USkeletalMesh* PreviewMesh);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Example Packaging")
	static bool ConfigureExampleCharacter(UBlueprint* Blueprint, USkeletalMesh* Mesh,
		TSubclassOf<UAnimInstance> AnimationClass, TEnumAsByte<ECollisionChannel> TraceChannel);
};
