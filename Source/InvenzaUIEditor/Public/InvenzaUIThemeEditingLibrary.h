// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/Theme/InvenzaUIThemeTypes.h"
#include "InvenzaUIThemeEditingLibrary.generated.h"

class UWidgetBlueprint;
class UBlueprint;
class UInvenzaUITheme;
class UInvenzaBaseWidget;
class UWidgetTree;

/** Scoped editor authoring and audit; never saves packages implicitly. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaUIThemeEditingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static FString InspectBlueprint(UWidgetBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool ConfigureWidget(UObject* Widget, const TArray<FInvenzaUIThemeBinding>& Bindings,
		EInvenzaUIThemeSource Source);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool SetThemeStyle(UInvenzaUITheme* Theme, FName Role, const FString& Type,
		const FString& Value, FName ColorRole, bool bUseDefaultFont);

	/** Preserve native struct defaults and non-text-exported brush fields exactly. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool CopyThemeStyle(UInvenzaUITheme* Theme, FName Role, UObject* Source,
		const FString& PropertyPath, FName ColorRole, bool bUseDefaultFont);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool ConnectThemePin(UBlueprint* Blueprint, const FString& Graph, const FString& NodeID,
		FName PinName, FName Role);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool AdoptBaseWidget(UWidgetBlueprint* Blueprint);

	/** Compile only this explicitly edited Blueprint, without pruning or restructuring its tree. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool CompileThemeBlueprint(UWidgetBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static TArray<FInvenzaUIThemeBindingResult> ValidateThemeWidget(UInvenzaBaseWidget* Widget, UWidgetTree* Tree);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static void MakeDesignerPreview(UInvenzaBaseWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Theme Editing")
	static bool EditPreviewStyle(UInvenzaBaseWidget* Owner, FName WidgetName, const FString& PropertyPath, const FInvenzaUIThemeValue& Value);
};
