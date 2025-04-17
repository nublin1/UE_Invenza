#pragma once

#include "CoreMinimal.h"
#include "LogicChecks.generated.h"

class UItemBase;

UENUM()
enum class EInventoryCheckType : uint8
{
	PreAdd		UMETA(DisplayName = "Pre-Add Check"),
	PreTransfer UMETA(DisplayName = "PreTransfer Check"),
	PreUse		UMETA(DisplayName = "Pre-Use Check"),
	PostUse		UMETA(DisplayName = "Post-Use Check")
};

USTRUCT()
struct FInventoryCheck
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	EInventoryCheckType CheckType;
	UPROPERTY()
	FName CheckID;
	
	TFunction<bool(UItemBase*)> CheckFunction;

	FInventoryCheck(): CheckType(), CheckID(NAME_None)
	{
	}
	
	FInventoryCheck(EInventoryCheckType InCheckType,  FName InCheckID, const TFunction<bool(UItemBase*)>& InCheckFunction)
		: CheckType(InCheckType), CheckID(InCheckID), CheckFunction(InCheckFunction)
	{}
};