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

USTRUCT(BlueprintType)
struct FObjectModalActionRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Rules")
	EObjectConditionType Condition = EObjectConditionType::AlwaysAvailable;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Rules")
	FGameplayTag ActionTag;
};

UCLASS(BlueprintType, Blueprintable)
class UModalAction : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FText DisplayText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UModalAction()
	: ActionTag()
	, DisplayText(FText::GetEmpty())
	, Icon(nullptr)
	{}
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("ModalAction"), GetFName());
	}
};

USTRUCT(BlueprintType)
struct FModalResult
{
	GENERATED_BODY()
 
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	FGameplayTag ResultTag;
	
	// Choice only
	UPROPERTY(BlueprintReadOnly, Category = "Modal")
	int32 ChoiceIndex = -1;
	
};