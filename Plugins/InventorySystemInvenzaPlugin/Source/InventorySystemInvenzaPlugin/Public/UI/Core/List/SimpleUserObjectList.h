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
	// Unreal overrides
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	// Properties and variables
	// Widgets
	UPROPERTY(EditAnywhere, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UFiltersPanel> ItemFiltersPanel;

	UPROPERTY(BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UListView> ObjectList;

protected:
	// Runtime

	// Functions
	UFUNCTION()
	virtual void SearchTextChanged(const FText& NewText) {};
};
