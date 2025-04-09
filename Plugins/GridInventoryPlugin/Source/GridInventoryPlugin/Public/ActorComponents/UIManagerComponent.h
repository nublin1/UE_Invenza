//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UIManagerComponent.generated.h"


class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UUIManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UUIManagerComponent();

	FUISettings GetUISettings() {return UISettings;}
	UCoreHUDWidget* GetCoreHUDWidget() {return CoreHUDWidget;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI)
	UCoreHUDWidget* CoreHUDWidget;
	
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	UInputAction* ToggleMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	FUISettings UISettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FRegularSettings RegularSettings;

	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void BindEvents(AActor* TargetActor);
	UFUNCTION(BlueprintCallable)
	void UIIteract(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION(BlueprintCallable)
	void InitializeMenuBindings();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
