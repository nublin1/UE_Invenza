// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "UI/Theme/InvenzaUIThemeTypes.h"
#include "InvenzaUITheme.generated.h"

/** Editor-selected UI appearance. Contains no texts, item icons, input or layout behavior. */
UCLASS(BlueprintType)
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaUITheme : public UDataAsset
{
	GENERATED_BODY()

protected:
	// Unreal overrides
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Typography", meta = (AllowedClasses = "/Script/Engine.Font,/Script/Engine.FontFace"))
	TObjectPtr<UObject> DefaultFont;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Palette")
	TMap<FName, FLinearColor> Palette;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Typography")
	TMap<FName, FInvenzaUIThemeFont> Typography;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme|Brushes")
	TMap<FName, FInvenzaUIThemeBrush> Brushes;

	// Functions
	bool ResolveFont(FName Role, FSlateFontInfo& OutFont) const;
	bool ResolveBrush(FName Role, FSlateBrush& OutBrush) const;
};
