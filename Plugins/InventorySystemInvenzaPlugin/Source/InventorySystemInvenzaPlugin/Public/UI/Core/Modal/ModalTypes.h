// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "ModalTypes.generated.h"

UENUM(BlueprintType)
enum class EModalHeaderType : uint8
{
	None,
	SimpleText,
	TextWithAmountSelection
};

UENUM(BlueprintType)
enum class EModalFooterType : uint8
{
	None,
	Notification, // OK
	Binary,
	Confirmation, // Yes / No / Cancel
	Selection	  // A list of choices.
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
	Ok,
	Yes,
	No,
	Cancel,
	UseItem,
	Drop,
	Destroy,
	Split,
	Equip,
	UnEquip,
	Sell,
	Buy
};

UENUM(BlueprintType)
enum class EModalStepRequirement : uint8
{
	None,
	RequiresConfirm,
	RequiresAmount
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FModalActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Flow")
	FText HeaderText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Flow")
	EModalStepRequirement StepRequirement = EModalStepRequirement::None;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FModalAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	FText DisplayText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal")
	TObjectPtr<UTexture2D> Icon = nullptr;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FModalHeaderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	EModalHeaderType HeaderType = EModalHeaderType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	bool bShowMinValue = false;
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FText MinValueLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	float MinValue = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	bool bShowMaxValue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FText MaxValueLabel;
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	float MaxValue = 1.f;

	FModalHeaderData() {}

	FModalHeaderData(EModalHeaderType HeaderType, FText InTitle) : HeaderType(HeaderType), Title(InTitle) {}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FModalHeaderResult
{
	GENERATED_BODY()

	UPROPERTY(Category = "Invenza|UI|Modal", BlueprintReadWrite)
	float SelectedAmount = 0;

	UPROPERTY(Category = "Invenza|UI|Modal", BlueprintReadWrite)
	FText EnteredText;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FModalResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Modal")
	EObjectInteractionType ResultInteractionType = EObjectInteractionType::None;

	UPROPERTY(Category = "Invenza|UI|Modal", BlueprintReadWrite)
	FModalHeaderResult HeaderResult;
};

#pragma region Delegates
DECLARE_DYNAMIC_DELEGATE_OneParam(FModalResultDelegate, FModalResult, Result);
#pragma endregion Delegates