// Fill out your copyright notice in the Description page of Project Settings.

#include "AI_Character.h"

#include "Solider_AI_Controller.h"
#include "../Player/FPS_Character.h"
#include "../GameModes//ZombieGameMode.h"
#include "Zombie_AI_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AISense_Damage.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ProjectZ/GameModes/ProjectZGameModeBase.h"

// Sets default values
AAI_Character::AAI_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AAI_Character::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef=Cast<AFPS_Character>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	GameModeRef =UGameplayStatics::GetGameMode(GetWorld());

	if (WeaponClass)
	{
		SpawnWeapon();
	}

	
}

float AAI_Character::SetDamage(float Damage, float CriticalHitChance, float CriticalHitModifier,const FHitResult& HitResult) const
{
	float LocalDamage = Damage;
	bool bIsCriticalHit;

	if (CriticalHitChance >= FMath::RandRange(0,100)) 
	{
		bIsCriticalHit = true; 
		LocalDamage *= CriticalHitModifier;
	}
	else
	{ 
		bIsCriticalHit = false;
	}
	
	LocalDamage = LocalDamage * HitResult.PhysMaterial->DestructibleDamageThresholdScale;
	return LocalDamage; 
}

float AAI_Character::SetRadialDamage(float Damage, float Radius, const FVector& ExplosiveLocation) const
{
	float LocalDamage = Damage;

	const float Distance = (GetActorLocation() - ExplosiveLocation).Size();

	LocalDamage = (1 - (Distance / Radius)) * LocalDamage;
	LocalDamage = FMath::Clamp(LocalDamage,0.0f, Damage);

	return LocalDamage;
}

bool AAI_Character::UpdateHealth(float Damage)
{
	bool bLocalIsDead=false;
	CurrentHealth -= Damage;
	if (CurrentHealth<=0)
	{
		bLocalIsDead = true;
		CurrentHealth = 0;
	}
	return bLocalIsDead;
}

void AAI_Character::TakeDamage(const FAmmoData& AmmoData, float CriticalHitModifier, const FHitResult& HitResult)
{
	if (!bIsDead)
	{
		UAISense_Damage::ReportDamageEvent
		    (
			GetWorld(), 
			this,
			UGameplayStatics::GetPlayerPawn(GetWorld(),0),0,HitResult.Location,
			GetActorLocation()
			); 
		
		const float DamageTaken = SetDamage(AmmoData.Damage, AmmoData.CriticalHitChance, CriticalHitModifier, HitResult);
		PlayerRef->SetPoints(PlayerRef->GetPoints()+DamageTaken);

		//If we are in zombie game mode add the damage amount to the total score
		const auto& ZombieGameMode= Cast<AZombieGameMode>(GameModeRef);
		if (ZombieGameMode)
		{
			ZombieGameMode->SetTotalScore(ZombieGameMode->GetTotalScore()+DamageTaken);
		}
		
		bIsDead = UpdateHealth(DamageTaken);

		if (bIsDead)
		{
			Die();
		}

		EventTakeDamage.Broadcast(DamageTaken,HitResult.ImpactPoint);
	}
}

void AAI_Character::TakeRadialDamage(const FAmmoData& AmmoData, float CriticalHitModifier,const FHitResult& HitResult, const FVector& ExplosiveLocation)
{
	if (!bIsDead)
	{
		UAISense_Damage::ReportDamageEvent
			(
			GetWorld(),
			this,
			UGameplayStatics::GetPlayerPawn(GetWorld(),0),0,HitResult.Location,
			GetActorLocation()
			);
		
		const float DamageTaken = SetRadialDamage(AmmoData.Damage,AmmoData.DamageRadius, ExplosiveLocation);
		PlayerRef->SetPoints(PlayerRef->GetPoints()+DamageTaken);

		//If we are in zombie game mode add the damage amount to the total score
		const auto& ZombieGameMode= Cast<AZombieGameMode>(GameModeRef);
		if (ZombieGameMode)
		{
			ZombieGameMode->SetTotalScore(ZombieGameMode->GetTotalScore()+DamageTaken);
		}
		
		bIsDead = UpdateHealth(DamageTaken);
 
		if (bIsDead)
		{
			Die();
		}

		EventTakeDamage.Broadcast(DamageTaken,HitResult.ImpactPoint);
	}
}

void AAI_Character::TakeMeleeDamage(float Damage, const FHitResult& HitResult)
{
	if (!bIsDead)
	{
		//if enemy is unaware triple the damage
		const auto State =static_cast<EAiState>(Cast<AAIController>(GetController())->GetBlackboardComponent()->GetValueAsEnum(FName("AIState")));
		if (ActorHasTag(FName("EnemyAI"))&&State!=EAiState::Chasing)
		{
			Damage*=3.0f;
		}

		bIsDead = UpdateHealth(Damage);
 
		if (bIsDead)
		{
			Die();
		}

		EventTakeDamage.Broadcast(Damage,HitResult.ImpactPoint);
	}
}


void AAI_Character::Melee()
{
	if (AttackMontage)
	{
		//Play attack animation
		PlayAnimMontage(AttackMontage);

		//Add Damage after given time
		GetWorldTimerManager().SetTimer(AddMeleeDamageTimer,this,&AAI_Character::AddMeleeDamage,1.26,false);
	}
}

void AAI_Character::StartFire() 
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AiWeaponFire();
		bIsAiming=true;
	}
}

void AAI_Character::StopFire() 
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AiStopFire();
		bIsAiming=false;
	}
}

void AAI_Character::SpawnWeapon()
{
	CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
		true), FName("GripPoint"));
}

void AAI_Character::AddMeleeDamage()
{
	//Object types to trace by
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	//Ignore self
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 1);

	//Start loc and end loc (from Ai to the distance specified)
	FVector StartLoc= GetActorLocation();
	FVector EndLoc= StartLoc + GetActorForwardVector()*SphereDistanceMultiplier;

	FHitResult Hit;
	
	//Sphere trace to apply damage if player is hit
	const bool bHit =UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),StartLoc,EndLoc,SphereRadius,ObjectTypes,false,IgnoreActors,EDrawDebugTrace::None,Hit,true);

	//Apply damage if the actor has a take damage interface
	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			//Dont do damage if the hit actor is another AI
			if (HitActor->ActorHasTag(Tags[0]))
			{
				return;
			}
			
			ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(HitActor);
			if (TakeDamageInterface)
			{
				TakeDamageInterface->TakeMeleeDamage(AttackDamage,Hit);
			}
		}
	}
}

void AAI_Character::HandleGameMode()
{
	//if we are in zombie game mode do these when dying
	const auto ZombieGameMode= Cast<AZombieGameMode>(GameModeRef);
	if(ZombieGameMode)
	{
		//Reduce the number of alive zombies
		ZombieGameMode->SetZombiesAlive(ZombieGameMode->GetZombiesAlive()-1);
		ZombieGameMode->UpdateRoundUI.Broadcast(ZombieGameMode->GetCurrentRound(),ZombieGameMode->GetZombiesAlive());
		ZombieGameMode->IsRoundOver();
		ZombieGameMode->SetZombiesKilled(ZombieGameMode->GetZombiesKilled()+1);
		ZombieGameMode->UpdateAiPerception();

		//Destroy the zombie character 3 seconds after death
		FTimerHandle DestroyHandle;
		GetWorldTimerManager().SetTimer(DestroyHandle,[&]
		{
			Destroy();
		},3,false);
	}
	else
	{
		const auto GameMode = Cast<AProjectZGameModeBase>(GameModeRef);
		if (GameMode)
		{
			GameMode->UpdateAiPerception();
			GameMode->EnemyDied();
		}
	}
}

void AAI_Character::Die() 
{
	//Simulate RagDoll physics
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	HandleGameMode();

	//Stop fire after death
	if (CurrentWeapon)
	{
		StopFire();
		CurrentWeapon->SpawnPickUp();
	}
	
	//Dont call melee function after death
	GetWorldTimerManager().ClearTimer(AddMeleeDamageTimer);
	
	if (GetController())
	{
		Cast<AAIController>(GetController())->Destroy();
	}
}


