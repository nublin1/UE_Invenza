// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "QueueCraftList.generated.h"

struct FQueuedRecipe;
class UProductionQueueListEntryObject;
class UListView;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UQueueCraftList : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UQueueCraftList();

protected:
	// Unreal overrides
	virtual void NativeConstruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQueueOrderChangeRequested, FName, RecipeID, int32, QueueIndex,
												   bool, bMoveUp);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueItemDeleteRequested, int32, QueueIndex);
#pragma endregion Delegates

	// Properties and variables
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Crafting|UI")
	FOnQueueOrderChangeRequested OnQueueOrderChangeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|UI")
	FOnQueueItemDeleteRequested OnQueueItemDeleteRequested;

	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> QueueList;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
	void SetNewProductionQueueList(const TArray<FQueuedRecipe>& InRecipeQueue);

	UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
	void UpdateDataInRecipe(const FQueuedRecipe& UpdatedRecipe);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|Crafting", VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UProductionQueueListEntryObject>> ProductionQueueList;

	// Functions
	UFUNCTION()
	void UpdateProductionQueueList();

	UFUNCTION()
	void HandleMoveItemRequest(UObject* Item, bool bMoveUp);

	UFUNCTION()
	void HandleDeleteWithIndexRequest(UObject* Item);

	void OnEntryGenerated(UUserWidget& UserWidget);
};
