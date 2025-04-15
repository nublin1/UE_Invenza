//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "AUIManagerActor.generated.h"


enum class EInteractableType : uint8;
class UItemCollection;
struct FItemMoveData;
struct FInputActionInstance;
class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;

UCLASS(ClassGroup=(Custom), Blueprintable)
class GRIDINVENTORYPLUGIN_API AUIManagerActor  : public AActor
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	AUIManagerActor();

	UFUNCTION(BlueprintCallable)
	void OnQuickTransferItem(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable)
	FItemAddResult ItemTransferRequest(FItemMoveData ItemMoveData);

	UInvBaseContainerWidget* GetMainInventory() {return CoreHUDWidget->GetMainInvWidget();}

	FUISettings GetUISettings() {return UISettings;}
	UCoreHUDWidget* GetCoreHUDWidget() {return CoreHUDWidget;}
	FInventoryModifierState GetInventoryModifierStates() const {return InventoryModifierState;}

	UFUNCTION(BlueprintCallable)
	void SetInteractableType(EInteractableType InteractableType);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI)
	UCoreHUDWidget* CoreHUDWidget;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInvBaseContainerWidget> CurrentInteractInvWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	FUISettings UISettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FRegularSettings RegularSettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FInventoryModifierState InventoryModifierState;
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void BindEvents(AActor* TargetActor);
	UFUNCTION(BlueprintCallable)
	void UIIteract(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION()
	void OnQuickGrabPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnQuickGrabReleased(const FInputActionInstance& Instance);
	UFUNCTION(BlueprintCallable)
	void InitializeMenuBindings();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction);
};
