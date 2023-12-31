
#pragma once

#include "CoreMinimal.h"
#include "BaseCombatComponent.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Health, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCharacterDeath, FString, Time, FString, Difficulty, FString, CauseOfDeath, FString, Tip);

UCLASS()
class IDLEADVENTURE_API ACombatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACombatManager();
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	static ACombatManager* GetInstance(UWorld* World);
	void ResetInstance();

	UFUNCTION()
	void HandleCombat(UBaseCombatComponent* attacker, UBaseCombatComponent* defender, int32 DamageMultiplier);

	void HandleMultiTargetCombat(UBaseCombatComponent* Attacker, const TArray<UBaseCombatComponent*>& Defenders);

	

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterDeath OnCharacterDeath;

	//UPROPERTY(BlueprintAssignable, Category = "Events")
	//FOnHealthChanged OnHealthChanged;

private:
	static ACombatManager* CombatManagerSingletonInstance;

};
