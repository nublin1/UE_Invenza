// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIInputModeSubsystem.generated.h"

/** Arbitrates UI input requests independently for each local player. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUIInputModeSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void Deinitialize() override;

public:
	// Functions
	/** Repeated requests from the same owner are idempotent. */
	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void RequestUIInput(UObject* RequestOwner);

	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void ReleaseUIInput(UObject* RequestOwner);

protected:
	// Runtime
	// Properties and variables
	TSet<TWeakObjectPtr<UObject>> RequestOwners;

	TWeakObjectPtr<APlayerController> AppliedController;

	bool bUIInputApplied = false;

	// Functions
	void UpdateInputMode();
};
