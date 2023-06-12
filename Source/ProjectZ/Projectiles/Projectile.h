// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Weapons/WeaponBase.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class AImpactEffect;
class USceneComponent;

UCLASS()
class PROJECTZ_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	FAmmoData AmmoData;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float CriticalHitModifier;

	//handles hit events like hitting enemy or non penetrable walls
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	//handles overlap events such as penetrable walls
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//handles overlap events such as penetrable walls
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void AddDamage(const FHitResult &Hit);

	UFUNCTION()
	void SpawnImpactEffect(const FHitResult& HitResult);

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf <AImpactEffect> ImpactEffectBP;

	//Max thickness of object that can penetrate
	UPROPERTY(EditAnywhere,Category="Property")
	float MaxPenetrate=100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	float PenetrationDamageMultiplier=0.7f;

	FVector HitLocation;

private:

	int16 WallsPassed=0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USphereComponent* Sphere;
	
	UStaticMeshComponent* GetMesh() const { return Mesh; }
};
