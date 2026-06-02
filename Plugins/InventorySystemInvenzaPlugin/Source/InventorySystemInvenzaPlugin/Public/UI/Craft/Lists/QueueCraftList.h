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

#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQueueOrderChangeRequested, FName, RecipeID, bool, bMoveUp);
#pragma endregion
	
public:
	UQueueCraftList();

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Crafting|UI")
	FOnQueueOrderChangeRequested OnQueueOrderChangeRequested;
	
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> QueueList;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
	void SetNewProductionQueueList(const TArray<FQueuedRecipe>& InRecipeQueue);

	UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
	void UpdateDataInRecipe(FQueuedRecipe& UpdatedRecipe);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UProductionQueueListEntryObject>> ProductionQueueList;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void UpdateProductionQueueList();
	UFUNCTION()
	void HandleMoveItemRequest(UObject* Item, bool bMoveUp);
	
	void OnEntryGenerated(UUserWidget& UserWidget);
};
