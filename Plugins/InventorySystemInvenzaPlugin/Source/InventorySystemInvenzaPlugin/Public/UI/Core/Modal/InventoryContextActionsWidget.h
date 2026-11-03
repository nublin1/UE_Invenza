// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "UI/Core/Modal/ModalButtonsPanelBase.h"
#include "InventoryContextActionsWidget.generated.h"

/** Inventory-only footer; existing modal footers retain their Blueprint behavior. */
UCLASS(Abstract, Blueprintable)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInventoryContextActionsWidget : public UModalButtonsPanelBase,
	public IModalInterface
{
	GENERATED_BODY()

public:
	// Functions
	virtual void ConfigureButtons_Implementation(const TArray<EObjectInteractionType>& Actions,
		const TArray<FModalAction>& Display) override;
	virtual TArray<UUIButton*> GetButtons_Implementation() const override;

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(EditDefaultsOnly, Category = "Modal")
	TSubclassOf<UUserWidget> DefaultButtonClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUIButton>> Buttons;
};
