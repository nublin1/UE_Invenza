//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HUD/GameHUD_Inz.h"
#include "FrontendMenuGameMode.generated.h"

UCLASS()
class UE_GRIDINVENTORY_API AFrontendMenuHUD : public AGameHUD_Inz
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};

UCLASS()
class UE_GRIDINVENTORY_API AFrontendMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFrontendMenuGameMode();
};
