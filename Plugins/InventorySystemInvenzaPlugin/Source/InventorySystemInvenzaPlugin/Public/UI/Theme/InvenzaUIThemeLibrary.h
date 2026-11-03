// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Theme/InvenzaUIThemeTypes.h"
#include "InvenzaUIThemeLibrary.generated.h"

class UInvenzaUITheme;
class UInvenzaBaseWidget;
class UWidget;

UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaUIThemeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	static UInvenzaUITheme* GetSelectedTheme();

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	static FLinearColor ThemeColor(FName Role, FLinearColor Fallback);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	static FSlateColor ThemeSlateColor(FName Role, FSlateColor Fallback);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	static FSlateFontInfo ThemeFont(FName Role, FSlateFontInfo Fallback);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	static FSlateBrush ThemeBrush(FName Role, FSlateBrush Fallback);

	/** Shared resolver/validator for runtime widgets and editor asset validation. */
	static UWidget* FindBindingTarget(UInvenzaBaseWidget* Owner, const FInvenzaUIThemeBinding& Binding,
		class UWidgetTree* Tree = nullptr);
	static bool ReadStyleValue(UObject* Target, const FString& Path, FInvenzaUIThemeValue& OutValue);
	static bool WriteStyleValue(UObject* Target, const FString& Path, const FInvenzaUIThemeValue& Value);
	static FInvenzaUIThemeBindingResult ResolveBinding(UInvenzaUITheme* Theme,
		const FInvenzaUIThemeBinding& Binding, const FInvenzaUIThemeValue& Local, bool bUseTheme);
	static TArray<FInvenzaUIThemeBindingResult> ValidateBindings(UInvenzaBaseWidget* Owner,
		class UWidgetTree* Tree = nullptr);
	static void ResolveStructStyle(const UInvenzaBaseWidget* Owner, const FString& Prefix,
		UScriptStruct* Type, void* Value);
	static void MergeEditedValue(FInvenzaUIThemeValue& Authored, const FInvenzaUIThemeValue& Rendered,
		const FInvenzaUIThemeValue& Edited);
	static void SynchronizeAppearance(class UWidget* Widget);
};
