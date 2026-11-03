// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "InvenzaUIThemeTypes.generated.h"

UENUM(BlueprintType)
enum class EInvenzaUIThemeSource : uint8
{
	Local,
	Theme,
	ThemeWithOverrides
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeFont
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	bool bUseDefaultFont = true;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeBrush
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FName ColorRole;
};

/** Explicit binding to an authored widget property, never inferred from its runtime value. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeBinding
{
	GENERATED_BODY()

	/** Empty targets the owning UserWidget; otherwise a name in its own WidgetTree. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FName WidgetName;

	/** Reflected style property, e.g. Color, FontInfo or WidgetStyle.Normal.TintColor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FString PropertyPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FName Role;

	/** Keeps the authored value when the widget uses ThemeWithOverrides. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	bool bOverrideLocally = false;
};


UENUM(BlueprintType)
enum class EInvenzaUIThemeValueType : uint8
{
	None, Color, SlateColor, Font, Brush
};

UENUM(BlueprintType)
enum class EInvenzaUIThemeBindingStatus : uint8
{
	Local, Theme, MissingTheme, MissingWidget, MissingProperty, UnsupportedProperty, MissingRole, WrongRoleType,
	Duplicate, Overlap
};

/** Typed authoring/render values; resource references remain visible to garbage collection. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeValue
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	EInvenzaUIThemeValueType Type = EInvenzaUIThemeValueType::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FLinearColor Color = FLinearColor::White;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FSlateColor SlateColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FSlateFontInfo Font;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FSlateBrush Brush;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeBindingResult
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FInvenzaUIThemeBinding Binding;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	EInvenzaUIThemeBindingStatus Status = EInvenzaUIThemeBindingStatus::Local;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FInvenzaUIThemeValue Value;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Theme")
	FString Diagnostic;
};

/** Per-instance backing store for raw UMG components; never serialized into a Widget Blueprint. */
USTRUCT()
struct INVENTORYSYSTEMINVENZAPLUGIN_API FInvenzaUIThemeLocalValue
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> Target;
	UPROPERTY(Transient)
	FName WidgetName;
	UPROPERTY(Transient)
	FString PropertyPath;
	UPROPERTY(Transient)
	FInvenzaUIThemeValue Authored;
	UPROPERTY(Transient)
	FInvenzaUIThemeValue Rendered;
};
