// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "InfoInteractableComponent.generated.h"

UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInfoInteractableComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	UInfoInteractableComponent();

public:
	// Functions
	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

	virtual void HandleStopInteract(UInteractionComponent* InteractionComponent, EInteractionType Type) override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Information", meta = (MultiLine = true))
	FText InformationText;

	TWeakObjectPtr<class UModalWindowManager> ActiveModalManager;

	bool bModalOpen = false;

	// Functions
	UFUNCTION()
	void HandleModalResult(FModalResult Result);

	virtual void UpdateInteractableData() override;
};
