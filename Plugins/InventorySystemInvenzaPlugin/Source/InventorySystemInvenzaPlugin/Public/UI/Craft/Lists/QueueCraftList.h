// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "QueueCraftList.generated.h"

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
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> QueueList;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable)
	void SetNewProductionQueueList(TArray<UProductionQueueListEntryObject*> InArray);

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UProductionQueueListEntryObject*> ProductionQueueList;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION()
	void UpdateProductionQueueList();
	UFUNCTION()
	void MoveItem(UObject* Item, bool bMoveUp);
	
	void OnEntryGenerated(UUserWidget& UserWidget);
};
