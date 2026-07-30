// Nublin Studio 2026 All Rights Reserved.

#pragma once
 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModalTypes.generated.h"


UENUM(BlueprintType)
enum class EModalHeaderType : uint8
{
	None,
	SimpleText
};

UENUM(BlueprintType)
enum class EModalFooterType : uint8
{
	Notification, // ОК
	Binary,
	Confirmation, // Да / Нет / Отмена
	Selection     // Список вариантов 
};

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
	Yes,
	No,
	Cancel,
	UseItem,
	Drop,
	Destroy,
	Split
};

UENUM(BlueprintType)
enum class EModalStepRequirement : uint8
{
	None,
	RequiresConfirm,
	RequiresAmount
};

USTRUCT(BlueprintType)
struct FObjectModalAction
{
	GENERATED_BODY()
		
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	//FGameplayTag ActionTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal|Flow")
	EModalStepRequirement StepRequirement = EModalStepRequirement::None;
};

USTRUCT(BlueprintType)
struct FModalAction
{
	GENERATED_BODY()
	
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
	EObjectInteractionType ResultInteractionType;
	
	//UPROPERTY(BlueprintReadOnly, Category = "Modal")
	//int32 ChoiceIndex = -1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal|Flow")
	EModalStepRequirement StepRequirement = EModalStepRequirement::None;
	
};

#pragma region Delegates
DECLARE_DYNAMIC_DELEGATE_OneParam(FModalResultDelegate, FModalResult, Result);
#pragma endregion Delegates