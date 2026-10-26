//  Nublin Studio 2026 All Rights Reserved.

#pragma once
#include "Subsystems/LocalPlayerSubsystem.h"
#include "FrontendMenuSubsystem.generated.h"

class UFrontendMenuWidget;
class UInputComponent;

UCLASS()
class UE_GRIDINVENTORY_API UFrontendMenuSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
protected:
	virtual void PlayerControllerChanged(APlayerController* NewController) override;
	virtual void Deinitialize() override;

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Frontend")
	void ShowMenu();
	UFUNCTION(BlueprintCallable, Category = "Frontend")
	void CloseMenu();

protected:
	// Runtime
	UPROPERTY(Transient)
	TObjectPtr<UFrontendMenuWidget> Menu;
	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> MenuInputComponent;
	TWeakObjectPtr<APlayerController> Controller;
	// Functions
	void InitializeMenuAccess();
	void RemoveMenuInput();
	void ToggleMenu();
};
