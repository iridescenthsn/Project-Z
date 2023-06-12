// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"				
#include "Components/TimelineComponent.h"		
#include "GameFramework/Actor.h"	
#include "WeaponBase.generated.h"


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol,
	AssaultRifle,
	ShotGun,
	SniperRifle,
	GrenadeLauncher,
	RocketLauncher
};

USTRUCT(BlueprintType)
struct FMagStatus
{
	GENERATED_USTRUCT_BODY()
	
	bool bHasAmmo;
	bool bMagFull;
};

USTRUCT(BlueprintType)
struct FAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float CriticalHitChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float DamageRadius;
};


class AImpactEffect;
class UCurveFloat;
class APistolAmmoShell;

UCLASS()
class PROJECTZ_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	FTimerHandle AutoFireHandle;

	//Player fire method
	virtual void WeaponFire();

	//Used when an Ai is using the weapon
	virtual void AiWeaponFire();

	//Player stop fire method
	UFUNCTION()
	virtual void StopFire();

	//AI stop fire method
	UFUNCTION()
	virtual void AiStopFire();
	
	UFUNCTION()
	void Reload();
	
	bool HasReservedAmmo() const;

	//The amount of player input during shooting(used for reversing recoil calculation)
	float PlayerPitchInput=0.0f;
	float PlayerYawInput = 0.0f;

	FMagStatus MagStatus() const;

	UPROPERTY(EditAnywhere, Category = "PickUp")
	TSubclassOf <class AWeaponPickUp> PickUpClass;

	void SpawnPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Calculates bullet spread and line traces with it
	bool CalculateShot(TArray<FHitResult>& InGoingShots, TArray<FHitResult>& OutGoingShots, FVector& TraceEnd) const;

	//Calculate shot when AI is using the weapon
	FHitResult AiCalculateShot() const;

	//Apply damage to actors which can take Damage
	void AddDamage(const FHitResult &Hit, const FName& ShooterTag, float Damage) const;

	//Ejects ammo shell after firing
	void AmmoShellEject() const;

	//Ammo shell to Spawn
	UPROPERTY(EditDefaultsOnly, Category = "Ammoshell")
	TSubclassOf <APistolAmmoShell> AmmoShellClass;

	//Player ref
	UPROPERTY(Transient)
	class AFPS_Character* Player;

	//Player Camera for line tracing
	UPROPERTY(Transient)
	class UCameraComponent* Camera;
	
	//Gun mesh 1st person view
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Suppressor")
	USkeletalMeshComponent* GunMesh;

	//Suppressor mesh
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Suppressor")
	UStaticMeshComponent* SuppressorMesh;
	
	//The scene root of gun
	UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
	USceneComponent* SceneRoot;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf <AImpactEffect> ImpactEffectBP;

	//Hit scan weapon range
	UPROPERTY(EditDefaultsOnly, Category = "LineTrace")
	float LineTraceRange=10000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 CurrentAmmoInMag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 MaxAmmoInMag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 CurrentReservedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 MaxReservedAmmo;

	//Properties of the ammo of the gun such as damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	FAmmoData AmmoData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	float PenetrationDamageMultiplier=0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(Transient)
	APlayerCameraManager* CameraManager;
	
	FTimerHandle StopFiringHandle;

	FTimerHandle RecoilResetHandle;
	
	FTimerHandle RecoilReverseHandle;

	FTimerHandle RecoilTimelineTicker;

	FTimerHandle ShootingDelayHandle;

	//Intensity of recoil animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float AnimRecoil;

	UPROPERTY(EditAnywhere,Category="Recoil")
	float RecoilMultiplier=1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	UCurveFloat* RecoilPitchCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	UCurveFloat* RecoilYawCurve;
	
	FTimeline RecoilTimeLine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float CriticalHitModifier=1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Property")
	float ReloadTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bIsWeaponAuto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float FireRate = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float ADSFov=85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float ADSWalkSpeed=400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float HipFireBulletSpread=300;

	UPROPERTY(VisibleAnywhere,Category="Property")
	float BulletSpread;

	//Max thickness of object that can penetrate
	UPROPERTY(EditAnywhere,Category="Property")
	float MaxPenetrate=100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float ADSBulletSpread=0;

	//UI icon of the weapon
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Property")
	UMaterialInstance* UI_Icon;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimationAsset* SuppressedFireAnimation;

	UPROPERTY(VisibleAnywhere)
	bool bIsWeaponFiring=false;

	UPROPERTY(VisibleAnywhere)
	bool bIsReadyToFire=true;

	UPROPERTY(EditAnywhere, Category = "Property")
	bool bIsWeaponSuppressed=false;

	//Shoots based on the weapon type
	virtual void Shoot();

	//Shoot function when Ai is using the weapon
	virtual void AiShoot();

	virtual void AiAutoFire();

	void SetWeaponState();

	//Stops the gun pull up animation
	void CharacterStopFireWeapon();

	//Spawns bullet decals and impact effects
	void SpawnImpactEffect(const FHitResult &HitResult) const;

	UFUNCTION()
	void AddRecoil();

	UFUNCTION()
	virtual void AddRecoilPitch(float Value);

	UFUNCTION()
	virtual void AddRecoilYaw(float Value);

	//Reverses the recoil effect(pulls the weapon down after shooting)
	UFUNCTION()
	virtual void RevertRecoil();

	//Decreases the number of shots over time
	void ResetRecoil();

	//Calculates how much we need to pull down or pull left/right the weapon to revert the recoil effect
	void CalculateReverseRecoil();

	//All added pitch before stopping fire (used to calculate recoil reversing)
	float RecoilAllAddedPitch = 0.0f;

	//All added Yaw before stopping fire(used to calculate recoil reversing)
	float RecoilAllAddedYaw = 0.0f;

	//Amount of reverse recoil/100
	float PitchPullDown = 0.0f;
	float YawPullDown = 0.0f;

	//number of shot the gun is currently at(for example 12th shot in a row in an auto gun)
	int32 NumberOfShot=0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float ResetRecoilRate=0.2;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Recoil")
	TArray<float> PerBulletRecoilPitch;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Recoil")
	TArray<float> PerBulletRecoilYaw;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Recoil")
	float RecoilReverseSpeed=0.001f;
	
	//Name of the socket the gun will attach to
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Socket")
	FName SocketName;

	//The weapon type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponType")
	EWeaponType WeaponType;

public:	
	
	/*Geters and setters*/
	
	USkeletalMeshComponent* GetGunMesh() const { return GunMesh; }

	UStaticMeshComponent* GetAttachmentMesh() const {return SuppressorMesh;}

	float GetReloadTime() const {return ReloadTime;}

	bool IsWeaponAuto() const {return bIsWeaponAuto;}

	float GetAdsFov() const {return ADSFov;}
	
	float GetAdsWalkSpeed() const {return ADSWalkSpeed;}
	
	float GetHipFireBulletSpread() const {return HipFireBulletSpread;}

	float GetBulletSpread() const {return BulletSpread;}

	void SetBulletSpread(const float Spread) {BulletSpread = Spread;}

	float GetAdsBulletSpread() const {return ADSBulletSpread;}

	void SetRecoilAllAddedPitch(const float AllAddedPitch) {RecoilAllAddedPitch = AllAddedPitch;}

	void SetRecoilAllAddedYaw(const float AllAddedYaw) {RecoilAllAddedYaw = AllAddedYaw;}

	bool GetIsWeaponFiring() const {return bIsWeaponFiring;}

	void SetIsWeaponFiring(const bool IsWeaponFiring) {this->bIsWeaponFiring = IsWeaponFiring;}

	bool GetIsReadyToFire() const{ return bIsReadyToFire;}

	void SetIsReadyToFire(const bool IsReadyToFire) {this->bIsReadyToFire = IsReadyToFire;}

	bool GetIsWeaponSuppressed() const {return bIsWeaponSuppressed;}

	void SetIsWeaponSuppressed(bool value) {bIsWeaponSuppressed = value;}

	int32 GetCurrentAmmoInMag() const {return CurrentAmmoInMag;}

	void SetCurrentAmmoInMag(int32 Value) {CurrentAmmoInMag = Value;}

	int32 GetCurrentReservedAmmo() const {return CurrentReservedAmmo;}

	void SetCurrentReservedAmmo(int32 Value) {CurrentReservedAmmo = Value;}

	int32 GetMaxAmmoInMag() const {return MaxAmmoInMag;}

	int32 GetMaxReservedAmmo() const {return MaxReservedAmmo;}
	
	const FName& GetSocketName() const {return SocketName;}

	EWeaponType GetWeaponType() const {return WeaponType;}

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Suppressor")
	bool CanBeSuppressed=false;

	UPROPERTY(EditAnywhere,Category="Sound")
	USoundBase* EquipSound;

	UPROPERTY(EditAnywhere,Category="Sound")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere,Category="Sound")
	USoundBase* SpawnSound;
};
