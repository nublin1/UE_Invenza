// Nublin Studio 2026 All Rights Reserved.
#pragma once
#include "EditorValidatorBase.h"
#include "InvenzaUIThemeValidator.generated.h"
UCLASS()
class INVENZAUITHEMEEDITOR_API UInvenzaUIThemeValidator : public UEditorValidatorBase
{
	GENERATED_BODY()
protected:
	// Unreal overrides
	virtual bool CanValidateAsset_Implementation(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& Context) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& Context) override;
};
