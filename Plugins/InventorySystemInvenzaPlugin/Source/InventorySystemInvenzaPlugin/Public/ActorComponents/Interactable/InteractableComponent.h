//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Interactable/InteractableData.h"
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

	const TMap<EInteractionType, FInteractableData>& GetInteractableDataMap() const { return InteractableDataMap; }
	const FInteractableData* GetInteractableDataForType(EInteractionType Type) const{return InteractableDataMap.Find(Type);	}
	virtual const FInteractableData& GetInteractableData() const;
	
	UFUNCTION(BlueprintPure, Category = "Interactable|Data")
	FText GetInteractionActionText(	EInteractionType Type,bool bActiveForInteractor) const;
	
	
	UFUNCTION(BlueprintCallable, Category="Interactable|Focus")
	virtual void BeginFocus();
	UFUNCTION(BlueprintCallable, Category="Interactable|Focus")
	virtual void EndFocus();
	
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void BeginInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);
	UFUNCTION(BlueprintCallable, Category="Interactable|Interaction")
	virtual void EndInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);
	
	virtual void HandleInteract(UInteractionComponent* InteractionComponent);
	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);

	UFUNCTION(BlueprintPure, Category="Interactable|State")
	bool IsInteracting() const {return bIsInteracting;}

	UFUNCTION(BlueprintCallable, Category="Interactable|State")
	virtual void SetInteracting(bool NewState);
	

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interactable|Data")
	TMap<EInteractionType, FInteractableData> InteractableDataMap;
	
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
