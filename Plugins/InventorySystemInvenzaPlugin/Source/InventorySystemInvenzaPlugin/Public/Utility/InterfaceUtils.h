// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Interaction/ObjectDataProvider.h"
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
	
	template <typename TInterface>
	UFUNCTION(BlueprintCallable, Category="Utils|Interface")
	static bool ValidateImplementsInterface(const UObject* Item, const FString& ContextName);
	
	FORCEINLINE static bool ImplementsObjectDataProvider(const UObject* Item)
	{
		return Item && Item->GetClass()->ImplementsInterface(UObjectDataProvider::StaticClass());
	}
};

template <typename TInterface>
bool UInterfaceUtils::ValidateImplementsInterface(const UObject* Item, const FString& ContextName)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Item is null"), *ContextName);
		return false;
	}

	if (!Item->GetClass()->ImplementsInterface(TInterface::UClassType::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Item '%s' does NOT implement %s"),
			*ContextName, *Item->GetName(), *TInterface::UClassType::StaticClass()->GetName());
		return false;
	}

	return true;
}

