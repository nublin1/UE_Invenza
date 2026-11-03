// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "InvenzaUIThemeSubsystem.generated.h"

class UInvenzaUITheme;
class UInvenzaBaseWidget;
struct FInvenzaUIThemeBindingResult;

/** One cached editor-selected theme, also available to Designer without a world or player. */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaUIThemeSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// Functions
	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Theme")
	UInvenzaUITheme* GetTheme();

	void RefreshDesignerWidgets();
	void ReportBindingIssue(const UInvenzaBaseWidget* Owner, const FInvenzaUIThemeBindingResult& Result);

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Transient)
	TObjectPtr<UInvenzaUITheme> CachedTheme;

	FSoftObjectPath CachedPath;
	bool bLoadAttempted = false;
	bool bRefreshingDesigner = false;
	TSet<FString> ReportedBindingIssues;
#if WITH_EDITOR
	FDelegateHandle DesignerEditHandle;
#endif

	// Functions
#if WITH_EDITOR
	void HandleDesignerPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);
#endif
};
