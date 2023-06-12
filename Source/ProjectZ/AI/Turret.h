// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Interfaces/Hackable.h"
#include "../Weapons/WeaponBase.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

UCLASS()
class PROJECTZ_API ATurret : public APawn , public IHackable
{
	GENERATED_BODY()

	friend class ATurretAIController;

	/*Turret body mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TurretBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow", meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* Arrow;

	UPROPERTY(EditAnywhere,Category="State")
	bool bIsFriendly=false;

	UPROPERTY(EditAnywhere,Category="State")
	float MaxAngleRot=0.1f;

	FRotator OriginalRotation;

	UPROPERTY(VisibleAnywhere,Category="State")
	AActor* FocusTarget=nullptr;

	FTimerHandle FocusHandle;

	UFUNCTION(BlueprintCallable)
	void StartFire();

	UFUNCTION(BlueprintCallable)
	void StopFire();

	UFUNCTION()
	void StartFocusTarget(AActor* Target);

	UFUNCTION()
	void Turn();

	UFUNCTION()
	void StopFocusTarget();

	void TurnToOriginalRot();

	void SpawnWeapon();

public:
	// Sets default values for this pawn's properties
	ATurret();

	virtual void FriendlyMode() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeaponBase> WeaponClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	bool IsFriendly() const {return bIsFriendly;}
};
