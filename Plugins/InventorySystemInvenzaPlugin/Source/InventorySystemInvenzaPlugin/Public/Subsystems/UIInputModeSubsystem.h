#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIInputModeSubsystem.generated.h"

/** Arbitrates UI input requests independently for each local player. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUIInputModeSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** Repeated requests from the same owner are idempotent. */
	UFUNCTION(BlueprintCallable, Category="UI|Input")
	void RequestUIInput(UObject* RequestOwner);

	UFUNCTION(BlueprintCallable, Category="UI|Input")
	void ReleaseUIInput(UObject* RequestOwner);

	virtual void Deinitialize() override;

private:
	TSet<TWeakObjectPtr<UObject>> RequestOwners;
	TWeakObjectPtr<APlayerController> AppliedController;
	bool bUIInputApplied = false;

	void UpdateInputMode();
};
