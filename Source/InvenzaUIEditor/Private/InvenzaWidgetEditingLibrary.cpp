//  Nublin Studio 2026 All Rights Reserved.

#include "InvenzaWidgetEditingLibrary.h"
#include "Styling/CoreStyle.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/WidgetAnimation.h"
#include "WidgetBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderingThread.h"
#include "RHITypes.h"
#include "TextureResource.h"
#include "ImageUtils.h"
#include "Modules/ModuleManager.h"
#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "UObject/Script.h"
#include "Input/DragAndDrop.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, InvenzaUIEditor)

FSlateBrush UInvenzaWidgetEditingLibrary::GetCoreStyleBrush(FName BrushName)
{
	return *FCoreStyle::Get().GetBrush(BrushName);
}

bool UInvenzaWidgetEditingLibrary::PressPreviewButton(UButton* Button)
{
	if (!IsValid(Button) || !Button->GetWorld() || !Button->GetWorld()->IsGameWorld() ||
		Button->GetWorld()->bIsTearingDown || !Button->GetIsEnabled())
	{
		return false;
	}

	// Python invocation enables this global, which forces all actor RPCs to run locally.
	TGuardValue<bool> PreviewRPCGuard(GAllowActorScriptExecutionInEditor, false);
	Button->OnPressed.Broadcast();
	return true;
}

void UInvenzaWidgetEditingLibrary::SetStringTableEntry(UStringTable* Table, const FString& Key,
													   const FString& SourceText)
{
	if (!Table)
	{
		return;
	}
	Table->Modify();
	Table->GetMutableStringTable()->SetSourceString(Key, SourceText);
}

FText UInvenzaWidgetEditingLibrary::StringTableText(UStringTable* Table, const FString& Key)
{
	return Table ? FText::FromStringTable(Table->GetStringTableId(), Key) : FText::GetEmpty();
}

static TWeakObjectPtr<UUserWidget> InvenzaWidgetPreviewOwner;
static TSharedPtr<SWidget> InvenzaWidgetPreviewSlate;
static TWeakObjectPtr<APlayerController> InvenzaWidgetPreviewController;
static TSharedPtr<FDragDropOperation> InvenzaWidgetPreviewDrag;
static TWeakObjectPtr<UUserWidget> InvenzaWidgetPreviewDragSource;

void UInvenzaWidgetEditingLibrary::ReleasePreview()
{
	if (InvenzaWidgetPreviewDrag.IsValid() && InvenzaWidgetPreviewDragSource.IsValid() &&
		InvenzaWidgetPreviewDragSource->GetWorld() && !InvenzaWidgetPreviewDragSource->GetWorld()->bIsTearingDown)
	{
		InvenzaWidgetPreviewDrag->OnDrop(false, FPointerEvent());
	}
	InvenzaWidgetPreviewDrag.Reset();
	InvenzaWidgetPreviewDragSource.Reset();
	InvenzaWidgetPreviewSlate.Reset();
	InvenzaWidgetPreviewOwner.Reset();
	if (InvenzaWidgetPreviewController.IsValid())
	{
		InvenzaWidgetPreviewController->Destroy();
	}
	InvenzaWidgetPreviewController.Reset();
}

UUserWidget* UInvenzaWidgetEditingLibrary::CreatePreview(UWorld* World, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!World || !WidgetClass)
	{
		return nullptr;
	}
	// Dynamic ingredient entries require an owning player even in an editor preview.
	if (!World->IsGameWorld())
	{
		if (!InvenzaWidgetPreviewController.IsValid())
		{
			FActorSpawnParameters Params;
			Params.ObjectFlags = RF_Transient;
			Params.bHideFromSceneOutliner = true;
			InvenzaWidgetPreviewController = World->SpawnActor<APlayerController>(Params);
			if (InvenzaWidgetPreviewController.IsValid())
			{
				ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine, NAME_None, RF_Transient);
				Player->PlayerController = InvenzaWidgetPreviewController.Get();
				InvenzaWidgetPreviewController->Player = Player;
				// Editor worlds do not initialize gameplay actors, so the spawned
				// controller must be registered for FLocalPlayerContext lookup.
				World->AddController(InvenzaWidgetPreviewController.Get());
			}
		}
		if (InvenzaWidgetPreviewController.IsValid())
		{
			return CreateWidget<UUserWidget>(InvenzaWidgetPreviewController.Get(), WidgetClass);
		}
	}
	return CreateWidget<UUserWidget>(World, WidgetClass);
}

UWidget* UInvenzaWidgetEditingLibrary::AddTemplate(UWidgetBlueprint* Blueprint, TSubclassOf<UWidget> WidgetClass,
												   FName Name)
{
	if (!Blueprint || !Blueprint->WidgetTree || !WidgetClass)
	{
		return nullptr;
	}
	Blueprint->Modify();
	Blueprint->WidgetTree->Modify();
	if (UWidget* Existing = FindObject<UWidget>(Blueprint->WidgetTree, *Name.ToString()))
	{
		if (Existing->GetClass() == WidgetClass.Get())
		{
			if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name))
			{
				Blueprint->OnVariableAdded(Name);
			}
			return Existing;
		}
		// Preserve the binding name when specializing an embedded Core widget.
		Existing->Modify();
		Existing->RemoveFromParent();
		const FName OldName = MakeUniqueObjectName(Blueprint->WidgetTree, Existing->GetClass(),
												   FName(Name.ToString() + TEXT("_Previous")));
		Existing->Rename(*OldName.ToString(), Blueprint->WidgetTree, REN_DontCreateRedirectors);
	}
	UWidget* Widget = NewObject<UWidget>(Blueprint->WidgetTree, WidgetClass, Name, RF_Transactional);
	Widget->bIsVariable = true;
	if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name))
	{
		Blueprint->OnVariableAdded(Name);
	}
	return Widget;
}

void UInvenzaWidgetEditingLibrary::SetRoot(UWidgetBlueprint* Blueprint, UWidget* Root)
{
	if (!Blueprint || !Blueprint->WidgetTree)
	{
		return;
	}
	Blueprint->WidgetTree->Modify();
	Blueprint->WidgetTree->RootWidget = Root;
}

void UInvenzaWidgetEditingLibrary::SetSlotContent(UUserWidget* Widget, FName SlotName, UWidget* Content)
{
	if (!Widget)
	{
		return;
	}
	Widget->Modify();
	Widget->SetContentForSlot(SlotName, Content);
}

bool UInvenzaWidgetEditingLibrary::CompileWidget(UWidgetBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}
	// Match the compiler's pruning before reconciling variable GUIDs. Detached
	// old layout objects still have WidgetTree as Outer until compilation.
	TSet<UWidget*> Reachable;
	Blueprint->WidgetTree->ForEachWidget(
		[&Reachable](UWidget* Widget)
		{
			Reachable.Add(Widget);
		});
	for (UWidget* Widget : Blueprint->GetAllSourceWidgets())
	{
		if (!Reachable.Contains(Widget))
		{
			Blueprint->OnVariableRemoved(Widget->GetFName());
			Widget->SetFlags(RF_Transient);
			Widget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
		}
	}
	TSet<FName> LiveNames;
	for (UWidget* Widget : Blueprint->GetAllSourceWidgets())
	{
		LiveNames.Add(Widget->GetFName());
	}
	for (UWidgetAnimation* Animation : Blueprint->Animations)
	{
		if (Animation)
		{
			LiveNames.Add(Animation->GetFName());
		}
	}
	TArray<FName> PreviousNames;
	Blueprint->WidgetVariableNameToGuidMap.GetKeys(PreviousNames);
	for (FName Name : PreviousNames)
	{
		if (!LiveNames.Contains(Name))
		{
			Blueprint->OnVariableRemoved(Name);
		}
	}
	for (FName Name : LiveNames)
	{
		if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name))
		{
			Blueprint->OnVariableAdded(Name);
		}
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

bool UInvenzaWidgetEditingLibrary::RenderWidget(UUserWidget* Widget, FVector2D Size, const FString& Filename)
{
	if (!Widget || Size.X <= 0 || Size.Y <= 0)
	{
		return false;
	}
	if (InvenzaWidgetPreviewOwner != Widget || !InvenzaWidgetPreviewSlate)
	{
		InvenzaWidgetPreviewSlate.Reset();
		InvenzaWidgetPreviewOwner.Reset();
		InvenzaWidgetPreviewOwner = Widget;
		InvenzaWidgetPreviewSlate = Widget->TakeWidget();
	}
	FWidgetRenderer Renderer(true);
	// Slate applies display gamma. A linear target avoids applying it a second time.
	UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>();
	Target->ClearColor = FLinearColor::Transparent;
	Target->TargetGamma = 1.f;
	Target->InitCustomFormat(FMath::RoundToInt(Size.X), FMath::RoundToInt(Size.Y), PF_B8G8R8A8, true);
	Target->UpdateResourceImmediate(true);
	Renderer.DrawWidget(Target, InvenzaWidgetPreviewSlate.ToSharedRef(), Size, 0.f);
	FlushRenderingCommands();
	TArray<FColor> Pixels;
	FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
	ReadFlags.SetLinearToGamma(false);
	if (!Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels, ReadFlags))
	{
		return false;
	}
	TArray64<uint8> ImageData;
	FImageUtils::PNGCompressImageArray(Target->SizeX, Target->SizeY, Pixels, ImageData);
	return FFileHelper::SaveArrayToFile(ImageData, *Filename);
}

FVector4 UInvenzaWidgetEditingLibrary::GetPreviewWidgetRect(UWidget* Widget, UWidget* RelativeTo)
{
	if (!IsValid(Widget))
	{
		return FVector4(0, 0, 0, 0);
	}
	const FGeometry& Geometry = Widget->GetPaintSpaceGeometry();
	FVector2D Minimum = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
	FVector2D Maximum = Geometry.LocalToAbsolute(Geometry.GetLocalSize());
	if (IsValid(RelativeTo))
	{
		const FGeometry& ParentGeometry = RelativeTo->GetPaintSpaceGeometry();
		Minimum = ParentGeometry.AbsoluteToLocal(Minimum);
		Maximum = ParentGeometry.AbsoluteToLocal(Maximum);
	}
	return FVector4(Minimum.X, Minimum.Y, Maximum.X - Minimum.X, Maximum.Y - Minimum.Y);
}

bool UInvenzaWidgetEditingLibrary::PreviewDragOver(UUserWidget* SourceWidget, UWidget* SourceCell,
	UUserWidget* TargetWidget, UWidget* HoverTarget, FVector2D NormalizedPosition)
{
	if (!IsValid(SourceWidget) || !IsValid(SourceCell) || !IsValid(TargetWidget) || !IsValid(HoverTarget) ||
		!SourceWidget->GetWorld() || !SourceWidget->GetWorld()->IsGameWorld() ||
		SourceWidget->GetWorld() != TargetWidget->GetWorld())
	{
		return false;
	}
	if (InvenzaWidgetPreviewDrag.IsValid())
	{
		InvenzaWidgetPreviewDrag->OnDrop(false, FPointerEvent());
		InvenzaWidgetPreviewDrag.Reset();
	}
	const TSharedRef<SWidget> SourceSlate = SourceWidget->TakeWidget();
	const FGeometry& SourceGeometry = SourceSlate->GetTickSpaceGeometry();
	const FGeometry& CellGeometry = SourceCell->GetCachedGeometry();
	const FVector2D SourceCursor = CellGeometry.LocalToAbsolute(CellGeometry.GetLocalSize() * 0.5f);
	const TSet<FKey> PressedButtons = {EKeys::LeftMouseButton};
	const FPointerEvent SourcePointer(0, SourceCursor, SourceCursor, PressedButtons,
		EKeys::LeftMouseButton, 0.f, FModifierKeysState());
	SourceSlate->OnMouseButtonDown(SourceGeometry, SourcePointer);
	InvenzaWidgetPreviewDrag = SourceSlate->OnDragDetected(SourceGeometry, SourcePointer).GetDragDropContent();
	InvenzaWidgetPreviewDragSource = SourceWidget;
	if (!InvenzaWidgetPreviewDrag.IsValid())
	{
		return false;
	}
	const TSharedRef<SWidget> TargetSlate = TargetWidget->TakeWidget();
	const FGeometry& TargetGeometry = HoverTarget->GetCachedGeometry();
	const FVector2D Cursor = TargetGeometry.LocalToAbsolute(TargetGeometry.GetLocalSize() * NormalizedPosition);
	const FPointerEvent Pointer(0, Cursor, Cursor, PressedButtons, EKeys::LeftMouseButton, 0.f, FModifierKeysState());
	const FDragDropEvent DragEvent(Pointer, InvenzaWidgetPreviewDrag);
	const FGeometry& Geometry = TargetSlate->GetTickSpaceGeometry();
	TargetSlate->OnDragEnter(Geometry, DragEvent);
	return TargetSlate->OnDragOver(Geometry, DragEvent).IsEventHandled();
}

bool UInvenzaWidgetEditingLibrary::CompletePreviewDrop(UUserWidget* TargetWidget, UWidget* TargetCell)
{
	if (!IsValid(TargetWidget) || !IsValid(TargetCell) || !InvenzaWidgetPreviewDrag.IsValid() ||
		!TargetWidget->GetWorld() || !TargetWidget->GetWorld()->IsGameWorld())
	{
		return false;
	}
	TGuardValue<bool> PreviewRPCGuard(GAllowActorScriptExecutionInEditor, false);
	const TSharedRef<SWidget> TargetSlate = TargetWidget->TakeWidget();
	const FGeometry& CellGeometry = TargetCell->GetCachedGeometry();
	const FVector2D Cursor = CellGeometry.LocalToAbsolute(CellGeometry.GetLocalSize() * 0.5f);
	const FPointerEvent Pointer(0, Cursor, Cursor, TSet<FKey>(), EKeys::LeftMouseButton, 0.f,
		FModifierKeysState());
	const FDragDropEvent DragEvent(Pointer, InvenzaWidgetPreviewDrag);
	const bool bHandled = TargetSlate->OnDrop(TargetSlate->GetTickSpaceGeometry(), DragEvent).IsEventHandled();
	InvenzaWidgetPreviewDrag->OnDrop(bHandled, Pointer);
	InvenzaWidgetPreviewDrag.Reset();
	InvenzaWidgetPreviewDragSource.Reset();
	return bHandled;
}
