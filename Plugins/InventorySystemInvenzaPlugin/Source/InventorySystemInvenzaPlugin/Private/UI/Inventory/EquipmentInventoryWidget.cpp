//  Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/EquipmentInventoryWidget.h"
#include "Engine/SkeletalMesh.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Misc/App.h"
#include "PreviewScene.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

/** Private rendering scene: lighting and preview primitives never affect the gameplay world. */
class FEquipmentPortraitScene
{
public:
	FEquipmentPortraitScene()
		: Scene(FPreviewScene::ConstructionValues()
					.SetEditor(false)
					.SetCreatePhysicsScene(false)
					.SetTransactional(false)
					.SetLightRotation(FRotator(-25, 160, 0))
					.SetLightBrightness(4.f))
	{
		auto* Fill = NewObject<UDirectionalLightComponent>(GetTransientPackage(), NAME_None, RF_Transient);
		Fill->SetIntensity(2.f);
		Fill->SetCastShadows(false);
		Scene.AddComponent(Fill, FTransform(FRotator(-15, -135, 0)));
	}

	UWorld* GetWorld() const { return Scene.GetWorld(); }

	FBox UpdateActor(AActor* Actor)
	{
		TArray<AActor*> Actors{Actor};
		Actor->GetAttachedActors(Actors, false, true);
		TSet<UPrimitiveComponent*> CurrentSources;
		for (auto* SourceActor : Actors)
		{
			TArray<UPrimitiveComponent*> Components;
			SourceActor->GetComponents(Components);
			for (auto* Source : Components)
			{
				if ((!Cast<USkeletalMeshComponent>(Source) && !Cast<UStaticMeshComponent>(Source)) ||
					!Source->IsVisible() || Source->bHiddenInGame)
				{
					continue;
				}
				CurrentSources.Add(Source);
				auto*& Preview = Meshes.FindOrAdd(Source);
				if (!Preview)
				{
					Preview = Cast<USkeletalMeshComponent>(Source)
								  ? static_cast<UPrimitiveComponent*>(
										NewObject<UPoseableMeshComponent>(Scene.GetWorld(), NAME_None, RF_Transient))
								  : NewObject<UStaticMeshComponent>(Scene.GetWorld(), NAME_None, RF_Transient);
					Preview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					Preview->SetCanEverAffectNavigation(false);
					Scene.AddComponent(Preview, FTransform::Identity);
				}
				Preview->SetWorldTransform(
					Source->GetComponentTransform().GetRelativeTransform(Actor->GetActorTransform()));
				if (auto* Pose = Cast<UPoseableMeshComponent>(Preview))
				{
					auto* SkeletalSource = CastChecked<USkeletalMeshComponent>(Source);
					if (Pose->GetSkinnedAsset() != SkeletalSource->GetSkeletalMeshAsset())
					{
						Pose->SetSkinnedAssetAndUpdate(SkeletalSource->GetSkeletalMeshAsset());
					}
					Pose->CopyPoseFromSkeletalComponent(SkeletalSource);
				}
				else
				{
					CastChecked<UStaticMeshComponent>(Preview)->SetStaticMesh(
						CastChecked<UStaticMeshComponent>(Source)->GetStaticMesh());
				}
				auto* SourceMesh = CastChecked<UMeshComponent>(Source);
				auto* PreviewMesh = CastChecked<UMeshComponent>(Preview);
				for (int32 Index = 0; Index < SourceMesh->GetNumMaterials(); ++Index)
				{
					if (PreviewMesh->GetMaterial(Index) != SourceMesh->GetMaterial(Index))
					{
						PreviewMesh->SetMaterial(Index, SourceMesh->GetMaterial(Index));
					}
				}
				Preview->UpdateBounds();
			}
		}
		FBox Bounds(ForceInit);
		for (auto It = Meshes.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid() || !CurrentSources.Contains(It.Key().Get()))
			{
				Scene.RemoveComponent(It.Value());
				It.RemoveCurrent();
			}
			else
			{
				Bounds += It.Value()->Bounds.GetBox();
			}
		}
		Scene.GetWorld()->SendAllEndOfFrameUpdates();
		return Bounds;
	}

protected:
	// Runtime
	FPreviewScene Scene;
	TMap<TWeakObjectPtr<UPrimitiveComponent>, UPrimitiveComponent*> Meshes;
};

UEquipmentInventoryWidget::UEquipmentInventoryWidget()
{
}

UEquipmentInventoryWidget::~UEquipmentInventoryWidget() = default;

void UEquipmentInventoryWidget::SetPreviewActor(AActor* Actor)
{
	if (PreviewActor == Actor)
	{
		return;
	}
	ReleaseCharacterPreview();
	PreviewActor = Actor;
	UpdateCharacterPreview();
}

void UEquipmentInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateCharacterPreview();
}

void UEquipmentInventoryWidget::NativeDestruct()
{
	ReleaseCharacterPreview();
	Super::NativeDestruct();
}

void UEquipmentInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Rendering the visible character is independent of inventory updates, which remain event driven.
	if (PreviewActor.IsValid())
	{
		UpdateCharacterPreview();
	}
}

static bool IsEquipmentPortraitResource(UObject* Resource)
{
	if (Cast<UTextureRenderTarget2D>(Resource))
	{
		return true;
	}
	if (auto* Material = Cast<UMaterialInterface>(Resource))
	{
		TArray<UTexture*> Textures;
		Material->GetUsedTextures(Textures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::SM5, true);
		return Textures.ContainsByPredicate(
			[](UTexture* Texture)
			{
				return Cast<UTextureRenderTarget2D>(Texture) != nullptr;
			});
	}
	return false;
}

void UEquipmentInventoryWidget::UpdateCharacterPreview()
{
	auto* Actor = PreviewActor.Get();
	auto* World = GetWorld();
	if (!IsValid(Actor) || Actor->IsActorBeingDestroyed() || !World || World->bIsTearingDown ||
		World->GetNetMode() == NM_DedicatedServer || !FApp::CanEverRender())
	{
		return;
	}
	if (!PreviewImage && WidgetTree)
	{
		WidgetTree->ForEachWidget(
			[&](UWidget* Widget)
			{
				if (auto* Image = Cast<UImage>(Widget);
					Image && IsEquipmentPortraitResource(Image->GetBrush().GetResourceObject()))
				{
					PreviewImage = Image;
				}
			});
		if (!PreviewImage)
		{
			return;
		}
		OriginalPreviewBrush = PreviewImage->GetBrush();
		bHasOriginalPreviewBrush = true;
	}
	if (!PreviewImage)
	{
		return;
	}
	if (!IsValid(PreviewCapture))
	{
		FActorSpawnParameters Parameters;
		Parameters.ObjectFlags = RF_Transient;
		Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PortraitScene = MakeShared<FEquipmentPortraitScene>();
		PreviewCapture = PortraitScene->GetWorld()->SpawnActor<ASceneCapture2D>(Parameters);
		if (!PreviewCapture)
		{
			return;
		}
		PreviewCapture->SetActorEnableCollision(false);
		PreviewCapture->SetActorTickEnabled(false);
		PreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
		PreviewRenderTarget->ClearColor = FLinearColor::Black;
		PreviewRenderTarget->InitAutoFormat(320, 512);
		auto* Capture = PreviewCapture->GetCaptureComponent2D();
		Capture->TextureTarget = PreviewRenderTarget;
		Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture->ProjectionType = ECameraProjectionMode::Orthographic;
		Capture->bCaptureEveryFrame = false;
		Capture->bCaptureOnMovement = false;
		Capture->ShowFlags.SetAtmosphere(false);
		Capture->ShowFlags.SetFog(false);
		Capture->ShowFlags.SetVolumetricFog(false);
		Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
		Capture->PostProcessSettings.bOverride_AutoExposureMethod = true;
		Capture->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
		Capture->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		Capture->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = false;
		Capture->PostProcessSettings.bOverride_AutoExposureBias = true;
		Capture->PostProcessSettings.AutoExposureBias = 0.f;
	}
	const FBox Bounds = PortraitScene->UpdateActor(Actor);
	if (!Bounds.IsValid)
	{
		return;
	}
	const FVector Center = Bounds.GetCenter();
	const FVector Extent = Bounds.GetExtent();
	const FVector CameraLocation = Center + FVector(FMath::Max(350.f, Extent.GetMax() * 4.f), 0, 0);
	PreviewCapture->SetActorLocationAndRotation(CameraLocation, (Center - CameraLocation).Rotation());
	auto* Capture = PreviewCapture->GetCaptureComponent2D();
	Capture->OrthoWidth = FMath::Max(FMath::Max(Extent.X, Extent.Y) * 2.f, Extent.Z * 2.f * 320.f / 512.f) * 1.15f;
	if (PreviewImage->GetBrush().GetResourceObject() != PreviewRenderTarget)
	{
		PreviewImage->SetBrushResourceObject(PreviewRenderTarget);
	}
	Capture->CaptureScene();
}

void UEquipmentInventoryWidget::ReleaseCharacterPreview()
{
	if (PreviewImage && bHasOriginalPreviewBrush)
	{
		PreviewImage->SetBrush(OriginalPreviewBrush);
	}
	if (IsValid(PreviewCapture))
	{
		PreviewCapture->Destroy();
	}
	PreviewCapture = nullptr;
	PortraitScene.Reset();
	PreviewRenderTarget = nullptr;
	PreviewImage = nullptr;
	bHasOriginalPreviewBrush = false;
}
