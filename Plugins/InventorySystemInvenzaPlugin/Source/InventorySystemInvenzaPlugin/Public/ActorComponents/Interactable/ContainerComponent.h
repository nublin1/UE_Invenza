//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "ActorComponents/ItemCollection.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "Interface/Interaction/LootContainerProvider.h"
#include "ContainerComponent.generated.h"

class UInvenzaBaseWidget;
class UInventoryContainerWidget;
/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UContainerComponent : public UInteractableComponent, public ILootContainerProvider
{
	GENERATED_BODY()

public:
	UContainerComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;
	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;

	virtual const TObjectPtr<UInventoryBase>& GetMainLootContainer() const override {return MainLootInventory;}
	UFUNCTION()
	virtual void CheckDestroyWhenEmpty() override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	TArray<FInventoryStartupData> StartupInventories;

	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	FGameplayTag MainLootContainerInvTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Container")
	bool bDestroyWhenEmpty = false;

	// Data
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryBase> MainLootInventory;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedMesh;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Inventory")
	TMap<TObjectPtr<UInventoryBase>, FInitItemsList> StartingItems;

	//Refs
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Inventory|Container")
	TObjectPtr<UItemCollection> ItemCollectionRef;

	//====================================================================
	// FUNCTIONS
	//====================================================================	
	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;

	UFUNCTION(BlueprintCallable)
	void InitializeInventoryStartupData();

	UFUNCTION(BlueprintCallable)
	void SetupStartingResources();
		
	
	UFUNCTION(Server, Reliable)
	virtual void Server_DestroyWhenEmpty();
};
