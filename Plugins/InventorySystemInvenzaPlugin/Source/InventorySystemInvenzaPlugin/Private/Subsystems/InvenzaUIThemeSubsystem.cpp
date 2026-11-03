// Nublin Studio 2026 All Rights Reserved.

#include "Subsystems/InvenzaUIThemeSubsystem.h"

#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Theme/InvenzaUITheme.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UObjectGlobals.h"

void UInvenzaUIThemeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if WITH_EDITOR
	DesignerEditHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &ThisClass::HandleDesignerPropertyChanged);
#endif
}

#if WITH_EDITOR
void UInvenzaUIThemeSubsystem::HandleDesignerPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	if (!Object || !Object->IsA<UWidget>())
	{
		return;
	}
	for (TObjectIterator<UInvenzaBaseWidget> It; It; ++It)
	{
		It->RecordDesignerStyleEdit(Object, Event);
	}
}
#endif

UInvenzaUITheme* UInvenzaUIThemeSubsystem::GetTheme()
{
	const auto& Selected = GetDefault<UInvenzaInventoryDeveloperSettings>()->UITheme;
	const auto Path = Selected.ToSoftObjectPath();
	if (Path != CachedPath)
	{
		CachedTheme = nullptr;
		CachedPath = Path;
		bLoadAttempted = false;
	}
	if (!bLoadAttempted)
	{
		bLoadAttempted = true;
		CachedTheme = Selected.LoadSynchronous();
		if (!Path.IsNull() && !CachedTheme)
		{
			UE_LOG(LogTemp, Warning, TEXT("Invenza UI theme could not be loaded: %s. Keeping local widget styles."), *Path.ToString());
		}
	}
	return CachedTheme;
}

void UInvenzaUIThemeSubsystem::ReportBindingIssue(const UInvenzaBaseWidget* Owner, const FInvenzaUIThemeBindingResult& Result)
{
	const FString Key = Owner->GetClass()->GetPathName() + CachedPath.ToString() + Result.Binding.WidgetName.ToString() +
		Result.Binding.PropertyPath + Result.Binding.Role.ToString() + Result.Diagnostic;
	if (!ReportedBindingIssues.Contains(Key))
	{
		ReportedBindingIssues.Add(Key);
		UE_LOG(LogTemp, Warning, TEXT("UI theme %s -> %s -> %s [role %s]: %s"), *Owner->GetClass()->GetPathName(),
			*Result.Binding.WidgetName.ToString(), *Result.Binding.PropertyPath, *Result.Binding.Role.ToString(), *Result.Diagnostic);
	}
}

void UInvenzaUIThemeSubsystem::RefreshDesignerWidgets()
{
#if WITH_EDITOR
	if (bRefreshingDesigner)
	{
		return;
	}
	TGuardValue<bool> Guard(bRefreshingDesigner, true);
	for (TObjectIterator<UInvenzaBaseWidget> It; It; ++It)
	{
		if (!It->IsTemplate() && It->IsDesignTime())
		{
			It->ApplyUITheme();
		}
	}
#endif
}

void UInvenzaUIThemeSubsystem::Deinitialize()
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(DesignerEditHandle);
#endif
	CachedTheme = nullptr;
	CachedPath.Reset();
	Super::Deinitialize();
}
