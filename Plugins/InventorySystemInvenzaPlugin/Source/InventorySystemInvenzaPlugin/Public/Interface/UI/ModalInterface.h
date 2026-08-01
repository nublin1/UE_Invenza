// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ModalInterface.generated.h"

struct FModalAction;
enum class EObjectInteractionType : uint8;
class UUIButton;

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UModalInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEMINVENZAPLUGIN_API IModalInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Modal")
	void ConfigureHeader(const FText& HeaderText);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Modal")
	void ConfigureButtons(const TArray<EObjectInteractionType>& Actions, const TArray<FModalAction>& Display);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Modal")
	void ConfigureModalBtn(const EObjectInteractionType& Action, const FModalAction& Display);
	
};
