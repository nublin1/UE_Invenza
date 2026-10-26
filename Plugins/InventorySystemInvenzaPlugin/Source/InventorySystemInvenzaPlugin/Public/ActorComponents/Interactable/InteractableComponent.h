//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Interactable/InteractableData.h"
#include "InteractableComponent.generated.h"

UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

protected:
	// Unreal overrides
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Functions
	const TMap<EInteractionType, FInteractableData>& GetInteractableDataMap() const { return InteractableDataMap; }

	const FInteractableData* GetInteractableDataForType(EInteractionType Type) const
	{
		return InteractableDataMap.Find(Type);
	}

	virtual const FInteractableData& GetInteractableData() const;

	UFUNCTION(BlueprintPure, Category = "Interactable|Data")
	FText GetInteractionActionText(EInteractionType Type, bool bActiveForInteractor) const;

	UFUNCTION(BlueprintCallable, Category = "Interactable|Focus")
	virtual void BeginFocus();

	UFUNCTION(BlueprintCallable, Category = "Interactable|Focus")
	virtual void EndFocus();

	UFUNCTION(BlueprintCallable, Category = "Interactable|Interaction")
	virtual void BeginInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Interaction")
	virtual void EndInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);

	/** Server-side, actor-wide ownership shared by all interactable components. */
	virtual bool TryAcquireInteraction(AActor* Interactor);
	virtual void ReleaseInteraction(AActor* Interactor);
	virtual bool IsInteractionLockedByOther(const AActor* Interactor) const;
	virtual bool IsInteractionOwnedBy(const AActor* Interactor) const;

	virtual void HandleInteract(UInteractionComponent* InteractionComponent);

	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type);

	UFUNCTION(BlueprintPure, Category = "Interactable|State")
	bool IsInteracting() const { return bIsInteracting; }

	UFUNCTION(BlueprintCallable, Category = "Interactable|State")
	virtual void SetInteracting(bool NewState);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Data")
	TMap<EInteractionType, FInteractableData> InteractableDataMap;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Interactable")
	bool bIsInteracting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interactable|State")
	TObjectPtr<UInteractionComponent> CurrentInteractionComponent = nullptr;

	TWeakObjectPtr<AActor> ExclusiveInteractor;

	// Functions
	UInteractableComponent* GetInteractionLockComponent() const;

	UFUNCTION()
	void HandleExclusiveInteractorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	UFUNCTION(BlueprintCallable, Category = "Interactable|Internal")
	virtual void InitializeInteractionComponent();

	UFUNCTION(BlueprintCallable, Category = "Interactable|Internal")
	virtual void UpdateInteractableData();
};
