//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "FrontendMenuSettings.generated.h"

class UFrontendMenuWidget;
class UStringTable;
class UPrimaryLayout;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Invenza Main Menu"))
class UE_GRIDINVENTORY_API UFrontendMenuSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// Properties and variables
	UPROPERTY(Config, EditAnywhere, Category = "Widgets")
	TSoftClassPtr<UFrontendMenuWidget> MenuClass;
	UPROPERTY(Config, EditAnywhere, Category = "Input")
	FKey MenuKey = EKeys::M;
	UPROPERTY(Config, EditAnywhere, Category = "Widgets")
	TSoftClassPtr<UPrimaryLayout> LayoutClass;
	UPROPERTY(Config, EditAnywhere, Category = "Navigation")
	TSoftObjectPtr<UWorld> PlayWorld;
	UPROPERTY(Config, EditAnywhere, Category = "Localization")
	TSoftObjectPtr<UStringTable> TextTable;
	// Functions
	UFUNCTION(BlueprintPure, Category = "Frontend")
	FText GetText(FName Key) const;
};
