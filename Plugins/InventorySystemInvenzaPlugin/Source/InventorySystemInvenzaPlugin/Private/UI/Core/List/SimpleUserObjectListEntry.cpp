// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/List/SimpleUserObjectListEntry.h"

#include "UI/Core/LabelBaseText.h"
#include "UI/Core/Image/ImageBaseWidget.h"

void USimpleUserObjectListEntry::UpdateImage(const TSoftObjectPtr<UTexture2D>& NewIcon)
{
	if (!ListEntry_Image || NewIcon.IsNull())
		return;

	UTexture2D* LoadedTexture = NewIcon.LoadSynchronous();
	if (!LoadedTexture)
		return;

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(LoadedTexture);
	NewBrush.ImageSize = FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());
	ListEntry_Image->UpdateBrush(NewBrush);
}

void USimpleUserObjectListEntry::UpdateText(const FText& Text)
{
	if (!ListEntry_Text)
		return;

	ListEntry_Text->UpdateText(Text);
}
