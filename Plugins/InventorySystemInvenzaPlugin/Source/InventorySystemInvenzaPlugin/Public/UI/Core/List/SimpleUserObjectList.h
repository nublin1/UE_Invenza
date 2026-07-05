// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "SimpleUserObjectList.generated.h"

class UListView;
class UFiltersPanel;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API USimpleUserObjectList : public UInvenzaBaseWidget
{
	GENERATED_BODY()
	
public:
	USimpleUserObjectList() {}

protected:
	virtual void NativeConstruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(EditAnywhere, Category = "UI|Components", meta=(BindWidgetOptional))
	TObjectPtr<UFiltersPanel> ItemFiltersPanel;
	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> ObjectList;

	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	

	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) {};
};
