// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableComponent.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "InfoInteractableComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInfoInteractableComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	UInfoInteractableComponent();

	virtual void HandleInteract(UInteractionComponent* InteractionComponent) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,	Category="Interactable|Information", meta=(MultiLine=true))
	FText InformationText;

	UFUNCTION()
	void HandleModalResult(FModalResult Result);
	
	virtual void UpdateInteractableData() override;

private:
	bool bModalOpen = false;
};
