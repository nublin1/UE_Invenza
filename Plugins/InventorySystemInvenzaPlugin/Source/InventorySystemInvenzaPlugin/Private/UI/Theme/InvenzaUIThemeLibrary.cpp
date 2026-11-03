// Nublin Studio 2026 All Rights Reserved.

#include "UI/Theme/InvenzaUIThemeLibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Subsystems/InvenzaUIThemeSubsystem.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Theme/InvenzaUITheme.h"
#include "UObject/UnrealType.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Widgets/Input/SEditableTextBox.h"

UInvenzaUITheme* UInvenzaUIThemeLibrary::GetSelectedTheme()
{
	auto* Themes = GEngine ? GEngine->GetEngineSubsystem<UInvenzaUIThemeSubsystem>() : nullptr;
	return Themes ? Themes->GetTheme() : nullptr;
}

FLinearColor UInvenzaUIThemeLibrary::ThemeColor(FName Role, FLinearColor Fallback)
{
	if (const auto* Theme = GetSelectedTheme())
	{
		if (const auto* Color = Theme->Palette.Find(Role))
		{
			return FLinearColor(Color->R, Color->G, Color->B, Fallback.A);
		}
	}
	return Fallback;
}

FSlateColor UInvenzaUIThemeLibrary::ThemeSlateColor(FName Role, FSlateColor Fallback)
{
	return FSlateColor(ThemeColor(Role, Fallback.GetSpecifiedColor()));
}

FSlateFontInfo UInvenzaUIThemeLibrary::ThemeFont(FName Role, FSlateFontInfo Fallback)
{
	FSlateFontInfo Resolved;
	if (const auto* Theme = GetSelectedTheme(); Theme && Theme->ResolveFont(Role, Resolved))
	{
		Resolved.OutlineSettings.OutlineColor.A = Fallback.OutlineSettings.OutlineColor.A;
		return Resolved;
	}
	return Fallback;
}

FSlateBrush UInvenzaUIThemeLibrary::ThemeBrush(FName Role, FSlateBrush Fallback)
{
	FSlateBrush Resolved;
	if (const auto* Theme = GetSelectedTheme(); Theme && Theme->ResolveBrush(Role, Resolved))
	{
		FLinearColor Tint = Resolved.TintColor.GetSpecifiedColor();
		Tint.A = Fallback.TintColor.GetSpecifiedColor().A;
		Resolved.TintColor = FSlateColor(Tint);
		return Resolved;
	}
	return Fallback;
}

static FStructProperty* FindInvenzaStyleProperty(UStruct* Type, void*& Container, const FString& Path)
{
	TArray<FString> Parts;
	Path.ParseIntoArray(Parts, TEXT("."), false);
	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		auto* Property = FindFProperty<FStructProperty>(Type, FName(Parts[Index]));
		if (!Property)
		{
			return nullptr;
		}
		Container = Property->ContainerPtrToValuePtr<void>(Container);
		if (Index + 1 == Parts.Num())
		{
			return Property;
		}
		Type = Property->Struct;
	}
	return nullptr;
}

static bool ReadInvenzaStyle(UStruct* Type, void* Container, const FString& Path, FInvenzaUIThemeValue& Out)
{
	auto* Property = FindInvenzaStyleProperty(Type, Container, Path);
	if (!Property)
	{
		return false;
	}
	if (Property->Struct == TBaseStructure<FLinearColor>::Get())
	{
		Out.Type = EInvenzaUIThemeValueType::Color;
		Out.Color = *static_cast<FLinearColor*>(Container);
	}
	else if (Property->Struct == FSlateColor::StaticStruct())
	{
		Out.Type = EInvenzaUIThemeValueType::SlateColor;
		Out.SlateColor = *static_cast<FSlateColor*>(Container);
	}
	else if (Property->Struct == FSlateFontInfo::StaticStruct())
	{
		Out.Type = EInvenzaUIThemeValueType::Font;
		Out.Font = *static_cast<FSlateFontInfo*>(Container);
	}
	else if (Property->Struct == FSlateBrush::StaticStruct())
	{
		Out.Type = EInvenzaUIThemeValueType::Brush;
		Out.Brush = *static_cast<FSlateBrush*>(Container);
	}
	else
	{
		return false;
	}
	return true;
}

static bool WriteInvenzaStyle(UStruct* Type, void* Container, const FString& Path, const FInvenzaUIThemeValue& Value)
{
	auto* Property = FindInvenzaStyleProperty(Type, Container, Path);
	if (!Property)
	{
		return false;
	}
	if (Value.Type == EInvenzaUIThemeValueType::Color && Property->Struct == TBaseStructure<FLinearColor>::Get())
	{
		*static_cast<FLinearColor*>(Container) = Value.Color;
	}
	else if (Value.Type == EInvenzaUIThemeValueType::SlateColor && Property->Struct == FSlateColor::StaticStruct())
	{
		*static_cast<FSlateColor*>(Container) = Value.SlateColor;
	}
	else if (Value.Type == EInvenzaUIThemeValueType::Font && Property->Struct == FSlateFontInfo::StaticStruct())
	{
		*static_cast<FSlateFontInfo*>(Container) = Value.Font;
	}
	else if (Value.Type == EInvenzaUIThemeValueType::Brush && Property->Struct == FSlateBrush::StaticStruct())
	{
		*static_cast<FSlateBrush*>(Container) = Value.Brush;
	}
	else
	{
		return false;
	}
	return true;
}

UWidget* UInvenzaUIThemeLibrary::FindBindingTarget(UInvenzaBaseWidget* Owner,
	const FInvenzaUIThemeBinding& Binding, UWidgetTree* Tree)
{
	if (!Owner)
	{
		return nullptr;
	}
	if (Binding.WidgetName.IsNone())
	{
		return Owner;
	}
	Tree = Tree ? Tree : Owner->WidgetTree.Get();
	return Tree ? Tree->FindWidget(Binding.WidgetName) : nullptr;
}

bool UInvenzaUIThemeLibrary::ReadStyleValue(UObject* Target, const FString& Path, FInvenzaUIThemeValue& OutValue)
{
	return Target && ReadInvenzaStyle(Target->GetClass(), Target, Path, OutValue);
}

bool UInvenzaUIThemeLibrary::WriteStyleValue(UObject* Target, const FString& Path, const FInvenzaUIThemeValue& Value)
{
	return Target && WriteInvenzaStyle(Target->GetClass(), Target, Path, Value);
}

FInvenzaUIThemeBindingResult UInvenzaUIThemeLibrary::ResolveBinding(UInvenzaUITheme* Theme,
	const FInvenzaUIThemeBinding& Binding, const FInvenzaUIThemeValue& Local, bool bUseTheme)
{
	FInvenzaUIThemeBindingResult Result;
	Result.Binding = Binding;
	Result.Value = Local;
	if (!bUseTheme)
	{
		return Result;
	}
	if (!Theme)
	{
		Result.Status = EInvenzaUIThemeBindingStatus::MissingTheme;
		Result.Diagnostic = TEXT("No UI theme selected; using authored style");
		return Result;
	}
	bool bResolved = false;
	FLinearColor Color;
	if (Local.Type == EInvenzaUIThemeValueType::Color || Local.Type == EInvenzaUIThemeValueType::SlateColor)
	{
		if (const auto* Found = Theme->Palette.Find(Binding.Role))
		{
			Color = *Found;
			if (Local.Type == EInvenzaUIThemeValueType::Color)
			{
				Color.A = Local.Color.A;
				Result.Value.Color = Color;
			}
			else
			{
				Color.A = Local.SlateColor.GetSpecifiedColor().A;
				Result.Value.SlateColor = FSlateColor(Color);
			}
			bResolved = true;
		}
	}
	else if (Local.Type == EInvenzaUIThemeValueType::Font)
	{
		bResolved = Theme->ResolveFont(Binding.Role, Result.Value.Font);
		Result.Value.Font.OutlineSettings.OutlineColor.A = Local.Font.OutlineSettings.OutlineColor.A;
	}
	else if (Local.Type == EInvenzaUIThemeValueType::Brush)
	{
		bResolved = Theme->ResolveBrush(Binding.Role, Result.Value.Brush);
		Color = Result.Value.Brush.TintColor.GetSpecifiedColor();
		Color.A = Local.Brush.TintColor.GetSpecifiedColor().A;
		Result.Value.Brush.TintColor = FSlateColor(Color);
		Color = Result.Value.Brush.OutlineSettings.Color.GetSpecifiedColor();
		Color.A = Local.Brush.OutlineSettings.Color.GetSpecifiedColor().A;
		Result.Value.Brush.OutlineSettings.Color = FSlateColor(Color);
		Result.Value.Brush.OutlineSettings.bUseBrushTransparency = Local.Brush.OutlineSettings.bUseBrushTransparency;
	}
	if (bResolved)
	{
		Result.Status = EInvenzaUIThemeBindingStatus::Theme;
	}
	else
	{
		Result.Value = Local;
		const bool bOtherType = Theme->Palette.Contains(Binding.Role) || Theme->Typography.Contains(Binding.Role) || Theme->Brushes.Contains(Binding.Role);
		Result.Status = bOtherType ? EInvenzaUIThemeBindingStatus::WrongRoleType : EInvenzaUIThemeBindingStatus::MissingRole;
		Result.Diagnostic = bOtherType ? TEXT("Role exists in an incompatible theme section; using authored style") : TEXT("Role is missing; using authored style");
	}
	return Result;
}

TArray<FInvenzaUIThemeBindingResult> UInvenzaUIThemeLibrary::ValidateBindings(UInvenzaBaseWidget* Owner, UWidgetTree* Tree)
{
	TArray<FInvenzaUIThemeBindingResult> Results;
	if (!Owner)
	{
		return Results;
	}
	for (int32 Index = 0; Index < Owner->ThemeBindings.Num(); ++Index)
	{
		const auto& Binding = Owner->ThemeBindings[Index];
		FInvenzaUIThemeBindingResult Result;
		Result.Binding = Binding;
		auto* Target = FindBindingTarget(Owner, Binding, Tree);
		void* Value = Target;
		if (!Target)
		{
			Result.Status = EInvenzaUIThemeBindingStatus::MissingWidget;
			Result.Diagnostic = TEXT("Widget does not exist");
		}
		else if (!FindInvenzaStyleProperty(Target->GetClass(), Value, Binding.PropertyPath))
		{
			Result.Status = EInvenzaUIThemeBindingStatus::MissingProperty;
			Result.Diagnostic = TEXT("Property path does not exist or is not a struct style property");
		}
		else if (!ReadStyleValue(Target, Binding.PropertyPath, Result.Value))
		{
			Result.Status = EInvenzaUIThemeBindingStatus::UnsupportedProperty;
			Result.Diagnostic = TEXT("Only color, font and brush properties are supported");
		}
		else
		{
			Result = ResolveBinding(GetSelectedTheme(), Binding, Result.Value, Owner->UsesUITheme());
		}
		for (int32 Earlier = 0; Earlier < Index; ++Earlier)
		{
			const auto& Other = Owner->ThemeBindings[Earlier];
			if (Binding.WidgetName == Other.WidgetName)
			{
				if (Binding.PropertyPath.Equals(Other.PropertyPath, ESearchCase::IgnoreCase))
				{
					Results[Earlier].Status = EInvenzaUIThemeBindingStatus::Duplicate;
					Results[Earlier].Diagnostic = TEXT("Duplicate binding target");
					Result.Status = EInvenzaUIThemeBindingStatus::Duplicate;
					Result.Diagnostic = TEXT("Duplicate binding target");
				}
				else if (Binding.PropertyPath.StartsWith(Other.PropertyPath + TEXT("."), ESearchCase::IgnoreCase) ||
					Other.PropertyPath.StartsWith(Binding.PropertyPath + TEXT("."), ESearchCase::IgnoreCase))
				{
					Results[Earlier].Status = EInvenzaUIThemeBindingStatus::Overlap;
					Results[Earlier].Diagnostic = TEXT("Overlapping parent/child style bindings");
					Result.Status = EInvenzaUIThemeBindingStatus::Overlap;
					Result.Diagnostic = TEXT("Overlapping parent/child style bindings");
				}
			}
		}
		Results.Add(MoveTemp(Result));
	}
	return Results;
}

void UInvenzaUIThemeLibrary::ResolveStructStyle(const UInvenzaBaseWidget* Owner, const FString& Prefix,
	UScriptStruct* Type, void* Value)
{
	if (!Owner)
	{
		return;
	}
	for (const auto& Binding : Owner->ThemeBindings)
	{
		if (!Binding.WidgetName.IsNone() || !Binding.PropertyPath.StartsWith(Prefix + TEXT(".")))
		{
			continue;
		}
		const FString Path = Binding.PropertyPath.RightChop(Prefix.Len() + 1);
		FInvenzaUIThemeValue Local;
		if (ReadInvenzaStyle(Type, Value, Path, Local))
		{
			WriteInvenzaStyle(Type, Value, Path, Owner->ResolveThemeValue(Binding.PropertyPath, Local));
		}
	}
}

static void MergeInvenzaEditedFields(UStruct* Type, void* Authored, const void* Rendered, const void* Edited)
{
	for (TFieldIterator<FProperty> It(Type); It; ++It)
	{
		FProperty* Property = *It;
		void* Destination = Property->ContainerPtrToValuePtr<void>(Authored);
		const void* Previous = Property->ContainerPtrToValuePtr<void>(Rendered);
		const void* Current = Property->ContainerPtrToValuePtr<void>(Edited);
		if (auto* Struct = CastField<FStructProperty>(Property))
		{
			MergeInvenzaEditedFields(Struct->Struct, Destination, Previous, Current);
		}
		else if (!Property->Identical(Previous, Current))
		{
			Property->CopyCompleteValue(Destination, Current);
		}
	}
}

void UInvenzaUIThemeLibrary::MergeEditedValue(FInvenzaUIThemeValue& Authored,
	const FInvenzaUIThemeValue& Rendered, const FInvenzaUIThemeValue& Edited)
{
	MergeInvenzaEditedFields(FInvenzaUIThemeValue::StaticStruct(), &Authored, &Rendered, &Edited);
}

void UInvenzaUIThemeLibrary::SynchronizeAppearance(UWidget* Widget)
{
	if (auto* Input = Cast<UEditableTextBox>(Widget))
	{
		if (const auto Slate = Input->GetCachedWidget())
		{
			StaticCastSharedPtr<SEditableTextBox>(Slate)->SetStyle(&Input->WidgetStyle);
		}
	}
	else if (auto* Text = Cast<UTextBlock>(Widget))
	{
		Text->SetFont(Text->GetFont());
		Text->SetColorAndOpacity(Text->GetColorAndOpacity());
		Text->SetShadowColorAndOpacity(Text->GetShadowColorAndOpacity());
	}
	else if (auto* Border = Cast<UBorder>(Widget))
	{
		Border->SetBrush(Border->Background);
		Border->SetBrushColor(Border->BrushColor);
	}
	else if (auto* Image = Cast<UImage>(Widget))
	{
		Image->SetBrush(Image->GetBrush());
		Image->SetColorAndOpacity(Image->GetColorAndOpacity());
	}
	else if (auto* Button = Cast<UButton>(Widget))
	{
		Button->SetStyle(Button->GetStyle());
		Button->SetBackgroundColor(Button->GetBackgroundColor());
		Button->SetColorAndOpacity(Button->GetColorAndOpacity());
	}
	else if (auto* Progress = Cast<UProgressBar>(Widget))
	{
		Progress->SetWidgetStyle(Progress->GetWidgetStyle());
		Progress->SetFillColorAndOpacity(Progress->GetFillColorAndOpacity());
	}
	else
	{
		Widget->SynchronizeProperties();
	}
}
