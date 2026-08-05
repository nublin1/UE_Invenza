// Nublin Studio 2026 All Rights Reserved.


#include "Utility/InterfaceUtils.h"

bool UInterfaceUtils::IsFunctionOverridden(UObject* Target, FName FunctionName, UClass* InterfaceClass)
{
	if (!IsValid(Target) || !IsValid(InterfaceClass))
	{
		return false;
	}

	const UFunction* TargetFunction = Target->GetClass()->FindFunctionByName(FunctionName);
	if (!TargetFunction)
	{
		return false;
	}

	// Blueprint implementation.
	if (TargetFunction->Script.Num() > 0)
	{
		return true;
	}

	// Native override.
	const UFunction* InterfaceFunction = InterfaceClass->FindFunctionByName(FunctionName);

	return InterfaceFunction && TargetFunction != InterfaceFunction;
}
