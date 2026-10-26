// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "InvenzaExamplePlayerController.generated.h"

/** Enhanced Input belongs to this example controller; host input settings remain untouched. */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API AInvenzaExamplePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Functions
	AInvenzaExamplePlayerController();

protected:
	// Unreal overrides
	virtual void SetupInputComponent() override;
};
