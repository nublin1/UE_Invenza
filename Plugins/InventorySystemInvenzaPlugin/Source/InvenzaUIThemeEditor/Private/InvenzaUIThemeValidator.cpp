// Nublin Studio 2026 All Rights Reserved.
#include "InvenzaUIThemeValidator.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"

bool UInvenzaUIThemeValidator::CanValidateAsset_Implementation(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& Context) const
{
	const auto* Blueprint = Cast<UWidgetBlueprint>(Asset);
	return Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(UInvenzaBaseWidget::StaticClass());
}

EDataValidationResult UInvenzaUIThemeValidator::ValidateLoadedAsset_Implementation(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& Context)
{
	auto* Blueprint = Cast<UWidgetBlueprint>(Asset);
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return EDataValidationResult::NotValidated;
	}
	TArray<UInvenzaBaseWidget*> Owners;
	Owners.Add(CastChecked<UInvenzaBaseWidget>(Blueprint->GeneratedClass->GetDefaultObject()));
	TArray<UWidget*> Widgets;
	Blueprint->WidgetTree->GetAllWidgets(Widgets);
	for (auto* Widget : Widgets)
	{
		if (auto* Owner = Cast<UInvenzaBaseWidget>(Widget))
		{
			Owners.Add(Owner);
		}
	}
	bool bFailed = false;
	for (auto* Owner : Owners)
	{
		UWidgetTree* Tree = Owner == Owners[0] ? Blueprint->WidgetTree : nullptr;
		if (!Tree || !Tree->RootWidget)
		{
			for (UClass* Type = Owner->GetClass(); Type; Type = Type->GetSuperClass())
			{
				if (auto* Class = Cast<UWidgetBlueprintGeneratedClass>(Type))
				{
					auto* Candidate = Class->GetWidgetTreeArchetype();
					if (Candidate && Candidate->RootWidget)
					{
						Tree = Candidate;
						break;
					}
				}
			}
		}
		for (const auto& Result : UInvenzaUIThemeLibrary::ValidateBindings(Owner, Tree))
		{
			if (Result.Diagnostic.IsEmpty())
			{
				continue;
			}
			const FText Message = FText::FromString(FString::Printf(TEXT("%s -> %s -> %s.%s [role %s]: %s"),
				*Blueprint->GetPathName(), *Owner->GetName(), *Result.Binding.WidgetName.ToString(),
				*Result.Binding.PropertyPath, *Result.Binding.Role.ToString(), *Result.Diagnostic));
			if (Result.Status == EInvenzaUIThemeBindingStatus::MissingTheme)
			{
				AssetWarning(Asset, Message);
			}
			else
			{
				AssetFails(Asset, Message);
				bFailed = true;
			}
		}
	}
	if (!bFailed)
	{
		AssetPasses(Asset);
	}
	return bFailed ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
