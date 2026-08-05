// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InterfaceUtils.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInterfaceUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	// If the function is owned by the interface itself (i.e. the default
	// BlueprintNativeEvent stub), the object has not overridden it.
	// Otherwise, the function is implemented either in C++ or in a Blueprint class.
	UFUNCTION(BlueprintCallable, Category="Utils|Interface")
	static bool IsFunctionOverridden(UObject* Target, FName FunctionName, UClass* InterfaceClass);
};
