// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interfaces/TakeDamage.h"
#include "AI_Character.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakeDamage,float,Damage,const FVector&,ImpactPoint);

UCLASS()
class PROJECTZ_API AAI_Character : public ACharacter, public ITakeDamage
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAI_Character();

	UPROPERTY(BlueprintAssignable)
	FOnTakeDamage EventTakeDamage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	AFPS_Character* PlayerRef;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IsDead")
	bool bIsDead;
	
	/** Distance of which the sphere trace will go */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float SphereDistanceMultiplier=50.0f;
	
	/** Sphere radius of damaging sphere trace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float SphereRadius=25.0f;

	/** Damage the Ai applies on attack */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float AttackDamage=35.0f;

	//Calculates the taken damage
	UFUNCTION()
	float SetDamage(float Damage,float CriticalHitChance, float CriticalHitModifier,const FHitResult& HitResult) const;

	//Calculates the amount of Damage applied based on distance
	UFUNCTION()
	float SetRadialDamage(float Damage, float Radius, const FVector& ExplosiveLocation = FVector::ZeroVector) const;

	//Updates health and returns true if character is dead
	UFUNCTION()
	bool UpdateHealth(float Damage);

	//Gets called when damage is applied
	UFUNCTION()
	virtual void TakeDamage(const FAmmoData& AmmoData, float CriticalHitModifier,const FHitResult& HitResult) override;

	//Gets called when radial damage is applied
	UFUNCTION()
	virtual void TakeRadialDamage(const FAmmoData& AmmoData, float CriticalHitModifier,const FHitResult& HitResult, const FVector& ExplosiveLocation = FVector::ZeroVector) override;

	//Gets called when melee damage is applied
	UFUNCTION(BlueprintCallable)
	virtual void TakeMeleeDamage(float Damage,const FHitResult& HitResult) override;

	//Spawns weapon to the hands of the character
	void SpawnWeapon();

	//Does a sphere trace and applies damage to hit target
	UFUNCTION()
	void AddMeleeDamage();

	//Handles zombie game mode specific actions
	void HandleGameMode();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	AWeaponBase* CurrentWeapon;

	//The weapon class to spawn
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeaponBase> WeaponClass;

	//Handles character death
	void Die();

	//Melee attack animation montage
	UPROPERTY(EditDefaultsOnly,Category="Animation")
	UAnimMontage* AttackMontage;

	//TimerHandle that handles a timer that calls AddMeleeDamage
	FTimerHandle AddMeleeDamageTimer;

	//A reference to the current game mode
	UPROPERTY()
	AGameModeBase* GameModeRef;

	//Patrol points for the character to go to
	UPROPERTY(EditAnywhere,Category="Patrolling")
	TArray<class ATargetPoint*> PatrolPoints;

	//Index of the current patrol point
	UPROPERTY(VisibleAnywhere,Category="Patrolling")
	int32 CurrentPatrolPoint=1;
	
public:
	
	int32 GetCurrentPatrolPoint() const {return CurrentPatrolPoint;}

	void SetCurrentPatrolPoint(int32 Value) {CurrentPatrolPoint = Value;}

	bool IsDead() const {return bIsDead;}

	const TArray<ATargetPoint*>& GetPatrolPoints() const{return PatrolPoints;}

	//Starts firing the weapon
	UFUNCTION()
	void StartFire();

	//Stops firing the weapon
	UFUNCTION()
	void StopFire();

	//Does a melee attack
	UFUNCTION()
	void Melee();

	//True if the character is aiming (Aiming down sight)
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Animation")
	bool bIsAiming=false;
	
};
