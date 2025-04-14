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
UCLASS()
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
	
	UFUNCTION()
	virtual void Interact(UInteractionComponent* InteractionComponent) override;

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemCollection> ItemCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContainerWidgetName;
	UPROPERTY()
	TObjectPtr<UInvBaseContainerWidget> ContainerWidget;

	//Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuyPriceFactor = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SellPriceFactor = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RemoveItemAfterPurchase = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSellOnly = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void OnRegister() override;

	virtual void InitializeInteractionComponent() override;
	virtual void UpdateInteractableData() override;
	
	UFUNCTION(BlueprintCallable)
	virtual UUInventoryWidgetBase* FindContainerWidget();
};
