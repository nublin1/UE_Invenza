// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CraftSystem/CraftingStructs.h"
#include "Interface/World/WorldCraft_WidProvider.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Craft/Lists/QueueCraftListEntryWidget.h"
#include "WorldCraftStateWidget.generated.h"

class UBorder;

/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UWorldCraftStateWidget : public UInvenzaBaseWidget,
																public IWorldCraft_WidProvider
{
	GENERATED_BODY()

public:
	UWorldCraftStateWidget();

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(Category = "Invenza|Crafting", VisibleInstanceOnly, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UQueueCraftListEntryWidget> QueueCraftListEntryWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Craft|Progress")
	FCraftProgressDisplaySettings ProgressSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Craft|Appearance")
	FLinearColor BackgroundColor = FLinearColor(0.028f, 0.019f, 0.011f, 0.96f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Craft|Appearance")
	FLinearColor FrameColor = FLinearColor(0.32f, 0.23f, 0.11f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Craft|Appearance")
	FMargin ContentPadding = FMargin(16.f, 12.f);

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> WorldSurface;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> WorldFrame;

	// Functions
	UFUNCTION()
	virtual UCraftingComponent* GetCraftComponentPtr() override { return CraftingComponentLink; }

	UFUNCTION()
	virtual void SetCraftComponentPtr(UCraftingComponent* NewCraftingComponent) override;

	UFUNCTION(BlueprintCallable, Category = "World Craft|Progress")
	void SetProgressDisplaySettings(const FCraftProgressDisplaySettings& Settings);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "World Craft")
	TObjectPtr<UCraftingComponent> CraftingComponentLink = nullptr;

	// Functions
	void BindCraftComponent();

	UFUNCTION()
	void HandleCurrentCraftDataChanged(const FQueuedRecipe& Recipe);

	UFUNCTION()
	void RefreshFromRecipe(const FQueuedRecipe& Recipe);
};
