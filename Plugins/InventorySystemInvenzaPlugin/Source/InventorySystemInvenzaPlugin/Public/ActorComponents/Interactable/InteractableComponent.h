//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable/InteractableData.h"
#include "InteractableComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	virtual const FInteractableData& GetInteractableData() const {return InteractableData;}
	
	UFUNCTION(BlueprintCallable, Category="Interactable|Focus")
	virtual void BeginFocus();
	UFUNCTION(BlueprintCallable, Category="Interactable|Focus")
	virtual void EndFocus();
	
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void BeginInteract(UInteractionComponent* InteractionComponent);
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void EndInteract(UInteractionComponent* InteractionComponent);
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void Interact(UInteractionComponent* InteractionComponent);
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void StopInteract(UInteractionComponent* InteractionComponent);

	UFUNCTION(BlueprintPure, Category="Interactable|State")
	bool IsInteracting() const {return bIsInteracting;}

	UFUNCTION(BlueprintCallable, Category="Interactable|State")
	virtual void SetInteracting(bool NewState);
	

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interactable|Data")
	FInteractableData InteractableData;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Interactable")
	bool bIsInteracting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Interactable|State")
	TObjectPtr<UInteractionComponent> CurrentInteractionComponent = nullptr;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category="Interactable|Internal")
	virtual void InitializeInteractionComponent();
	UFUNCTION(BlueprintCallable, Category="Interactable|Internal")
	virtual void UpdateInteractableData();
	
};
