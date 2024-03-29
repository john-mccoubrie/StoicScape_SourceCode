

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Combat/BaseCombatComponent.h"
#include "GhoulBossCombatComponent.generated.h"

class AIdleCharacter;

UCLASS()
class IDLEADVENTURE_API UGhoulBossCombatComponent : public UBaseCombatComponent
{
	GENERATED_BODY()
public:

	virtual void BeginPlay() override;
	virtual void HandleDeath() override;
	virtual void TakeDamage(float amount, float level) override;
	virtual void DamageCheck() override;
	void StopDamageCheckTimer();

	// Method to initiate the tailspin attack
	void InitiateSwipeAttack();

	// Method to check if player is in range for the tailspin attack
	void CheckForPlayerInRange();

	// Poison methods
	void ApplyPoisonEffectToPlayer(AIdleCharacter* PlayerCharacter);
	void DealPoisonDamage();
	void StopPoisonEffect();

	// Method to apply damage to the player during the tailspin attack
	void ApplySwipeDamage();

	// Method to stop applying tailspin damage
	void StopSwipeDamage();

	void StopSwipeAttack();

	void InitializeSwipeAttackTimer();
	virtual void StopCircleDamageCheckTimer() override;
	virtual void StartCircleDamageCheckTimer() override;

	bool bIsSwipeAttackActive;

	FTimerHandle PoisonTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* PoisonCircleEffect;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* PoisonDamageEffect;
	

private:
	//FTimerHandle DamageCheckTimer;
	//void DamageCheck();
	//float PendingDamage;

	// Timers for the tailspin attack
	FTimerHandle SwipeAttackTimer;
	FTimerHandle SwipeDamageTimer;

	// Swipe attack parameters
	float SwipeAttackIntervalMin;
	float SwipeAttackIntervalMax;
	float SwipeTimeToDodge;
	float SwipeDamageInterval;
	float SwipeRange;
	float zAdjustmentPoisonSpawn;

	//Poison attack parameters
	float PoisonDamagePerSecond;
	float PoisonDuration;

	bool bCanInitializeCircle = true;
	
};
