// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"	
#include "CoreMinimal.h"			
#include "../Interfaces/TakeDamage.h"
#include "GameFramework/Character.h"	
#include "Perception/AISightTargetInterface.h"
#include "FPS_Character.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireWeaponDelegate, EWeaponType, WeaponType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopFireWeaponDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponSwitchDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnADSEnterDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnADSExitDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHudWeaponUpdateDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainHudUpdateDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

class UCurveFloat;
class AWeaponBase;
class AThrowingObjects;
class APortalActor;


UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	FirstSlot,
	SecondSlot
};

UENUM(BlueprintType)
enum class EJumpMode : uint8
{
	NormalJump,
	DoubleJump,
	LongJump
};

UCLASS()
class PROJECTZ_API AFPS_Character : public ACharacter , public ITakeDamage , public IAISightTargetInterface
{
	GENERATED_BODY()

	friend class UClimbingComponent;
	friend class UPortalComponent;

public:
	// Sets default values for this character's properties
	AFPS_Character();

	UPROPERTY(BlueprintAssignable)
	FOnFireWeaponDelegate CharacterFireWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnStopFireWeaponDelegate CharacterStopFireWeapon;

	UPROPERTY(BlueprintAssignable)
	FOnWeaponSwitchDelegate CharacterWeaponSwitch;

	UPROPERTY(BlueprintAssignable)
	FOnADSEnterDelegate EventADSEnter;

	UPROPERTY(BlueprintAssignable)
	FOnADSExitDelegate EventADSExit;

	UPROPERTY(BlueprintAssignable)
	FOnHudWeaponUpdateDelegate UpdateWeaponHud;

	UPROPERTY(BlueprintAssignable)
	FOnMainHudUpdateDelegate UpdateMainHud;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerDeath EventPlayerDeath;

	UFUNCTION(BlueprintCallable)
	void PickUpWeapon(TSubclassOf<AWeaponBase> WeaponToSpawn, int32 AmmoInMag, int32 ReservedAmmo);

	//Check if the player can be seen by AI characters
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const override;

	AActor* DetachGrabbed();

	UPROPERTY(Transient)
	APortalActor* GrabbedObject;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	virtual void Jump() override;
	void StartCrouch();
	void EndCrouch();
	void crouch();
	void ForwardKeyDown();
	void ForwardKeyUp();
	
	void StartSlide();
	void StopSlide();
	void ADSEnter();
	void ADSExit();

	//Checks if we can peak from behind the cover. If we can then peaks
	bool TryPeak();

	//Peak above the current cover
	void PeakAbove(float TargetHalfHeight);

	//Peak Right side of the current cover
	void PeakRight(float Offset);

	//Peak left side of the current cover
	void PeakLeft(float Offset);

	//Stop peaking
	void StopPeakAbove();
	void StopPeakCorner();

	//Hold the throwable objects in hand and aim
	void HoldThrowable();

	//Throw the throwable object
	void ThrowThrowable();

	//Attach suppressor to current weapon if possible
	void AttachSuppressor();

	//Calculates the influence of the current floor on sliding speed
	FVector CalculateFloorInfluence() const;
	
	void StartSprinting();
	void StopSprinting();

	//Checks the possibility of peaking from behind the cover
	bool CanPeakAbove(float TargetHalfHeight);
	bool CanPeakCorner(float Offset);

	//Switches current jump mode
	void SwitchJumpMode();

	//Resets reload timeline and reload state
	void ResetReloadTimeline();

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	//3rd person pawn skeletal mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;
	
	//The camera used when player dies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	ACameraActor* DeathCamera;

	//The position death camera should relocate to when player dies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* DeathCameraTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UPortalComponent* PortalComponent;
	
	//Component that handles ledge grab and climbing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UClimbingComponent* ClimbingComponent;
	
	//The class of the object to throw
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AThrowingObjects> ThrowableClass;

	//Curve used for animation of pulling down the weapon
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveFloat* PullDownCurve;

	//Curve used for animation of pulling up the weapon
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveFloat* PullUpCurve;

	//Curve used for equiping weapon animation
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UCurveFloat* EquipWeaponCurve;

	//Curve used for ADSing
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	UCurveFloat* ADSCurve;

	//Curve used for when peaking the cover
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	UCurveFloat* PeakCornerCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	UCurveFloat* RotationCorrectionCurve;
	
	FTimeline ReloadPullDownTimeLine;

	FTimeline ReloadPullUpTimeline;

	FTimeline EquipWeaponTimeLine;

	FTimeline ADSTimeline;

	FTimeline CrouchTimeline;

	FTimeline PeakCornerTimeline;

	FTimeline CorrectRotTimeline;

	//The relative location camera needs to be at when peaking the corner
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Cover")
	FVector CameraRelativePeakLoc;

	//Default camera relative location
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Cover")
	FVector CameraRelativeDefaultLoc;

	//Extra offset to add to camera when peaking cover from left and right
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Cover")
	float ExtraOffset;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ThrowSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* KnifeSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SlideSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* AttachmentSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ADSSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ZoomInSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ZoomOutSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PlayerDeathSound;

	UPROPERTY()
	UAudioComponent* SlideAudio;

	//Alpha of pulling the weapon animation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category="Animation")
	float WeaponPullAlpha;

	//Sets FOV and arms transform when ADSing
	UFUNCTION()
	void SetFOV(float value) const;

	UFUNCTION()
	void SetRotation(float value);

	//Sets alpha for weapon pull animations
	UFUNCTION()
	void SetAlpha(float value);

	//Sets alpha for crouching
	UFUNCTION()
	void SetCrouchAlpha(float value);

	//Sets alpha for peaking
	UFUNCTION()
	void SetPeakAlpha(float value);

	//Weapon pull down finished function
	UFUNCTION()
	void OnReloadPullDownFinished();

	//Weapon pull up finished function
	UFUNCTION()
	void OnReloadPullUpFinished();

	//Handles switching weapon between 2 weapon slots
	UFUNCTION()
	void WeaponSwitch();

	//When weapon equiping finishes
	UFUNCTION()
	void EquipWeaponFinished();

	//Checks if we are near a wall
	UFUNCTION()
	void CheckIsNearWall();

	//Turns on friendly mode on turrets and security cameras
	UFUNCTION(BlueprintCallable)
	void Hack();

	//Predicts and shows the path the throwable will take
	UFUNCTION()
	void PredictThrowablePath();

	void SpawnKnife();

	UPROPERTY(Transient)
	AActor* KnifeRef;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> KnifeClass;

	//Distance in which a wall in considered close
	UPROPERTY(EditAnywhere, Category = "Cover")
	float NearWallCheckRange=10;

	//Minimum distance of the player from the cover in order to be able to peak the cover
	UPROPERTY(EditAnywhere, Category = "Cover")
	float CoverCheckRange=180;

	//Height in which peak above line trace starts (this gets added with capsule half height to the real height)
	UPROPERTY(EditAnywhere, Category = "Cover")
	float CoverCheckHeight=180;

	//Maximum distance we can peak from left or right of a cover
	UPROPERTY(EditAnywhere, Category = "Cover")
	float CoverCheckRightLeft=100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bIsPeakingAbove=false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bIsPeakingCorner=false;

	//Spawns weapon to the given slot
	void SpawnToSlot(TSubclassOf<AWeaponBase> WeaponToSpawn, EWeaponSlot Slot, int32 AmmoInMag, int32 ReservedAmmo);

	//Spawns a throwable in characters hand
	void SpawnThrowable(TSubclassOf<AThrowingObjects> ThrowableToSpawn);

	UPROPERTY(EditAnywhere,Category="Jump")
	float JumpHeight=300;

	UPROPERTY(EditAnywhere,Category="Jump")
	float LongJumpHeight=800;

	UPROPERTY(EditAnywhere,Category="Jump")
	int8 JumpsInARow=0;

	UPROPERTY(EditAnywhere, Category = "ADS")
	float DefaultFOV=90.0f;

	UPROPERTY(EditAnywhere, Category = "ADS")
	float CurrentFOV=90.0f;

	UPROPERTY(EditAnywhere, Category = "ADS")
	float TargetFOV=90.0f;

	//Max weapon sway degree
	UPROPERTY(EditAnywhere, Category = "WeaponSway")
	float MaxSwayDegree=2.5f;

	//How fast the weapon sway happens
	UPROPERTY(EditAnywhere, Category = "WeaponSway")
	float SwaySpeed=2.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsReloading=false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsChangingWeapon=false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly , Category = "Weapon")
	bool bHasWeapon=false;

	UPROPERTY(VisibleAnywhere, Category = "Throwable")
	AThrowingObjects* Throwable;

	void DropWeapon() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeaponBase* CurrentWeapon;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeaponBase* WeaponSlot_01;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	AWeaponBase* WeaponSlot_02;

	//Slot of the current weapon that we are holding
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponSlot WeaponSlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bFirstSlotFull=false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bSecondSlotFull=false;

	//Current jump mode
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jump")
	EJumpMode JumpMode;

	//The actor that is in interact range
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Interact")
	AActor* InteractHitActor=nullptr;

	//Range of interacting
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Interact")
	float InteractRange=200.0f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Interact")
	float InteractSphereTraceRadius=10.0f;

private:

	
	void EquipSlot1();
	void EquipSlot2();
	void EquipWeapon();

	//Shows and hides weapons on equip and switching
	void ShowWeapon() const;

	//Interacts with and interactable
	void Interact();
	

	//Calculates the transform for arms mesh when ADSing
	FTransform CalculateAdsTransform() const;
	
	void ReloadPullUp();

	//Gets called when we take melee damage
	UFUNCTION(BlueprintCallable)
	virtual void TakeMeleeDamage(float Damage, const FHitResult& HitResult) override;

	//Gets called when we take damage
	virtual void TakeDamage(float Damage) override;

	//Called when we take damage from shooting
	virtual void TakeDamage(const FAmmoData& AmmoData, float CriticalHitModifier, const FHitResult& HitResult) override;

	//Updates player health
	void UpdateHealth(float Damage);

	//Regenerates player health
	void RegenerateHealth();

	//Handles player death
	void HandleDeath();

	//Performs a melee attack
	void Melee();
	
	void UpdateGrabbedObjectLoc();

	//Grabs a dead body
	void TryGrabBody();

	//Updates the location of the body that we are dragging
	void UpdateDragBodyLocation() const;

	//Sways current weapon when we move to right and left
	void WeaponSway(float DeltaTime) const;

	//Finds interactable objects
	void FindInteractable();

	void TrySpawnPortalA();
	void TrySpawnPortalB();

	FTimerHandle HealthRegenHandle;
	
	FTimerHandle ReloadTimerHandle;

	FTimerHandle GrabTimerHandle;

	FTimerHandle PredictPathHandle;

	FTimerHandle PeakHandle;

	bool bIsForwardKeyDown=false;

	bool bIsSprintKeyDown=false;

	//The time it takes for the camera to blend to the death camera
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DeathCameraBlendTime=1.0f;

	/** Sphere radius of damaging sphere trace */
	UPROPERTY(EditAnywhere,  Category = "Melee")
	float MeleeSphereRadius=25.0f;

	/** Melee Damage applies */
	UPROPERTY(EditAnywhere,  Category = "Melee")
	float MeleeDamage=100.0f;

	/** Distance of which the melee sphere trace will go */
	UPROPERTY(EditAnywhere,Category = "Melee")
	float MeleeRange=50.0f;

	UPROPERTY(VisibleAnywhere,Category = "Melee")
	bool bCanMelee=true;

	UPROPERTY(EditAnywhere,Category = "Melee")
	float MeleeCoolDown=1.0f;

	/** Distance of which the melee sphere trace will go */
	UPROPERTY(EditAnywhere,Category = "Grab")
	float GrabRange=50.0f;
	
	UPROPERTY(EditAnywhere,Category = "Hack")
	float HackRange=500.0f;

	/** Sphere radius of damaging sphere trace */
	UPROPERTY(EditAnywhere,  Category = "Grab")
	float GrabSphereRadius=25.0f;
	
	UPROPERTY(VisibleAnywhere,  Category = "Grab")
	bool bIsBodyGrabbed=false;

	//Rate of Regenerating health
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthRegenRate;

	//The delay after taking damage to start regenerating health
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthRegenDelay;

	//The amount of health to be added every time regenerate health is called
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthRegenAmount;
	
	UPROPERTY(EditAnywhere,Category="Health")
	float CurrentHealth=100.0f;
	
	UPROPERTY(EditAnywhere,Category="Health")
	float MaxHealth=100.0f;

	UPROPERTY(EditAnywhere,Category="Health")
	float CurrentArmor=100.0f;

	UPROPERTY(EditAnywhere,Category="Health")
	float MaxArmor=100.0f;

	UPROPERTY(VisibleAnywhere,Category="Health")
	bool bIsDead=false;

	UPROPERTY(VisibleAnywhere,Category="Crouch")
	bool bIsCrouching=false;

	UPROPERTY(EditAnywhere,Category="Hack")
	bool bIsScanning=false;

	UPROPERTY(EditAnywhere,Category="Hack")
	float HackSphereTraceRadius=10.0f;

	UPROPERTY(EditAnywhere, Category = "Crouch")
	float CrouchWalkSpeed=300.0f;

	UPROPERTY(EditAnywhere, Category = "Sprint")
	float SprintWalkSpeed=900.0f;

	UPROPERTY(EditAnywhere, Category = "Slide")
	float MaxSlideSpeed=1200.0f;

	UPROPERTY(VisibleAnywhere,Category="Slide")
	bool bIsSliding=false;

	//Force added to character when sliding gets multiplied by this
	UPROPERTY(EditAnywhere, Category = "Slide")
	float SteepnessForceMultiplier=500000.0f;

	UPROPERTY(VisibleAnywhere,Category="Slide")
	bool bCanSlide=true;

	UPROPERTY(EditAnywhere, Category = "Crouch")
	float CrouchHalfHeight=44.0f;
	
	UPROPERTY(EditAnywhere, Category = "Crouch")
	float StandingHalfHeight=88.0f;

	//Default 1st person mesh transform
	UPROPERTY(VisibleAnywhere,Category="ADS")
	FTransform DefaultArmsTransform;

	//Player points (used to buy stuff)
	UPROPERTY(EditAnywhere, Category = "Points")
	int32 Points;
	
public:

	UPROPERTY(VisibleAnywhere,  Category = "Grab")
	bool bIsObjectGrabbed=false;

	UPROPERTY(VisibleAnywhere,  Category = "Grab")
	bool bGrabThroughPortal=false;

	UPROPERTY(EditAnywhere,Category = "Grab")
	float GrabbedObjectDistance=100.0f;

	UPROPERTY(EditAnywhere,Category = "Grab")
	float GrabbedObjectHeight=50.0f;

	void CorrectRotation();

	void LockTeleport();

	void GrabPortalActor(APortalActor* PortalActor);

	UPROPERTY(VisibleAnywhere, Category = "Teleporting")
	bool bCanTeleport=true;

	//If this player is fake (used in portals)
	UPROPERTY(VisibleAnywhere, Category = "Teleporting")
	bool bIsFake=false;
	
	UFUNCTION(BlueprintCallable)
	int GetPoints() const{return Points;}
	
	UFUNCTION(BlueprintCallable)
	void SetPoints(const int Value) {Points = Value;}
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	//Physics handle component handles interaction with objects simulating physics (Dragging dead bodies)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	class UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bCanFire = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ADS")
	bool bIsADSing = false;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Property")
	bool bIsNearWall = false;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Sprint")
	bool bIsSprinting=false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float WalkSpeed=500.0f;

	//Fires weapon
	UFUNCTION(BlueprintCallable)
	void OnFire();

	//Stops firing weapon
	UFUNCTION(BlueprintCallable)
	void StopFire();

	//Reload weapon
	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const {return CurrentHealth;}

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const {return MaxHealth;}

	UFUNCTION(BlueprintCallable)
	float GetCurrentArmor() const {return CurrentArmor;}

	void SetCurrentArmor(const float Value) {CurrentArmor = Value;}

	UFUNCTION(BlueprintCallable)
	float GetMaxArmor() const {return MaxArmor;}

	UFUNCTION(BlueprintCallable)
	bool IsDead() const {return bIsDead;}

	float GetCrouchHalfHeight() const {return CrouchHalfHeight;}

	AWeaponBase* GetCurrentWeapon() const {return CurrentWeapon;}

	USkeletalMeshComponent* GetMesh1P() const {return Mesh1P;}

	UPortalComponent* GetPortalComponent() const {return PortalComponent;}

	USkeletalMeshComponent* GetMesh3P() const{return Mesh3P;}
};
