// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "ActorComponents/ItemCollection.h"
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
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	bool bUseInteractorInventory = false;

	UPROPERTY(Transient)
	TObjectPtr<UCraftingComponent> CraftingComponentRef = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UItemCollection> ItemCollectionRef = nullptr;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void InitializeCraftingStation(AActor* ContextActor);
	
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

};
