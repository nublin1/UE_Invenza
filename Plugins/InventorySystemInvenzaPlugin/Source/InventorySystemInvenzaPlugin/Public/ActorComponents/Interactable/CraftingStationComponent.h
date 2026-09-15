// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "ActorComponents/ItemCollection.h"
#include "Interface/Interaction/CraftProvider.h"
#include "CraftingStationComponent.generated.h"


class UWidgetComponent;
class UCraftingComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingStationComponent : public UInteractableComponent, public ICraftProvider
{
	GENERATED_BODY()

public:
	UCraftingStationComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	
protected:

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	virtual UCraftingComponent* GetCraftingComponent() const override { return CraftingComponentLink; }
	
	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;
	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting")
	bool bUseInteractorInventory = false;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CraftingComponentLink)
	TObjectPtr<UCraftingComponent> CraftingComponentLink = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UItemCollection> ItemCollectionRef = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crafting|UI")
	TObjectPtr<UWidgetComponent> ProgressWidgetComponentLink;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	void OnRep_CraftingComponentLink();
	
	UFUNCTION()
	void BindProgressWidget();
	
	UFUNCTION(BlueprintCallable, Category="Crafting")
	void InitializeCraftingStation(AActor* ContextActor);
	
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

};
