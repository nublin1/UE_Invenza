// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Interaction/InteractionUIProvider.h"
#include "UI/Layout/UILayer.h"
#include "GameLayer.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UGameLayer : public UUILayer, public IInteractionUIProvider
{
	GENERATED_BODY()

public:
	UGameLayer();

protected:
	// Unreal overrides
	virtual void NativeOnInitialized() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(Category = "Invenza|UI|Game", VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UPanelWidget> InteractionPanel;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	virtual UInteractionWidget* GetPawnInteractionWidget() const override;
};
