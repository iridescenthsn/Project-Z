// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ExplosiveProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AExplosiveProjectile : public AProjectile
{
	GENERATED_BODY()

protected:

	AExplosiveProjectile();

	virtual void AddDamage(const FHitResult& Hit) override;

	//Override the wall penetration on explosive projectiles cause we dont need it
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Radial force", meta = (AllowPrivateAccess = "true"))
	class URadialForceComponent* ExplosionForce;
};
