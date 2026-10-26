//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "Data/Inventory/InventoryTemplateTypes.h"
#include "UI/UIStructs.h"
#include "InvenzaInventoryDeveloperSettings.generated.h"

class UInvenzaInventorySettingsAsset;
class USkeletalMesh;
class UDataTable;
class UFont;
class UStaticMesh;
class UStringTable;
class UCharacterInventoryExchangeWidget;
class UInventoryWorkspaceRowWidget;
class UControlsHelpRowWidget;
/**
 *
 */
UCLASS(Config = InventorySystemInvenzaPlugin, DefaultConfig, meta = (DisplayName = "Invenza Inventory"))
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventoryDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual FName GetCategoryName() const override;

public:
	// Properties and variables
	UPROPERTY(Category = "Invenza|Inventory", Config, EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UInvenzaInventorySettingsAsset> InventorySettingsAsset;

	/** Built-in Visibility works without reserving a trace-channel slot in the host project. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Invenza|Interaction")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	/** Optional mesh for the example actor. The plugin does not require a project mannequin. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Invenza|Examples")
	TSoftObjectPtr<USkeletalMesh> ExampleMannequinMesh;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Invenza|Examples")
	TSoftObjectPtr<UDataTable> ExampleEquipmentTable;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Localization")
	TSoftObjectPtr<UStringTable> RuntimeTextTable;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	TSoftClassPtr<UCharacterInventoryExchangeWidget> CharacterInventoryWindowClass;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	TSoftClassPtr<UInventoryWorkspaceRowWidget> StorageInventoryRowClass;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	TSoftClassPtr<UControlsHelpRowWidget> ControlsHelpRowClass;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	TSoftObjectPtr<UFont> DefaultUIFont;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	int32 HelpRowFontSize = 15;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	int32 HelpHeadingFontSize = 17;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	int32 ProgressFontSize = 13;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface")
	int32 RestockFontSize = 18;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Storage")
	TSoftObjectPtr<UStaticMesh> DefaultStorageWallMesh;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	TSoftObjectPtr<UStaticMesh> DefaultAccessPointMesh;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	FInventoryTemplateSettings CharacterBagTemplate;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	FInventoryTemplateSettings CharacterEquipmentTemplate;

	// Functions
	UFUNCTION(BlueprintPure, Category = "Invenza|Localization")
	FText GetText(FName Key) const;
};
