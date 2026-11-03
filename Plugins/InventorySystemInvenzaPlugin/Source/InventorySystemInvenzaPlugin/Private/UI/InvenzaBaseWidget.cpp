//  Nublin Studio 2025 All Rights Reserved.

#include "UI/InvenzaBaseWidget.h"

#include "Components/PanelWidget.h"
#include "Engine/Engine.h"
#include "Subsystems/InvenzaUIThemeSubsystem.h"
#include "UI/Layout/UILayer.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"

UUILayer* UInvenzaBaseWidget::GetOwningLayer() const
{
	return OwningLayer.Get();
}

void UInvenzaBaseWidget::OnRemovedFromLayer_Implementation()
{
}

void UInvenzaBaseWidget::SetLayerScreenState(UUILayer* Layer, bool bActive)
{
	OwningLayer = Layer;
	if (bLayerScreenActive == bActive)
	{
		return;
	}
	bLayerScreenActive = bActive;
	if (bActive)
	{
		OnLayerScreenActivated();
	}
	else
	{
		OnLayerScreenDeactivated();
	}
}

void UInvenzaBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyUITheme();
}

void UInvenzaBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CalculateParentWidget();
	ApplyUITheme();
}

void UInvenzaBaseWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyUITheme();
}

void UInvenzaBaseWidget::CaptureLocalThemeValues()
{
	for (const auto& Binding : ThemeBindings)
	{
		if (Binding.WidgetName.IsNone())
		{
			continue;
		}
		auto* Target = UInvenzaUIThemeLibrary::FindBindingTarget(this, Binding);
		const bool bCaptured = LocalThemeValues.ContainsByPredicate([&](const FInvenzaUIThemeLocalValue& Entry)
		{
			return Entry.Target == Target && Entry.PropertyPath == Binding.PropertyPath;
		});
		FInvenzaUIThemeLocalValue Entry;
		if (!bCaptured && UInvenzaUIThemeLibrary::ReadStyleValue(Target, Binding.PropertyPath, Entry.Authored))
		{
			Entry.Target = Target;
			Entry.WidgetName = Binding.WidgetName;
			Entry.PropertyPath = Binding.PropertyPath;
			Entry.Rendered = Entry.Authored;
			LocalThemeValues.Add(MoveTemp(Entry));
		}
	}
}

void UInvenzaBaseWidget::ApplyUITheme()
{
	if (bApplyingUITheme || IsTemplate())
	{
		return;
	}
	TGuardValue<bool> Guard(bApplyingUITheme, true);
	CaptureLocalThemeValues();
	TSet<UWidget*> Changed;
	// Restore every captured raw component first, including removed or broken bindings.
	for (auto& Entry : LocalThemeValues)
	{
		if (auto* Target = Cast<UWidget>(Entry.Target.Get()))
		{
			UInvenzaUIThemeLibrary::WriteStyleValue(Target, Entry.PropertyPath, Entry.Authored);
			Changed.Add(Target);
		}
	}
	ThemeBindingResults = UInvenzaUIThemeLibrary::ValidateBindings(this);
	for (auto& Result : ThemeBindingResults)
	{
		auto* Target = UInvenzaUIThemeLibrary::FindBindingTarget(this, Result.Binding);
		if (Result.Status == EInvenzaUIThemeBindingStatus::Theme || Result.Status == EInvenzaUIThemeBindingStatus::Local ||
			Result.Status == EInvenzaUIThemeBindingStatus::MissingTheme || Result.Status == EInvenzaUIThemeBindingStatus::MissingRole ||
			Result.Status == EInvenzaUIThemeBindingStatus::WrongRoleType)
		{
			FInvenzaUIThemeValue Local;
			if (Target && UInvenzaUIThemeLibrary::ReadStyleValue(Target, Result.Binding.PropertyPath, Local))
			{
				Result = UInvenzaUIThemeLibrary::ResolveBinding(UInvenzaUIThemeLibrary::GetSelectedTheme(), Result.Binding, Local,
					UsesUITheme() && !(ThemeSource == EInvenzaUIThemeSource::ThemeWithOverrides && Result.Binding.bOverrideLocally));
				if (Target != this)
				{
					UInvenzaUIThemeLibrary::WriteStyleValue(Target, Result.Binding.PropertyPath, Result.Value);
					Changed.Add(Target);
				}
			}
		}
		if (!Result.Diagnostic.IsEmpty() && UsesUITheme())
		{
			if (auto* Themes = GEngine ? GEngine->GetEngineSubsystem<UInvenzaUIThemeSubsystem>() : nullptr)
			{
				Themes->ReportBindingIssue(this, Result);
			}
		}
	}
	for (auto* Target : Changed)
	{
		UInvenzaUIThemeLibrary::SynchronizeAppearance(Target);
	}
	RefreshThemeAppearance();
	for (auto& Entry : LocalThemeValues)
	{
		UInvenzaUIThemeLibrary::ReadStyleValue(Entry.Target.Get(), Entry.PropertyPath, Entry.Rendered);
	}
	OnUIThemeApplied();
}

#if WITH_EDITOR
void UInvenzaBaseWidget::RecordDesignerStyleEdit(UObject* Changed, const FPropertyChangedEvent& Event)
{
	if (bApplyingUITheme || !IsDesignTime() || IsTemplate())
	{
		return;
	}
	for (auto& Entry : LocalThemeValues)
	{
		auto* Target = Entry.Target.Get();
		if (Target && (Changed == Target || Changed == Target->GetArchetype()))
		{
			FInvenzaUIThemeValue Edited;
			if (UInvenzaUIThemeLibrary::ReadStyleValue(Changed, Entry.PropertyPath, Edited))
			{
				if (Changed == Target)
				{
					UInvenzaUIThemeLibrary::MergeEditedValue(Entry.Authored, Entry.Rendered, Edited);
				}
				else
				{
					Entry.Authored = Edited;
				}
			}
		}
	}
}
#endif

FInvenzaUIThemeValue UInvenzaBaseWidget::ResolveThemeValue(const FString& Path, FInvenzaUIThemeValue Local, FName WidgetName) const
{
	if (!WidgetName.IsNone())
	{
		for (const auto& Entry : LocalThemeValues)
		{
			if (Entry.WidgetName == WidgetName && Entry.PropertyPath == Path && Entry.Target.IsValid())
			{
				Local = Entry.Authored;
				break;
			}
		}
	}
	for (const auto& Result : ThemeBindingResults)
	{
		if (Result.Binding.WidgetName == WidgetName && Result.Binding.PropertyPath == Path &&
			(Result.Status == EInvenzaUIThemeBindingStatus::Duplicate || Result.Status == EInvenzaUIThemeBindingStatus::Overlap))
		{
			return Local;
		}
	}
	for (const auto& Binding : ThemeBindings)
	{
		if (Binding.WidgetName == WidgetName && Binding.PropertyPath == Path)
		{
			const bool bUse = UsesUITheme() && !(ThemeSource == EInvenzaUIThemeSource::ThemeWithOverrides && Binding.bOverrideLocally);
			return UInvenzaUIThemeLibrary::ResolveBinding(UInvenzaUIThemeLibrary::GetSelectedTheme(), Binding, Local, bUse).Value;
		}
	}
	return Local;
}

FSlateFontInfo UInvenzaBaseWidget::ResolveThemeFont(const FString& Path, const FSlateFontInfo& Local) const
{
	FInvenzaUIThemeValue Value;
	Value.Type = EInvenzaUIThemeValueType::Font;
	Value.Font = Local;
	return ResolveThemeValue(Path, Value).Font;
}

FSlateColor UInvenzaBaseWidget::ResolveThemeSlateColor(const FString& Path, const FSlateColor& Local) const
{
	FInvenzaUIThemeValue Value;
	Value.Type = EInvenzaUIThemeValueType::SlateColor;
	Value.SlateColor = Local;
	return ResolveThemeValue(Path, Value).SlateColor;
}

FSlateBrush UInvenzaBaseWidget::ResolveThemeBrush(const FString& Path, const FSlateBrush& Local) const
{
	FInvenzaUIThemeValue Value;
	Value.Type = EInvenzaUIThemeValueType::Brush;
	Value.Brush = Local;
	return ResolveThemeValue(Path, Value).Brush;
}

FLinearColor UInvenzaBaseWidget::ResolveThemeWidgetColor(FName WidgetName, const FString& Path, FLinearColor Local) const
{
	FInvenzaUIThemeValue Value;
	Value.Type = EInvenzaUIThemeValueType::Color;
	Value.Color = Local;
	return ResolveThemeValue(Path, Value, WidgetName).Color;
}

void UInvenzaBaseWidget::RefreshThemeAppearance()
{
}

void UInvenzaBaseWidget::SetThemePropertyOverride(const FString& PropertyPath, bool bOverride)
{
	for (auto& Binding : ThemeBindings)
	{
		if (Binding.WidgetName.IsNone() && Binding.PropertyPath == PropertyPath)
		{
			Binding.bOverrideLocally = bOverride;
		}
	}
	if (bOverride && ThemeSource == EInvenzaUIThemeSource::Theme)
	{
		ThemeSource = EInvenzaUIThemeSource::ThemeWithOverrides;
	}
}

FLinearColor UInvenzaBaseWidget::ResolveThemeColorForProperty(const FString& PropertyPath, FLinearColor Fallback) const
{
	return ResolveThemeWidgetColor(NAME_None, PropertyPath, Fallback);
}

void UInvenzaBaseWidget::CalculateParentWidget()
{
	if (this->GetParent())
	{
		if (auto PW = this->GetParent()->GetOuter()->GetOuter())
		{
			ParentWidget = Cast<UInvenzaBaseWidget>(PW);
		}
	}
}
