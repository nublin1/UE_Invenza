//  Nublin Studio 2026 All Rights Reserved.

#include "World/InventoryMannequin.h"
#include "ActorComponents/InventoryCollectionComponent.h"
#include "ActorComponents/Interactable/CharacterInventoryAccessComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Inventory/Equipment/EquipmentComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/DataTable.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"

AInventoryMannequin::AInventoryMannequin()
{
	bReplicates = true;
	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->InitCapsuleSize(40, 96);
	Collision->SetCollisionProfileName(TEXT("BlockAll"));

	SetRootComponent(Collision);
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mannequin"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetRelativeLocation(FVector(0, 0, -96));
	Mesh->SetRelativeRotation(FRotator(0, -90, 0));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InventoryCollection = CreateDefaultSubobject<UInventoryCollectionComponent>(TEXT("InventoryCollection"));
	Equipment = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));

	InventoryAccess = CreateDefaultSubobject<UCharacterInventoryAccessComponent>(TEXT("InventoryAccess"));
	InventoryAccess->bCreateDefaultInventories = true;
}

void AInventoryMannequin::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyExampleDefaults();
}

void AInventoryMannequin::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyExampleDefaults();
}

void AInventoryMannequin::ApplyExampleDefaults()
{
	const UInvenzaInventoryDeveloperSettings* ExampleSettings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	Collision->SetCollisionResponseToChannel(ExampleSettings->InteractionTraceChannel, ECR_Block);
	if (!Mesh->GetSkeletalMeshAsset() && !ExampleSettings->ExampleMannequinMesh.IsNull())
	{
		Mesh->SetSkeletalMesh(ExampleSettings->ExampleMannequinMesh.LoadSynchronous());
	}
	if (Equipment->HasSlotDefinitions())
	{
		return;
	}
	UDataTable* EquipmentTable = ExampleSettings->ExampleEquipmentTable.LoadSynchronous();
	if (EquipmentTable)
	{
		TArray<FDataTableRowHandle> Definitions;
		for (const FName Name : EquipmentTable->GetRowNames())
		{
			FDataTableRowHandle Row;
			Row.DataTable = EquipmentTable;
			Row.RowName = Name;
			Definitions.Add(Row);
		}
		Equipment->SetSlotDefinitions(Definitions);
	}
}
