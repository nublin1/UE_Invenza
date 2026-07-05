// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputUtility.generated.h"

enum class ETriggerEvent : uint8;
class UInputAction;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInputUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Helpers|Input")
	static FText GetKeyForAction(UWorld* World, UInputAction* Action);
	
	template<typename UserClass, typename FuncType, typename... VarTypes>
	static uint32 BindAction(
		UEnhancedInputComponent* Input,
		UInputAction* Action,
		ETriggerEvent Trigger,
		UserClass* Object,
		FuncType Func,
		VarTypes... Vars);
	
	template<typename UserClass, typename FuncType, typename... VarTypes>
	static uint32 RebindAction(
		UEnhancedInputComponent* Input,
		uint32& Handle,
		UInputAction* Action,
		ETriggerEvent Trigger,
		UserClass* Object,
		FuncType Func,
		VarTypes... Vars);

	
	static void RemoveBinding(
		UEnhancedInputComponent* Input,
		uint32& Handle);
};


template <typename UserClass, typename FuncType, typename ... VarTypes>
uint32 UInputUtility::BindAction(UEnhancedInputComponent* Input, UInputAction* Action, ETriggerEvent Trigger,
	UserClass* Object, FuncType Func, VarTypes... Vars)
{
	if (!Input || !Action || !Object)
		return INDEX_NONE;

	auto& Binding = Input->BindAction(
		Action,
		Trigger,
		Object,
		Func,
		Vars...);

	return Binding.GetHandle();
}

template <typename UserClass, typename FuncType, typename ... VarTypes>
uint32 UInputUtility::RebindAction(UEnhancedInputComponent* Input, uint32& Handle, UInputAction* Action,
	ETriggerEvent Trigger, UserClass* Object, FuncType Func, VarTypes... Vars)
{
	RemoveBinding(Input, Handle);

	Handle = BindAction(
		Input,
		Action,
		Trigger,
		Object,
		Func,
		Vars...);

	return Handle;
}
