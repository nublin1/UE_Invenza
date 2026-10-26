// Nublin Studio 2026 All Rights Reserved.

#include "InvenzaMediaCaptureLibrary.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Widgets/SViewport.h"
#include "Widgets/SWindow.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

bool UInvenzaMediaCaptureLibrary::StartPlayPreview(int32 Width, int32 Height)
{
	if (!GEditor || GEditor->PlayWorld || Width <= 0 || Height <= 0)
	{
		return false;
	}
	auto* PlaySettings = DuplicateObject<ULevelEditorPlaySettings>(
		GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
	PlaySettings->NewWindowWidth = Width;
	PlaySettings->NewWindowHeight = Height;
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetRunUnderOneProcess(true);
	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	Params.bAllowOnlineSubsystem = false;
	TSharedRef<SWindow> CaptureWindow = SNew(SWindow)
		.ClientSize(FVector2D(Width, Height))
		.AutoCenter(EAutoCenter::None)
		.ScreenPosition(FVector2D::ZeroVector)
		.SaneWindowPlacement(false)
		.SizingRule(ESizingRule::FixedSize)
		.AdjustInitialSizeAndPositionForDPIScale(false);
	FSlateApplication::Get().AddWindow(CaptureWindow, true);
	Params.CustomPIEWindow = CaptureWindow;
	GEditor->RequestPlaySession(Params);
	return true;
}

bool UInvenzaMediaCaptureLibrary::CaptureViewport(UWorld* World, const FString& Filename)
{
	if (!IsValid(World) || !World->IsGameWorld() || !World->GetGameViewport() ||
		!FSlateApplication::IsInitialized())
	{
		return false;
	}
	TSharedPtr<SViewport> Viewport = World->GetGameViewport()->GetGameViewportWidget();
	if (!Viewport.IsValid())
	{
		return false;
	}
	TArray<FColor> Pixels;
	FIntVector Size;
	if (!FSlateApplication::Get().TakeScreenshot(Viewport.ToSharedRef(), Pixels, Size))
	{
		return false;
	}
	for (FColor& Pixel : Pixels)
	{
		Pixel.A = 255;
	}
	TArray64<uint8> ImageData;
	FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, ImageData);
	return FFileHelper::SaveArrayToFile(ImageData, *Filename);
}

ACameraActor* UInvenzaMediaCaptureLibrary::FrameCamera(APlayerController* Controller, ACameraActor* Camera,
	FVector Location, FVector Target, float FieldOfView)
{
	if (!IsValid(Controller) || !Controller->GetWorld() || !Controller->GetWorld()->IsGameWorld())
	{
		return nullptr;
	}
	if (!IsValid(Camera))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
		Camera = Controller->GetWorld()->SpawnActor<ACameraActor>(SpawnParameters);
	}
	if (IsValid(Camera) && Camera->GetWorld() == Controller->GetWorld())
	{
		Camera->SetActorLocationAndRotation(Location, (Target - Location).Rotation());
		Camera->GetCameraComponent()->SetFieldOfView(FieldOfView);
		Controller->SetViewTarget(Camera);
		return Camera;
	}
	return nullptr;
}
