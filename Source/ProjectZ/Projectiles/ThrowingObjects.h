// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ThrowingObjects.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AThrowingObjects : public AProjectile
{
	GENERATED_BODY()

	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void AddDamage(const FHitResult& Hit) override;

	AThrowingObjects();

public:
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Projectile")
	float VelocityMultiplier=500.0f;
};
