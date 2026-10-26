//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ControlsHelpTypes.generated.h"

class UInputAction;

/** A named action in the help window. Keys are resolved for the owning local player. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FControlsHelpEntry
{
	GENERATED_BODY()
	UPROPERTY(Category = "Invenza|UI|Controls", EditAnywhere, BlueprintReadWrite)
	FText Group;
	UPROPERTY(Category = "Invenza|UI|Controls", EditAnywhere, BlueprintReadWrite)
	FText Label;
	UPROPERTY(Category = "Invenza|UI|Controls", EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> Action;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FControlsHelpRow
{
	GENERATED_BODY()
	UPROPERTY(Category = "Invenza|UI|Controls", BlueprintReadOnly)
	FText Group;
	UPROPERTY(Category = "Invenza|UI|Controls", BlueprintReadOnly)
	FText Label;
	UPROPERTY(Category = "Invenza|UI|Controls", BlueprintReadOnly)
	FText Keys;
};
