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

IMPLEMENT_MODULE(FDefaultModuleImpl, InvenzaUIEditor)

namespace
{
    TWeakObjectPtr<UUserWidget> PreviewOwner;
    TSharedPtr<SWidget> PreviewSlate;
}

void UInvenzaWidgetEditingLibrary::ReleasePreview()
{
    PreviewSlate.Reset();
    PreviewOwner.Reset();
}

UUserWidget* UInvenzaWidgetEditingLibrary::CreatePreview(UWorld* World, TSubclassOf<UUserWidget> WidgetClass)
{
    return World && WidgetClass ? CreateWidget<UUserWidget>(World, WidgetClass) : nullptr;
}

UWidget* UInvenzaWidgetEditingLibrary::AddTemplate(UWidgetBlueprint* Blueprint, TSubclassOf<UWidget> WidgetClass, FName Name)
{
    if (!Blueprint || !Blueprint->WidgetTree || !WidgetClass) return nullptr;
    Blueprint->Modify();
    Blueprint->WidgetTree->Modify();
    if (UWidget* Existing = FindObject<UWidget>(Blueprint->WidgetTree, *Name.ToString()))
    {
        if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Name)) Blueprint->OnVariableAdded(Name);
        return Existing;
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
        ReleasePreview();
        PreviewOwner = Widget;
        PreviewSlate = Widget->TakeWidget();
    }
    FWidgetRenderer Renderer(true);
    UTextureRenderTarget2D* Target = Renderer.DrawWidget(PreviewSlate.ToSharedRef(), Size);
    if (!Target) return false;
    FlushRenderingCommands();
    TArray<FColor> Pixels;
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
    ReadFlags.SetLinearToGamma(false);
    if (!Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels, ReadFlags)) return false;
    TArray64<uint8> ImageData;
    FImageUtils::PNGCompressImageArray(Target->SizeX, Target->SizeY, Pixels, ImageData);
    return FFileHelper::SaveArrayToFile(ImageData, *Filename);
}
