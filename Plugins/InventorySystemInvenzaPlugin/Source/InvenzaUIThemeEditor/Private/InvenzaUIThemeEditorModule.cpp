// Nublin Studio 2026 All Rights Reserved.
#include "Modules/ModuleManager.h"
#include "GameDelegates.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "EditorValidatorSubsystem.h"
#include "WidgetBlueprint.h"
#include "UI/InvenzaBaseWidget.h"
#include "Misc/DataValidation.h"

static void ValidateInvenzaThemesBeforeCook(TConstArrayView<const ITargetPlatform*> Platforms, TArray<FName>& Cook, TArray<FName>& NeverCook)
{
	if (!GEditor)
	{
		return;
	}
	auto* Validation = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>();
	auto& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> Assets;
	Registry.GetAssetsByClass(UWidgetBlueprint::StaticClass()->GetClassPathName(), Assets, true);
	for (const auto& Data : Assets)
	{
		FString NativeParent;
		Data.GetTagValue(TEXT("NativeParentClass"), NativeParent);
		if (!NativeParent.Contains(TEXT("/Script/InventorySystemInvenzaPlugin.")))
		{
			continue;
		}
		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {});
		if (Validation->IsAssetValidWithContext(Data, Context) == EDataValidationResult::Invalid)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Invenza UI theme bindings in %s. Run asset validation before cooking."), *Data.GetObjectPathString());
		}
	}
}

class FInvenzaUIThemeEditorModule : public IModuleInterface
{
public:
	// Functions
	virtual void StartupModule() override
	{
		CookHandle = FGameDelegates::Get().GetModifyCookDelegate().AddStatic(&ValidateInvenzaThemesBeforeCook);
	}
	virtual void ShutdownModule() override
	{
		FGameDelegates::Get().GetModifyCookDelegate().Remove(CookHandle);
	}
protected:
	// Runtime
	// Properties and variables
	FDelegateHandle CookHandle;
};
IMPLEMENT_MODULE(FInvenzaUIThemeEditorModule, InvenzaUIThemeEditor)
