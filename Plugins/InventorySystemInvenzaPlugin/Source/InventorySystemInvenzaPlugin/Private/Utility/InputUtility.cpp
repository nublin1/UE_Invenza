// Nublin Studio 2026 All Rights Reserved.


#include "Utility/InputUtility.h"

#include "EnhancedInputSubsystems.h"

FText UInputUtility::GetKeyForAction(UWorld* World, UInputAction* Action)
{
	FText Result = FText::GetEmpty();
	
	if (!Action)
		return Result;
	
	if (!World)
		return Result;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
		return Result;

	auto* EnhancedInputLocalPlayerSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (!EnhancedInputLocalPlayerSubsystem)
		return Result;

	auto QueryKeysMapped=EnhancedInputLocalPlayerSubsystem->QueryKeysMappedToAction(Action);
	if (QueryKeysMapped.IsEmpty())
		return Result;
	
	return QueryKeysMapped[0].GetDisplayName();
}

void UInputUtility::RemoveBinding(
	UEnhancedInputComponent* Input,
	uint32& Handle)
{
	if (!Input)
		return;

	if (Handle == INDEX_NONE)
		return;

	Input->RemoveBindingByHandle(Handle);

	Handle = INDEX_NONE;
}