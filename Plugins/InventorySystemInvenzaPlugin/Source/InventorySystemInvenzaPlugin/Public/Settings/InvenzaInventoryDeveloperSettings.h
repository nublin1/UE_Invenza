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
class UInvenzaUITheme;
class UStaticMesh;
class UStringTable;
class UCharacterInventoryExchangeWidget;
class UInventoryWorkspaceRowWidget;
class UControlsHelpRowWidget;
class UInventoryAdmissionWidget;
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
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Properties and variables
	UPROPERTY(Category = "Invenza|Inventory", Config, EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UInvenzaInventorySettingsAsset> InventorySettingsAsset;

	/** Shared appearance used in Widget Designer and when widgets are created in game. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Invenza|Interface|Theme")
	TSoftObjectPtr<UInvenzaUITheme> UITheme;

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

	/** Compatibility defaults used only when no UI theme is selected. */
	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	TSoftObjectPtr<UFont> DefaultUIFont;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	FLinearColor DefaultTextColor = FLinearColor::White;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	int32 HelpRowFontSize = 15;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	int32 HelpHeadingFontSize = 17;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	int32 ProgressFontSize = 13;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Interface|Legacy Appearance", AdvancedDisplay, meta = (ToolTip = "Compatibility fallback used only when no UI theme is selected."))
	int32 RestockFontSize = 18;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Storage")
	TSoftObjectPtr<UStaticMesh> DefaultStorageWallMesh;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	TSoftObjectPtr<UStaticMesh> DefaultAccessPointMesh;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	FInventoryTemplateSettings CharacterBagTemplate;

	/** Any item in this category receives the inventory configured in its item table row. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Invenza|Inventory")
	FGameplayTag PortableInventoryCategory;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Examples")
	FInventoryTemplateSettings CharacterEquipmentTemplate;

	/** Root of the item category tree; unrelated Gameplay Tags are not exposed. */
	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Admission")
	FGameplayTag AdmissionCategoryRoot;

	/** Catalogs used by the admission viewer and by server-side validation of item rules. */
	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Admission")
	TArray<TSoftObjectPtr<UDataTable>> AdmissionItemTables;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Admission")
	TMap<FGameplayTag, FText> AdmissionCategoryLabels;

	UPROPERTY(Config, EditAnywhere, Category = "Invenza|Admission")
	TSoftClassPtr<UInventoryAdmissionWidget> AdmissionWindowClass;

	// Functions
	UFUNCTION(BlueprintPure, Category = "Invenza|Localization")
	FText GetText(FName Key) const;
};
