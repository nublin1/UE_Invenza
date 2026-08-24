// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "CraftingStationComponent.generated.h"


class UCraftingComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingStationComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	UCraftingStationComponent();

protected:
	virtual void BeginPlay() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	bool bUseInteractorInventory = false;

	UPROPERTY(Transient)
	TObjectPtr<UCraftingComponent> CraftingComponent = nullptr;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category="Crafting")
	bool InitializeCraftingStation();
	
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

};
