//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Frontend/FrontendTypes.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "TimerManager.h"
#include "InvenzaSessionsSubsystem.generated.h"

class UNetDriver;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvenzaSessionsChanged);
#pragma endregion Delegates

UCLASS()
class UE_GRIDINVENTORY_API UInvenzaSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// Properties and variables
	UPROPERTY(BlueprintAssignable)
	FInvenzaSessionsChanged OnChanged;
	UPROPERTY(BlueprintReadOnly)
	TArray<FInvenzaServerInfo> Servers;
	UPROPERTY(BlueprintReadOnly)
	FString Status;
	UPROPERTY(BlueprintReadOnly)
	bool bBusy = false;

	// Functions
	UFUNCTION(BlueprintPure)
	bool IsSteam() const;
	UFUNCTION(BlueprintCallable)
	void Host(const FString& Name, int32 Capacity = 4);
	UFUNCTION(BlueprintCallable)
	void Find();
	UFUNCTION(BlueprintCallable)
	void Join(int32 Index);
	UFUNCTION(BlueprintCallable)
	void ConnectIP(const FString& Address);
	UFUNCTION(BlueprintCallable)
	void Leave();
	UFUNCTION(BlueprintCallable)
	void CancelSearch();
	static const TCHAR* MenuMap();
	static const TCHAR* PlayMap();

protected:
	// Properties and variables
	// Runtime
	IOnlineSessionPtr Sessions;
	TSharedPtr<FOnlineSessionSearch> Search;
	TArray<FOnlineSessionSearchResult> Results;
	FDelegateHandle CreateHandle, FindHandle, JoinHandle, DestroyHandle;
	FDelegateHandle NetworkHandle, TravelHandle, MapHandle;
	FTimerHandle Timeout;
	FString FailureReason;
	EInvenzaSessionOperation Operation = EInvenzaSessionOperation::None;

	// Functions
	void SetStatus(const FString& Message, bool Busy = false);
	bool Ready();
	void ArmTimeout();
	void ClearOperations();
	void TimedOut();
	void Created(FName Name, bool Success);
	void Found(bool Success);
	void Joined(FName Name, EOnJoinSessionCompleteResult::Type Result);
	void Destroyed(FName Name, bool Success);
	void Loaded(UWorld* World);
	void NetworkFailed(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Message);
	void TravelFailed(UWorld* World, ETravelFailure::Type Type, const FString& Message);
	void ReturnToMenu();
};
