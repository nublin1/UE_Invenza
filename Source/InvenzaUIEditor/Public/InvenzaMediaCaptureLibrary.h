// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvenzaMediaCaptureLibrary.generated.h"

class ACameraActor;
class APlayerController;

/** Transient editor tools for capturing the actual example viewport and its UI. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaMediaCaptureLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Media Capture")
	static bool StartPlayPreview(int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Media Capture")
	static bool CaptureViewport(UWorld* World, const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Media Capture")
	static ACameraActor* FrameCamera(APlayerController* Controller, ACameraActor* Camera,
		FVector Location, FVector Target, float FieldOfView);
};
