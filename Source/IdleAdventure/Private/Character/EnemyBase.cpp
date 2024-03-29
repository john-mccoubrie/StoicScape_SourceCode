


#include "Character/EnemyBase.h"
#include <Combat/CombatManager.h>
#include "Character/IdleCharacter.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "GameFramework/CharacterMovementComponent.h" 
#include <NiagaraFunctionLibrary.h>

// Sets default values
AEnemyBase::AEnemyBase()
{
    //GetCharacterMovement()->bOrientRotationToMovement = true;
    //GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
    //GetCharacterMovement()->bConstrainToPlane = true;
    //GetCharacterMovement()->bSnapToPlaneAtStart = true;

    //bUseControllerRotationPitch = false;
    //bUseControllerRotationRoll = false;
    //bUseControllerRotationYaw = false;

	//Set the default AI controller class
	NPCAIControllerClass = ANPCAIController::StaticClass();
}

void AEnemyBase::Interact()
{
	ACombatManager* CombatManager = ACombatManager::GetInstance(GetWorld());
	AIdleCharacter* PlayerCharacter = Cast<AIdleCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	CombatManager->HandleCombat(PlayerCharacter->CombatComponent, this->CombatComponent, 0);
}

void AEnemyBase::SpawnEnemyAttackEffect()
{
    // Play attack montage
    UAnimMontage* AnimMontage = EnemyAttackMontage;
    PlayAnimMontage(AnimMontage);

    //UE_LOG(LogTemp, Warning, TEXT("SpawnCombatEffect in Enemy_Goblin"));

    // Get player character
    AIdleCharacter* PlayerCharacter = Cast<AIdleCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerCharacter)
    {
        return; // Exit if player character is not found
    }

    // Get enemy location
    FVector EnemyLocation = GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();

    // Calculate rotation towards player
    FRotator RotationTowardsPlayer = UKismetMathLibrary::FindLookAtRotation(EnemyLocation, PlayerLocation);

    // Preserve the current Pitch and Roll of the enemy
    RotationTowardsPlayer.Pitch = GetActorRotation().Pitch;
    RotationTowardsPlayer.Roll = GetActorRotation().Roll;

    // Set enemy rotation to face the player
    SetActorRotation(RotationTowardsPlayer);

    // Spawn effect on the Player
    SpawnedEnemyAttackEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnemyAttackEffect, PlayerLocation);
}

void AEnemyBase::EndCombatEffects()
{
    if (SpawnedEnemyAttackEffect)
    {
        SpawnedEnemyAttackEffect->Deactivate();
    }  
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnedEnemyAttackEffect is null in Enemy base"));
    }
}

void AEnemyBase::EnemyDeathAnimation()
{
    UAnimMontage* DeathMontage = EnemyDeathMontage;
    PlayAnimMontage(DeathMontage);
}

void AEnemyBase::DemonDeathAnimation()
{
    UAnimMontage* DeathMontage = DemonDeathMontage;
    PlayAnimMontage(DemonDeathMontage);
}

void AEnemyBase::EnemyAttacksPlayer()
{
    //UE_LOG(LogTemp, Warning, TEXT("Enemy attacks player in EnemyBase"));
    ACombatManager* CombatManager = ACombatManager::GetInstance(GetWorld());
    AIdleCharacter* PlayerCharacter = Cast<AIdleCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    CombatManager->HandleCombat(this->CombatComponent, PlayerCharacter->CombatComponent, 0);
}

void AEnemyBase::UpdateWalkSpeed(float NewSpeed)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

