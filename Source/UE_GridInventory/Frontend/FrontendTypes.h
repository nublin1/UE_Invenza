//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FrontendTypes.generated.h"

USTRUCT(BlueprintType)
struct FInvenzaServerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	FString Map;

	UPROPERTY(BlueprintReadOnly)
	int32 Players = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Capacity = 0;
};

enum class EInvenzaSessionOperation : uint8
{
	None,
	Create,
	Find,
	Join,
	Destroy,
	Travel
};

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFrontendPageBackRequested);
#pragma endregion Delegates

UENUM(BlueprintType)
enum class EFrontendPage : uint8
{
	Home,
	Settings,
	Network,
	Save,
	Load
};
