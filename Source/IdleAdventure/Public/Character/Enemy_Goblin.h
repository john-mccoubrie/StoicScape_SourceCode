

#pragma once

#include "CoreMinimal.h"
#include "Character/EnemyBase.h"
#include "Enemy_Goblin.generated.h"

/**
 * 
 */
UCLASS()
class IDLEADVENTURE_API AEnemy_Goblin : public AEnemyBase
{
	GENERATED_BODY()
public:
	AEnemy_Goblin();
	virtual void Interact() override;
	virtual void SpawnEnemyAttackEffect() override;
	virtual void EndCombatEffects() override;
	virtual void EnemyAttacksPlayer() override;
	virtual void EnemyDeathAnimation() override;
	virtual void UpdateWalkSpeed(float NewSpeed) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoblinWeapon")
	USkeletalMeshComponent* GoblinWeapon;

};
