#include "Player/IdlePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/IdleAbilitySystemComponent.h"
#include "AbilitySystem/IdleAttributeSet.h"
#include <Game/IdleGameModeBase.h>
//#include <Leaderboard/LeaderboardManager.h>
#include <AbilitySystem/Abilities/WoodcuttingAbility.h>
#include <UI/HUD/IdleHUD.h>
#include <Player/IdlePlayerController.h>
#include <UI/IdleWidgetController.h>
#include <Leaderboard/PlayLoginActor.h>
#include <Kismet/GameplayStatics.h>
#include <Save/IdleSaveGame.h>


AIdlePlayerState::AIdlePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UIdleAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UIdleAttributeSet>("AttributeSet");

	NetUpdateFrequency = 100.f;
	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);

	//LeaderboardManager = CreateDefaultSubobject<ULeaderboardManager>("LeaderboardManager");

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		IdleAttributeSet->GetWoodcutExpAttribute()).AddUObject(this, &AIdlePlayerState::CheckForLevelUp);

}

void AIdlePlayerState::BeginPlay()
{
	/*
	// Instantiate Woodcutting Ability
	WoodcuttingAbility = NewObject<UWoodcuttingAbility>(this, UWoodcuttingAbility::StaticClass());
	
	if (WoodcuttingAbility)
	{
		// Initialize ability system
		if (AbilitySystemComponent)
		{
			// Setup ability system component, bind to ASC if needed
			// ...

			// Grant the Woodcutting Ability to this player
			FGameplayAbilitySpec WoodcuttingSpec(WoodcuttingAbility, 1, INDEX_NONE);
			AbilitySystemComponent->GiveAbility(WoodcuttingSpec);

			// Notify the ability system globals about this actor having an ability system component
			//UAbilitySystemGlobals::Get().InitAbilityActorInfo(this, this);
		}
	}

	//Instantiate the conversion ability
	ConversionAbility = NewObject<UConversionAbility>(this, UConversionAbility::StaticClass());

	if (ConversionAbility)
	{
		if (AbilitySystemComponent)
		{
			//Grant the conversion ability to this player
			FGameplayAbilitySpec ConversionSpec(ConversionAbility, 1, INDEX_NONE);
			AbilitySystemComponent->GiveAbility(ConversionSpec);
		}
	}
	*/
	// Handle auto save
	LoadExp();
	GetWorld()->GetTimerManager().SetTimer(SaveGameTimerHandle, this, &AIdlePlayerState::AutoSaveGame, 10.f, true);
	
	//Set initial values
	InitializePlayerValues();
}


UAbilitySystemComponent* AIdlePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AIdlePlayerState::GiveAbility(const FGameplayAbilityInfo& AbilityInfo)
{
	//UE_LOG(LogTemp, Warning, TEXT("GIVE ABILITY PS"));
	FGameplayAbilitySpec Spec(AbilityInfo.Ability, AbilityInfo.Level, AbilityInfo.InputID, this);
	AbilitySystemComponent->GiveAbility(Spec);
	AbilityHandles.Add(AbilityInfo.Ability, Spec.Handle);
	GrantedAbilities.Add(AbilityInfo);
}

void AIdlePlayerState::ActivateAbility(TSubclassOf<UGameplayAbility> AbilityToActivate)
{
	FGameplayAbilitySpecHandle* FoundHandle = AbilityHandles.Find(AbilityToActivate);
	if (FoundHandle)
	{
		AbilitySystemComponent->TryActivateAbility(*FoundHandle);
		//UE_LOG(LogTemp, Warning, TEXT("ACTIVATE ABILITY PS"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability not found in player state"));
	}
}

void AIdlePlayerState::DeactivateAbility(TSubclassOf<UGameplayAbility> AbilityToDeactivate)
{
	FGameplayAbilitySpecHandle* FoundHandle = AbilityHandles.Find(AbilityToDeactivate);
	if (FoundHandle)
	{
		// Get the spec using the handle
		FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(*FoundHandle);

		// Check if we got a valid spec
		if (AbilitySpec && AbilitySpec->IsActive())
		{
			// Get the primary active instance (if there are multiple instances of this ability, you might need more complex logic)
			UGameplayAbility* ActiveAbility = AbilitySpec->GetPrimaryInstance();

			// Cancel the ability
			if (ActiveAbility)
			{
				ActiveAbility->CancelAbility(*FoundHandle, ActiveAbility->GetCurrentActorInfo(), ActiveAbility->GetCurrentActivationInfo(), true); // Assuming you want to replicate
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability not found in player state"));
	}
}

bool AIdlePlayerState::IsAbilityActive(TSubclassOf<UGameplayAbility> AbilityClass)
{
	FGameplayAbilitySpecHandle* FoundHandle = AbilityHandles.Find(AbilityClass);
	if (FoundHandle)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void AIdlePlayerState::InitializePlayerValues()
{
	LoadExp();

	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	OnExpGained.Broadcast(IdleAttributeSet->GetWoodcutExp());
	WhenLevelUp.Broadcast(IdleAttributeSet->GetWoodcuttingLevel());
}

void AIdlePlayerState::CheckForLevelUp(const FOnAttributeChangeData& Data) const
{
	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	FString PlayerName = TEXT("PlayerName");

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LoginActorTag"), FoundActors);
	if (FoundActors.Num() > 0)
	{
		APlayLoginActor* MyPlayLoginActor = Cast<APlayLoginActor>(FoundActors[0]);
		if (MyPlayLoginActor)
		{
			MyPlayLoginActor->UpdateLeaderboard(PlayerName, IdleAttributeSet->GetWoodcutExp(), IdleAttributeSet->GetWoodcuttingLevel());
		}
	}

	//LeaderboardManager->UpdateLeaderboard(PlayerName, IdleAttributeSet->GetWoodcutExp(), IdleAttributeSet->GetWoodcuttingLevel());
	OnExpGained.Broadcast(IdleAttributeSet->GetWoodcutExp());

	// Current level's required experience
	float expForCurrentLevel = (IdleAttributeSet->GetWoodcuttingLevel() == 1) ? 0 : IdleAttributeSet->GetMaxWoodcutExp() * pow(2, IdleAttributeSet->GetWoodcuttingLevel() - 2);

	// Next level's required experience
	float expForNextLevel = IdleAttributeSet->GetMaxWoodcutExp() * pow(2, IdleAttributeSet->GetWoodcuttingLevel() - 1);

	// Player's total experience
	float totalExp = IdleAttributeSet->GetWoodcutExp();

	// Experience relative to the current level
	float relativeExp = totalExp - expForCurrentLevel;

	// Progress as a percentage
	float progress = (relativeExp / (expForNextLevel - expForCurrentLevel)) * 100;

	// Logs for debugging
	/*
	UE_LOG(LogTemp, Warning, TEXT("expForCurrentLevel: %f"), expForCurrentLevel);
	UE_LOG(LogTemp, Warning, TEXT("expForNextLevel: %f"), expForNextLevel);
	UE_LOG(LogTemp, Warning, TEXT("totalExp: %f"), totalExp);
	UE_LOG(LogTemp, Warning, TEXT("relativeExp: %f"), relativeExp);
	UE_LOG(LogTemp, Warning, TEXT("progress: %f"), progress);
	*/
	// Broadcast the current progress
	WhenLevelUp.Broadcast(progress);

	// If the player has leveled up
	if (IdleAttributeSet->GetWoodcutExp() >= expForNextLevel)
	{
		// Increase the level
		IdleAttributeSet->SetWoodcuttingLevel(IdleAttributeSet->GetWoodcuttingLevel() + 1.f);

		// Recalculate relativeExp and progress
		expForCurrentLevel = expForNextLevel;
		expForNextLevel = IdleAttributeSet->GetMaxWoodcutExp() * pow(2, IdleAttributeSet->GetWoodcuttingLevel() - 1);
		relativeExp = totalExp - expForCurrentLevel;
		progress = (relativeExp / (expForNextLevel - expForCurrentLevel)) * 100;

		// Broadcast level up delegate
		WhenLevelUp.Broadcast(progress);
	}
}

void AIdlePlayerState::UpdateXPThreshold()
{
	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	IdleAttributeSet->SetXPToNextLevel(IdleAttributeSet->GetXPToNextLevel() + 100);
}

void AIdlePlayerState::AutoSaveGame()
{
	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	int32 CurrentWoodcuttingExp = IdleAttributeSet->GetWoodcutExp();
	int32 CurrentPlayerLevel = IdleAttributeSet->GetWoodcuttingLevel();
	UIdleSaveGame::SaveGame(CurrentWoodcuttingExp, CurrentPlayerLevel);
}

void AIdlePlayerState::LoadExp()
{
	//Load values from playfab
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LoginActorTag"), FoundActors);
		if (FoundActors.Num() > 0)
		{
			APlayLoginActor* MyPlayLoginActor = Cast<APlayLoginActor>(FoundActors[0]);
			if (MyPlayLoginActor)
			{
				// Ensure we only bind once
				if (!bHasBoundToPlayFabDelegate)
				{
					MyPlayLoginActor->OnExpLoaded.AddDynamic(this, &AIdlePlayerState::OnPlayFabExpLoaded);
					MyPlayLoginActor->OnPlayerLevelLoaded.AddDynamic(this, &AIdlePlayerState::OnPlayFabPlayerLevelLoaded); // Bind to the Player Level delegate
					bHasBoundToPlayFabDelegate = true;
				}

				// Now, request PlayFab to load the EXP and Player Level
				MyPlayLoginActor->LoadWoodcuttingExpFromPlayFab();
			}
		}
	
}

void AIdlePlayerState::OnPlayFabExpLoaded(int32 LoadedExp)
{
	int32 FinalExp = FMath::Max(LocalWoodcuttingExp, LoadedExp);

	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	IdleAttributeSet->SetWoodcutExp(LoadedExp);
}

void AIdlePlayerState::OnPlayFabPlayerLevelLoaded(int32 LoadedPlayerLevel)
{
	int32 FinalLevel = FMath::Max(LocalPlayerLevel, LoadedPlayerLevel);

	UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(AttributeSet);
	IdleAttributeSet->SetWoodcuttingLevel(LoadedPlayerLevel);
}


