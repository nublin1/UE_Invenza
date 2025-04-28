//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/Interactable/InteractableComponent.h"
#include "VendorComponent.generated.h"

class UInvBaseContainerWidget;
class UUInventoryWidgetBase;
class UItemCollection;

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UVendorComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================


	//====================================================================
	// FUNCTIONS
	//====================================================================
	UVendorComponent();
	
	virtual void BeginFocus() override;
	virtual void EndFocus() override;
	
	virtual void Interact(UInteractionComponent* InteractionComponent) override;
	virtual void StopInteract(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY()
	bool bIsInteract = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;

	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;
	
};
