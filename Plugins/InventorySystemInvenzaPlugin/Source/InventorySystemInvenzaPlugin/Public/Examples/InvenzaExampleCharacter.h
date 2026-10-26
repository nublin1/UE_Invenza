// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "InvenzaExampleCharacter.generated.h"

/** Input defaults scoped to the playable example, independent of host project settings. */
UCLASS(Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API AInvenzaExampleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Functions
	AInvenzaExampleCharacter();
};
