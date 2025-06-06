// Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/Buttons/UIButton.h"
#include "ItemCategoryButton.generated.h"

enum class EItemCategory : uint8;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UItemCategoryButton : public UUIButton
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UItemCategoryButton(); 

	EItemCategory GetItemCategory() const {return Category;}
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EItemCategory Category;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================

	virtual void OnMainButtonClicked() override;
};
