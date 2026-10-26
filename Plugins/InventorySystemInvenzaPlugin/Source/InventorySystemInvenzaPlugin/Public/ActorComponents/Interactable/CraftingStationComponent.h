// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Interface/Interaction/CraftProvider.h"
#include "CraftingStationComponent.generated.h"

class UWidgetComponent;
class UCraftingComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingStationComponent : public UInteractableComponent, public ICraftProvider
{
	GENERATED_BODY()

public:
	UCraftingStationComponent();

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

public:
	// Functions
	virtual UCraftingComponent* GetCraftingComponent() const override { return CraftingComponentLink; }

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	bool bUseInteractorInventory = false;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CraftingComponentLink)
	TObjectPtr<UCraftingComponent> CraftingComponentLink = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryCollectionComponent> ItemCollectionRef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting|UI")
	TObjectPtr<UWidgetComponent> ProgressWidgetComponentLink;

	// Functions
	UFUNCTION()
	void OnRep_CraftingComponentLink();

	UFUNCTION()
	void BindProgressWidget();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void InitializeCraftingStation(AActor* ContextActor);

	virtual void InitializeInteractionComponent() override;

	virtual void UpdateInteractableData() override;
};
