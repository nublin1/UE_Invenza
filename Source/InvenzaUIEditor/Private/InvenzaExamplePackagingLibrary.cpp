// Nublin Studio 2026 All Rights Reserved.

#include "InvenzaExamplePackagingLibrary.h"
#include "Animation/AnimationAsset.h"
#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "ActorComponents/InteractionComponent.h"
#include "UObject/UnrealType.h"

bool UInvenzaExamplePackagingLibrary::RefreshMigratedPreview(UObject* Asset, USkeletalMesh* PreviewMesh)
{
	if (!IsValid(Asset) || !IsValid(PreviewMesh) ||
		!Asset->GetPathName().StartsWith(TEXT("/InventorySystemInvenzaPlugin/Characters/")) ||
		!PreviewMesh->GetPathName().StartsWith(TEXT("/InventorySystemInvenzaPlugin/Characters/")))
	{
		return false;
	}
	Asset->Modify();
	if (auto* Skeleton = Cast<USkeleton>(Asset))
	{
		Skeleton->SetPreviewMesh(PreviewMesh);
		Skeleton->SetAdditionalPreviewSkeletalMeshes(nullptr);
	}
	if (auto* Mesh = Cast<USkeletalMesh>(Asset))
	{
		if (Mesh->GetDefaultAnimatingRig().ToSoftObjectPath().ToString().StartsWith(TEXT("/Game/")))
		{
			Mesh->SetDefaultAnimatingRig(TSoftObjectPtr<UObject>());
		}
	}
	if (auto* Animation = Cast<UAnimationAsset>(Asset))
	{
		Animation->SetPreviewMesh(PreviewMesh);
	}
	if (auto* Pose = Cast<UPoseAsset>(Asset))
	{
		Pose->SetRetargetSourceAsset(PreviewMesh);
		if (Pose->SourceAnimation)
		{
			Pose->UpdatePoseFromAnimation(Pose->SourceAnimation);
		}
	}
	Asset->MarkPackageDirty();
	return true;
}

bool UInvenzaExamplePackagingLibrary::ConfigureExampleCharacter(UBlueprint* Blueprint, USkeletalMesh* Mesh,
	TSubclassOf<UAnimInstance> AnimationClass, TEnumAsByte<ECollisionChannel> TraceChannel)
{
	if (!IsValid(Blueprint) || !Blueprint->GeneratedClass || !IsValid(Mesh) || !AnimationClass ||
		!Blueprint->GetPathName().StartsWith(TEXT("/InventorySystemInvenzaPlugin/Blueprints/Examples/BP_Character.")))
	{
		return false;
	}
	auto* Character = Cast<ACharacter>(Blueprint->GeneratedClass->GetDefaultObject());
	if (!Character || !Character->GetMesh() || !Blueprint->SimpleConstructionScript)
	{
		return false;
	}
	Blueprint->Modify();
	Character->GetMesh()->Modify();
	Character->GetMesh()->SetSkeletalMeshAsset(Mesh);
	Character->GetMesh()->SetAnimInstanceClass(AnimationClass);
	bool bConfiguredInteraction = false;
	for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
	{
		if (Node && Cast<UInteractionComponent>(Node->ComponentTemplate))
		{
			auto* Property = FindFProperty<FByteProperty>(Node->ComponentTemplate->GetClass(), TEXT("TraceChannel"));
			if (Property)
			{
				Node->ComponentTemplate->Modify();
				Property->SetPropertyValue_InContainer(Node->ComponentTemplate, TraceChannel.GetValue());
				bConfiguredInteraction = true;
			}
		}
	}
	Blueprint->MarkPackageDirty();
	return bConfiguredInteraction;
}
