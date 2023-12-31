

#pragma once

#include <PlayerEquipment/EquipmentManager.h>
#include <Character/IdleCharacter.h>
#include "Game/SpawnManager.h"
#include "Core/PlayFabClientDataModels.h"
#include "Core/PlayFabClientAPI.h"
#include <PlayFab.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayFabManager.generated.h"


//class ASpawnManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnEssenceUpdate, int32, Wisdom, int32, Temperance, int32, Justice, int32, Courage, int32, Legendary);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurchaseCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEssenceTransferredPlayFab, const TArray<FEssenceCoffer>&, EssenceCofferArray);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryLoaded, const TArray<FName>&, InventoryRowNames);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestVersionRetrieved, FString, QuestID, FString, QuestVersion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestDataReady);
/**
 * 
 */
UCLASS(Blueprintable)
class IDLEADVENTURE_API APlayFabManager : public AActor
{
	GENERATED_BODY()


public:

	APlayFabManager();
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	//Singleton pattern
	static APlayFabManager* GetInstance(UWorld* World);
	void ResetInstance();

	UFUNCTION(BlueprintCallable, Category = "PlayFab")
	bool PurchaseEquipment(const FString& EquipmentName, const FEquipmentData& EquipmentData);
	bool UpdatePlayFabEssenceCount(const FEquipmentData& EquipmentData);
	bool UpdatePlayerEquipmentInventory(const TArray<FEquipmentData>& NewPlayerEquipmentInventory);
	void LoadPlayerEquipmentInventory();
	void OnSuccessFetchInventory(const PlayFab::ClientModels::FGetUserDataResult& Result);
	void OnErrorFetchInventory(const PlayFab::FPlayFabCppError& Error);

	//Update and fetch data to/from PlayFab
	bool TryPlayFabUpdate(FName EssenceType, int32 NewCount);
	void OnSuccessUpdateEssence(const PlayFab::ClientModels::FUpdateUserDataResult& Result);
	void OnErrorUpdateEssence(const PlayFab::FPlayFabCppError& Error);
	void OnSuccessUpdateInventory(const PlayFab::ClientModels::FUpdateUserDataResult& Result);
	void OnErrorUpdateInventory(const PlayFab::FPlayFabCppError& Error);
	TSharedPtr<FJsonObject> TMapToJsonObject(const TMap<FName, int32>& Map);
	bool UpdateEssenceAddedToCofferOnPlayFab();
	void InitializeEssenceCounts();
	//PlayFab helper classes
	FString ConvertToPlayFabFormat(const TArray<FEquipmentData>& EquipmentDataArray);
	TArray<FName> ConvertFromPlayFabFormat(const FString& PlayFabData);

	//Quest Logic
	void SaveQuestStatsToPlayFab(const FString& QuestID, const FString& CompletionDate);
	void OnFetchedCompletedQuestsBeforeSaving(const PlayFab::ClientModels::FGetUserDataResult& Result);
	void OnUpdateQuestStatsSuccess(const PlayFab::ClientModels::FUpdateUserDataResult& Result);
	void OnUpdateQuestStatsFailure(const PlayFab::FPlayFabCppError& ErrorResult);
	void CanAcceptQuest(UQuest* Quest, AIdleCharacter* Player);
	void CompleteQuest(UQuest* Quest);
	void GetCompletedQuestVersion(FString QuestID, AIdleCharacter* Player);
	void OnGetQuestVersionSuccess(const PlayFab::ClientModels::FGetUserDataResult& Result, AIdleCharacter* Player);
	void OnGetQuestVersionFailure(const PlayFab::FPlayFabCppError& ErrorResult);
	void MarkQuestAsCompleted(FString QuestID, FString Version);
	void CheckIfQuestCompleted(UQuest* Quest, AIdleCharacter* Player);
	bool NeedsReset(const FString& LastCompletedDate);

	void FetchCompletedQuestsData();
	void OnFetchCompletedQuestsDataSuccess(const PlayFab::ClientModels::FGetUserDataResult& Result);
	void OnFetchCompletedQuestsDataFailure(const PlayFab::FPlayFabCppError& ErrorResult);


	//Basic fetch and update methods
	// Method to initiate fetching current essence counts from PlayFab
	void FetchCurrentEssenceCounts();
	// Callback for successful fetch of essence counts
	void OnFetchCurrentEssenceCountsSuccess(const PlayFab::ClientModels::FGetUserDataResult& Result);
	// Callback for failed fetch of essence counts
	void OnFetchCurrentEssenceCountsFailure(const PlayFab::FPlayFabCppError& Error);
	// Method to update essence counts on PlayFab
	void UpdateEssenceCountsOnPlayFab(const TMap<FName, int32>& NewEssenceCounts);
	void OnSuccessUpdateEssenceCounts(const PlayFab::ClientModels::FUpdateUserDataResult& Result);
	void OnErrorUpdateEssenceCounts(const PlayFab::FPlayFabCppError& Error);

	void UpdatePlayerRewardsOnPlayFab(FRunCompleteRewards Rewards);
	void CombineAndSendUpdatedCounts(const TMap<FName, int32>& CurrentEssenceCounts);

	int32 WisdomToAdd = 0;
	int32 TemperanceToAdd = 0;
	int32 JusticeToAdd = 0;
	int32 CourageToAdd = 0;
	int32 LegendaryToAdd = 0;


	FString PendingQuestID;
	FString PendingCompletionDate;

	UPROPERTY(BlueprintAssignable)
	FOnQuestVersionRetrieved OnQuestVersionRetrieved;

	//Delegate broadcasts to UI
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPurchaseCompleted OnPurchaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEssenceTransferredPlayFab OnEssenceTransferred;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryLoaded OnInventoryLoaded;

	AIdleCharacter* Character;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TArray<FName> DataTableRowNames;
	
	TMap<FString, FString> PlayerCompletedQuestsData;
	FOnQuestDataReady OnQuestDataReady;

	TMap<FString, FString> CompletedQuests;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEssenceUpdate OnEssenceUpdate;

private:

	PlayFabClientPtr clientAPI = nullptr;

	// Static variable to hold the singleton instance
	static APlayFabManager* SingletonInstance;

	//Only broadcast to the UI once, keep track of a count
	int32 SuccessfulUpdateCount;
	
};
