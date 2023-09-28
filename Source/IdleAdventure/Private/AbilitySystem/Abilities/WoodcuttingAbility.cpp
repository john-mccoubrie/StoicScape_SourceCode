
#include "AbilitySystem/Abilities/WoodcuttingAbility.h"
#include "Character/IdleCharacter.h"
#include "Player/IdlePlayerController.h"
#include "AbilitySystem/Abilities/WoodcuttingAbility.h"
#include "Actor/IdleEffectActor.h"
#include <Player/IdlePlayerState.h>
#include <AbilitySystemBlueprintLibrary.h>
#include <Kismet/GameplayStatics.h>
#include <AbilitySystem/IdleAttributeSet.h>

int32 UWoodcuttingAbility::InstanceCounter = 0;

//still a lot of instances will need to fix this at some point
UWoodcuttingAbility::UWoodcuttingAbility()
{
    
    InstanceCounter++;
    //UE_LOG(LogTemp, Warning, TEXT("UWoodcuttingAbility instances: %d"), InstanceCounter);
}
UWoodcuttingAbility::~UWoodcuttingAbility()
{
    // Destructor logic
    InstanceCounter--;
    //UE_LOG(LogTemp, Warning, TEXT("UWoodcuttingAbility instances: %d"), InstanceCounter);
}

void UWoodcuttingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    //UE_LOG(LogTemp, Warning, TEXT("Activate ability wca"));
    OnTreeLifespanChanged.AddDynamic(this, &UWoodcuttingAbility::SetDuration);
    bIsTreeBeingChopped = false;
    if (bAbilityIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("WoodcuttingAbility is already active."));
        return;
    }
    bAbilityIsActive = true;
    AIdleCharacter* Character = Cast<AIdleCharacter>(ActorInfo->AvatarActor.Get());
    AIdlePlayerController* PC = Cast<AIdlePlayerController>(GetWorld()->GetFirstPlayerController());
    AIdlePlayerState* PS = PC->GetPlayerState<AIdlePlayerState>();
    //AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PS);

    //PC->OnPeriodFired.AddDynamic(this, &UWoodcuttingAbility::AddEssenceToInventory);
    UAnimMontage* AnimMontage = Character->WoodcutMontage;
    Character->PlayAnimMontage(AnimMontage);

    //Spawn particle effect from the character
    PC->SpawnTreeCutEffect();


    //FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
    //const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(PC->WoodcuttingGameplayEffect, 1.f, EffectContextHandle);
    //ActiveEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
    PS->AbilitySystemComponent->OnPeriodicGameplayEffectExecuteDelegateOnSelf.RemoveAll(this);
    PS->AbilitySystemComponent->OnPeriodicGameplayEffectExecuteDelegateOnSelf.AddUObject(this, &UWoodcuttingAbility::CalculateLogYield);
    //PS->AbilitySystemComponent->OnPeriodicGameplayEffectExecuteDelegateOnTarget

    //int32 delegateCount = AbilitySystemComponent->OnPeriodicGameplayEffectExecuteDelegateOnTarget.GetAllocatedSize();
    //UE_LOG(LogTemp, Warning, TEXT("Number of Delegate Bindings: %d"), delegateCount);

    bIsTreeBeingChopped = true;

    PC->CurrentWoodcuttingAbilityInstance = this;
}

void UWoodcuttingAbility::OnTreeCutDown()
{
    //UE_LOG(LogTemp, Warning, TEXT("OnTreeCutDown"));
    AIdleCharacter* Character = Cast<AIdleCharacter>(GetAvatarActorFromActorInfo());
    AIdlePlayerController* PC = Cast<AIdlePlayerController>(GetWorld()->GetFirstPlayerController());
    AIdlePlayerState* PS = PC->GetPlayerState<AIdlePlayerState>();
    PS->AbilitySystemComponent->OnPeriodicGameplayEffectExecuteDelegateOnSelf.RemoveAll(this);
    PC->bIsChoppingTree = false;
    bAbilityIsActive = false;

    //Deactivate particle effect from the character
    PC->EndTreeCutEffect();
    
    PS->AbilitySystemComponent->RemoveActiveGameplayEffect(PC->WoodcuttingEffectHandle);
    UAnimMontage* AnimMontage = Character->WoodcutMontage;
    Character->StopAnimMontage(AnimMontage);
    //EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UWoodcuttingAbility::SetDuration(float TotalDuration)
{
    //UE_LOG(LogTemp, Warning, TEXT("SetDuration"));
    GetWorld()->GetTimerManager().ClearTimer(StopWoodcuttingTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(StopWoodcuttingTimerHandle, this, &UWoodcuttingAbility::OnTreeCutDown, TotalDuration, false);
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    float TimeRemainingBefore = TimerManager.GetTimerRemaining(StopWoodcuttingTimerHandle);
    //UE_LOG(LogTemp, Warning, TEXT("Time at set: %f"), TimeRemainingBefore);
}

void UWoodcuttingAbility::AddEssenceToInventory()
{
    AIdleCharacter* Character = Cast<AIdleCharacter>(GetAvatarActorFromActorInfo());
    Character->EssenceCount++;
    //UE_LOG(LogTemp, Warning, TEXT("Essence Added to inventory"));
    UE_LOG(LogTemp, Warning, TEXT("EssenceCount: %f"), Character->EssenceCount);
    UItem* Essence = NewObject<UItem>();
    Character->CharacterInventory->AddItem(Essence); 
}

void UWoodcuttingAbility::CalculateLogYield(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecExecuted, FActiveGameplayEffectHandle ActiveHandle)
{
    //UE_LOG(LogTemp, Warning, TEXT("Periodic delegate called on target"));
    //Unbind the delegate once the tree is chopped down
    AIdlePlayerController* PC = Cast<AIdlePlayerController>(GetWorld()->GetFirstPlayerController());
    AIdlePlayerState* PS = PC->GetPlayerState<AIdlePlayerState>();
    UIdleAttributeSet* IdleAttributeSet = CastChecked<UIdleAttributeSet>(PS->AttributeSet);

    //Woodcutting algorithm
    float LevelMultiplier = IdleAttributeSet->GetWoodcuttingLevel();
    //UE_LOG(LogTemp, Warning, TEXT("LevelMultiplier: %f"), LevelMultiplier);

    // Adjust the base chance and the level influence to balance the ease of gathering logs
    float BaseChance = 10.0f;
    float LevelInfluence = 0.5f;
    float ChanceToYield = BaseChance + (LevelMultiplier * LevelInfluence);
    //UE_LOG(LogTemp, Warning, TEXT("ChanceToYield: %f"), ChanceToYield);

    // Random factor to add some unpredictability
    float RandomFactor = FMath::RandRange(-10.0f, 10.0f);
    ChanceToYield += RandomFactor;
    ChanceToYield = FMath::Clamp(ChanceToYield, 0.0f, 100.0f);  // Ensure chance stays between 0 and 100
    //UE_LOG(LogTemp, Warning, TEXT("ChanceToYield after RandomFactor: %f"), ChanceToYield);

    float RandomRoll = FMath::RandRange(0, 100);
    //UE_LOG(LogTemp, Warning, TEXT("RandomRoll: %f"), RandomRoll);

    //uncomment this to return to normal algorithm
    RandomRoll = 1;
    if (RandomRoll <= ChanceToYield)
    {
        // Award the log
        //UE_LOG(LogTemp, Warning, TEXT("Get log in woodcuttingability!"));
        //AddEssenceToInventory();

        // Determine the type of log based on the rarity roll
        UItem* NewLog = NewObject<UItem>();
        float RarityRoll = FMath::RandRange(0.f, 100.f);

        if (RarityRoll <= 50.f)  // 50% chance for Water
        {
            NewLog->EssenceRarity = "Wisdom";
            //UE_LOG(LogTemp, Warning, TEXT("Wisdom"));
        }
        else if (RarityRoll <= 75.f)  // 25% chance for Earth
        {
            NewLog->EssenceRarity = "Temperance";
            //UE_LOG(LogTemp, Warning, TEXT("Temperance"));
        }
        else if (RarityRoll <= 95.f)  // 20% chance for Wind
        {
            NewLog->EssenceRarity = "Justice";
            //UE_LOG(LogTemp, Warning, TEXT("Justice"));
        }
        else  // 5% chance for Fire
        {
            NewLog->EssenceRarity = "Courage";
            //UE_LOG(LogTemp, Warning, TEXT("Courage"));
        }

        // Load the data table
        UDataTable* EssenceDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Script/Engine.DataTable'/Game/Blueprints/UI/Inventory/DT_EssenceType.DT_EssenceType'"));
        if (EssenceDataTable)
        {
            // Get the log data based on the essence rarity
            FEssenceData* EssenceData = EssenceDataTable->FindRow<FEssenceData>(NewLog->EssenceRarity, TEXT(""));
            if (EssenceData)
            {
                // Set the log properties from the data table
                NewLog->Name = EssenceData->Name;
                NewLog->Icon = EssenceData->Icon;
                NewLog->ItemDescription = EssenceData->Description;
            }
        }

        AIdleCharacter* Character = Cast<AIdleCharacter>(GetAvatarActorFromActorInfo());
        Character->CharacterInventory->AddItem(NewLog);
    }
    //else
    //{
        //UE_LOG(LogTemp, Warning, TEXT("No log awarded"));
    //}
}

FActiveGameplayEffectHandle UWoodcuttingAbility::GetActiveEffectHandle() const
{
    return ActiveEffectHandle;
}

void UWoodcuttingAbility::StopCutDownTimer()
{
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (GetWorld()->GetTimerManager().IsTimerActive(StopWoodcuttingTimerHandle))
    {
        // Check the time remaining before clearing the timer
        float TimeRemainingBefore = TimerManager.GetTimerRemaining(StopWoodcuttingTimerHandle);
        //UE_LOG(LogTemp, Warning, TEXT("Time remaining before clear: %f"), TimeRemainingBefore);

        TimerManager.ClearTimer(StopWoodcuttingTimerHandle);

        // Check the time remaining after clearing the timer
        float TimeRemainingAfter = TimerManager.GetTimerRemaining(StopWoodcuttingTimerHandle);
        //UE_LOG(LogTemp, Warning, TEXT("Time remaining after clear: %f"), TimeRemainingAfter);
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("Timer is not active"));
    }

}

