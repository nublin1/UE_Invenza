//  Nublin Studio 2026 All Rights Reserved.


#include "Data/Settings/InvenzaInventorySettingsAsset.h"

#include "UI/Core/Modal/ModalTypes.h"

const FBlockReasonData* UInvenzaInventorySettingsAsset::FindBlockReason(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return nullptr;
	}

	return AvailableBlockReasons.FindByPredicate(
		[&Tag](const FBlockReasonData& Data)
		{
			return Data.Tag == Tag;
		});
}