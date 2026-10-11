#include "InvenzaWidgetEditingLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/WidgetAnimation.h"
#include "WidgetBlueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Modules/ModuleManager.h"
#include "Serialization/BufferArchive.h"
#include "Misc/FileHelper.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, InvenzaUIEditor)

namespace
{
    TWeakObjectPtr<UUserWidget> PreviewOwner;
    TSharedPtr<SWidget> PreviewSlate;
    TWeakObjectPtr<APlayerController> PreviewController;
}

void UInvenzaWidgetEditingLibrary::ReleasePreview()
{
    PreviewSlate.Reset();
    PreviewOwner.Reset();
    if (PreviewController.IsValid()) PreviewController->Destroy();
    PreviewController.Reset();
}

UUserWidget* UInvenzaWidgetEditingLibrary::CreatePreview(UWorld* World, TSubclassOf<UUserWidget> WidgetClass)
{
    if (!World || !WidgetClass) return nullptr;
    // Dynamic ingredient entries require an owning player even in an editor preview.
    if (!World->IsGameWorld())
    {
        if (!PreviewController.IsValid())
        {
            FActorSpawnParameters Params;
            Params.ObjectFlags = RF_Transient;
            Params.bHideFromSceneOutliner = true;
            PreviewController = World->SpawnActor<APlayerController>(Params);
            if (PreviewController.IsValid())
            {
                ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine, NAME_None, RF_Transient);
                Player->PlayerController = PreviewController.Get();
                PreviewController->Player = Player;
                // Editor worlds do not initialize gameplay actors, so the spawned
                // controller must be registered for FLocalPlayerContext lookup.
                World->AddController(PreviewController.Get());
            }
        }
        if (PreviewController.IsValid()) return CreateWidget<UUserWidget>(PreviewController.Get(), WidgetClass);
    }
    return CreateWidget<UUserWidget>(World, WidgetClass);
}

UWidget* UInvenzaWidgetEditingLibrary::AddTemplate(UWidgetBlueprint* Blueprint, TSubclassOf<UWidget> WidgetClass, FName Name)
{
    if (!Blueprint || !Blueprint->WidgetTree || !WidgetClass) return nullptr;
    Blueprint->Modify();
    Blueprint->WidgetTree->Modify();
    if (UWidget* Existing = FindObject<UWidget>(Blueprint->WidgetTree, *Name.ToString()))
    {
        if (Existing->GetClass() == WidgetClass.Get())
        {
            if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name)) Blueprint->OnVariableAdded(Name);
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
    if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name)) Blueprint->OnVariableAdded(Name);
    return Widget;
}

void UInvenzaWidgetEditingLibrary::SetRoot(UWidgetBlueprint* Blueprint, UWidget* Root)
{
    if (!Blueprint || !Blueprint->WidgetTree) return;
    Blueprint->WidgetTree->Modify();
    Blueprint->WidgetTree->RootWidget = Root;
}

void UInvenzaWidgetEditingLibrary::SetSlotContent(UUserWidget* Widget, FName SlotName, UWidget* Content)
{
    if (!Widget) return;
    Widget->Modify();
    Widget->SetContentForSlot(SlotName, Content);
}

bool UInvenzaWidgetEditingLibrary::CompileWidget(UWidgetBlueprint* Blueprint)
{
    if (!Blueprint) return false;
    // Match the compiler's pruning before reconciling variable GUIDs. Detached
    // old layout objects still have WidgetTree as Outer until compilation.
    TSet<UWidget*> Reachable;
    Blueprint->WidgetTree->ForEachWidget([&Reachable](UWidget* Widget) { Reachable.Add(Widget); });
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
        if (Animation) LiveNames.Add(Animation->GetFName());
    TArray<FName> PreviousNames;
    Blueprint->WidgetVariableNameToGuidMap.GetKeys(PreviousNames);
    for (FName Name : PreviousNames)
        if (!LiveNames.Contains(Name)) Blueprint->OnVariableRemoved(Name);
    for (FName Name : LiveNames)
        if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name)) Blueprint->OnVariableAdded(Name);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
    return Blueprint->Status != BS_Error;
}

bool UInvenzaWidgetEditingLibrary::RenderWidget(UUserWidget* Widget, FVector2D Size, const FString& Filename)
{
    if (!Widget || Size.X <= 0 || Size.Y <= 0) return false;
    if (PreviewOwner != Widget || !PreviewSlate)
    {
        PreviewSlate.Reset();
        PreviewOwner.Reset();
        PreviewOwner = Widget;
        PreviewSlate = Widget->TakeWidget();
    }
    FWidgetRenderer Renderer(true);
    // Slate applies display gamma. A linear target avoids applying it a second time.
    UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>();
    Target->ClearColor = FLinearColor::Transparent;
    Target->TargetGamma = 1.f;
    Target->InitCustomFormat(FMath::RoundToInt(Size.X), FMath::RoundToInt(Size.Y), PF_B8G8R8A8, true);
    Target->UpdateResourceImmediate(true);
    Renderer.DrawWidget(Target, PreviewSlate.ToSharedRef(), Size, 0.f);
    FlushRenderingCommands();
    TArray<FColor> Pixels;
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
    ReadFlags.SetLinearToGamma(false);
    if (!Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels, ReadFlags)) return false;
    TArray64<uint8> ImageData;
    FImageUtils::PNGCompressImageArray(Target->SizeX, Target->SizeY, Pixels, ImageData);
    return FFileHelper::SaveArrayToFile(ImageData, *Filename);
}
