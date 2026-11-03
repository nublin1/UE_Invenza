// Nublin Studio 2026 All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "ActorComponents/InventoryCollectionComponent.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/Inventory/ListInventory/ListInventory.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SaveLoad/InvenzaSaveGame.h"
#include "SaveLoad/InvenzaSaveSettings.h"
#include "Subsystems/InvenzaSaveSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInvenzaManualSavePolicyTest, "Invenza.Inventory.ManualSavePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInvenzaManualSavePolicyTest::RunTest(const FString& Parameters)
{
	const auto* Settings = GetDefault<UInvenzaSaveSettings>();
	TestFalse(TEXT("Periodic saves are disabled by default"), Settings->bAutoSave);
	TestFalse(TEXT("Exit saves are disabled by default"), Settings->bSaveOnExit);
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone();
	auto* World = Instance->GetWorld();
	auto* Save = Instance->GetSubsystem<UInvenzaSaveSubsystem>();
	const FString Profile = TEXT("ManualPolicyTest_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Save->SetProfile(Profile);
	auto* Actor = World->SpawnActor<AActor>();
	auto* Collection = NewObject<UInventoryCollectionComponent>(Actor);
	Actor->AddInstanceComponent(Collection);
	Collection->RegisterComponent();
	Collection->InitItemCollection();
	FInventoryStartupData Startup;
	Startup.Settings.InventoryClass = UListInventory::StaticClass();
	Startup.Settings.CustomInventoryID = TEXT("PolicyStorage");
	auto* Inventory = UInventoryBase::CreateInventoryAdvanced(Actor, Startup, Actor, Collection);
	Collection->AddPawnInventory_Internal(Inventory);
	Save->RegisterCollection(Collection);
	TestTrue(TEXT("Manual save is available"), Save->SaveSession(TEXT("Manual1"), false));
	FString Error;
	auto* Legacy = Save->CaptureSnapshot(Error);
	if (TestNotNull(TEXT("Capture isolated legacy snapshot"), Legacy))
	{
		Legacy->Revision += 100;
		const FString Prefix = FString::Printf(TEXT("Invenza_%s_%08X_"), *Profile,
			FCrc::StrCrc32(*UWorld::RemovePIEPrefix(World->GetOutermost()->GetName())));
		const FString LegacySlot = Prefix + TEXT("Shutdown_0");
		TestTrue(TEXT("Create isolated legacy exit save"), UGameplayStatics::SaveGameToSlot(Legacy, LegacySlot, 0));
		TestTrue(TEXT("Continue loads the manual save"), Save->LoadSession(TEXT("Continue")));
		if (const auto* Loaded = Save->GetLoadedSnapshot())
		{
			TestTrue(TEXT("A newer legacy shutdown save cannot override manual Continue"), Loaded->Revision < Legacy->Revision);
		}
		TestTrue(TEXT("Exit is allowed without starting a save"), Save->SaveBeforeExit());
		TestFalse(TEXT("Exit does not create a save file"), UGameplayStatics::DoesSaveGameExist(Prefix + TEXT("Exit_0"), 0));
		TestEqual(TEXT("Manual mode exposes only four explicit save slots"), Save->GetSaveSlots().Num(), 4);
		Save->bBusy = true;
		TestFalse(TEXT("An in-flight manual save blocks exit"), Save->SaveBeforeExit());
		Save->bBusy = false;
		UGameplayStatics::DeleteGameInSlot(LegacySlot, 0);
		UGameplayStatics::DeleteGameInSlot(Prefix + TEXT("Manual1_0"), 0);
		UGameplayStatics::DeleteGameInSlot(Prefix + TEXT("Manual1_1"), 0);
	}
	Save->UnregisterCollection(Collection, EEndPlayReason::Quit);
	Instance->Shutdown();
	return true;
}
#endif
