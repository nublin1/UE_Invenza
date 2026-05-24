// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Core/Buttons/UIButton.h"
#include "FilterTagButton.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UFilterTagButton : public UUIButton
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFilterTagButton(); 

	FGameplayTag GetFilterTag() const { return FilterTag; }
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FGameplayTag FilterTag;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void OnMainButtonClicked() override;
};
