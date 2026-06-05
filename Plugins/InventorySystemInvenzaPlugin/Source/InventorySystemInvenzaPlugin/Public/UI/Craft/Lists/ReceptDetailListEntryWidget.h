// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/InvenzaBaseWidget.h"
#include "ReceptDetailListEntryWidget.generated.h"

class UVerticalBox;
struct FRecipeRequirementResult;
struct FRecipeItemRequirementCheck;
class URequirementOptionEntry;
class UCurrentMaxDisplay;
class ULabelBaseText;
class UImageBaseWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UReceptDetailListEntryWidget : public UInvenzaBaseWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

		
public:
	UReceptDetailListEntryWidget();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* DetailItemObject) override;

public:	
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Componets", meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> RequirementsContainer;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	TSubclassOf<URequirementOptionEntry> RequirementOptionClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Config")
	TSubclassOf<URequirementOptionEntry> RequirementOptionClassAlt;

	// Data
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "UI|Data")
	int SelectedOption = 0;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	
	UFUNCTION()
	URequirementOptionEntry* CreateRequirementOptionEntry(const FRecipeRequirementResult& RequirementResult, TSubclassOf<URequirementOptionEntry> OptionClass);
	
	UFUNCTION()
	TArray<URequirementOptionEntry*> CreateRequirementOptionEntries(const FRecipeItemRequirementCheck& ItemRequirementCheck);	
	
};
