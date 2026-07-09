// Nublin Studio 2026 All Rights Reserved.

#pragma once
 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModalTypes.generated.h"

UENUM(BlueprintType)
enum class EObjectConditionType : uint8
{
	AlwaysAvailable,
	IfStackable,
	IfCanBeUsed,
};

UENUM(BlueprintType)
enum class EObjectInteractionType : uint8
{
	None,
	UseItem,
	Drop,
	Destroy,
	Split
};

USTRUCT(BlueprintType)
struct FObjectModalAction
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	EObjectInteractionType ObjectInteractionType = EObjectInteractionType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	EObjectConditionType Condition = EObjectConditionType::AlwaysAvailable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	FGameplayTag ActionTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FText DisplayText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	TObjectPtr<UTexture2D> Icon = nullptr;
};

USTRUCT(BlueprintType)
struct FModalResult
{
	GENERATED_BODY()
 
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FObjectModalAction ResultAction;
	
	// Choice only
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	int32 ChoiceIndex = -1;
	
};