//  Nublin Studio 2026 All Rights Reserved.

#include "ActorComponents/LinkedStorageIdentityComponent.h"
#include "GameFramework/PlayerController.h"

bool ULinkedStorageIdentityComponent::SetPersistentPlayerID(const FString& PlayerID)
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority() || !Cast<APlayerController>(GetOwner()) ||
		PlayerID.TrimStartAndEnd().IsEmpty() || PlayerID.Len() > 256 || !PersistentPlayerID.IsEmpty())
	{
		return false;
	}
	PersistentPlayerID = PlayerID.TrimStartAndEnd();
	return true;
}
