// Nublin Studio 2026 All Rights Reserved.

#pragma once
 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModalTypes.generated.h"

UENUM(BlueprintType)
enum class EModalResultType : uint8
{
	Ok,
	Yes,
	No,
	Cancel,
	Choice
};

USTRUCT(BlueprintType)
struct FModalResult
{
	GENERATED_BODY()
 
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	EModalResultType Type = EModalResultType::Cancel;
	
	// Choice only
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	int32 ChoiceIndex = -1;
	
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FGameplayTag ChoiceTag ;
};