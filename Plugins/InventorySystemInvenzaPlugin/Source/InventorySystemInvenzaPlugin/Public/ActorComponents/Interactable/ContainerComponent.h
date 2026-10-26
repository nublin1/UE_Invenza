//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Interface/Interaction/LootContainerProvider.h"
#include "ContainerComponent.generated.h"

class UInvenzaBaseWidget;
class UInventoryContainerWidget;
/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UContainerComponent : public UInteractableComponent,
															 public ILootContainerProvider
{
	GENERATED_BODY()

public:
	UContainerComponent();

protected:
	// Unreal overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRegister() override;

	virtual void BeginPlay() override;

public:
	// Functions
	virtual void BeginFocus() override;

	virtual void EndFocus() override;

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;

	virtual const TObjectPtr<UInventoryBase>& GetMainLootContainer() const override { return MainLootInventory; }

	UFUNCTION()
	virtual void CheckDestroyWhenEmpty() override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	TArray<FInventoryStartupData> StartupInventories;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	FGameplayTag MainLootContainerInvTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	bool bDestroyWhenEmpty = false;

	// Data
	UPROPERTY(Category = "Invenza|Interaction", ReplicatedUsing = OnRep_MainLootInventory, VisibleAnywhere,
			  BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainLootInventory;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	TMap<TObjectPtr<UInventoryBase>, FInitItemsList> StartingItems;

	//Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Inventory|Container")
	TObjectPtr<UInventoryCollectionComponent> ItemCollectionRef;

	// Functions
	virtual void InitializeInteractionComponent() override;

	virtual void UpdateInteractableData() override;

	UFUNCTION()
	virtual void OnRep_MainLootInventory();

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	void InitializeInventoryStartupData();

	UFUNCTION(Category = "Invenza|Interaction", BlueprintCallable)
	void SetupStartingResources();

	UFUNCTION(Server, Reliable)
	virtual void Server_DestroyWhenEmpty();
};
