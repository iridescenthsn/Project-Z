// Fill out your copyright notice in the Description page of Project Settings.

#include "FPS_Character.h"
#include "../AI/AI_Character.h"
#include "ClimbingComponent.h"
#include "Blueprint/UserWidget.h"
#include "../Weapons/WeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "../Interfaces/Hackable.h"
#include "../Interactable/Interactables.h"
#include "Camera/CameraActor.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "../Projectiles/ThrowingObjects.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectZ/CollisionChannels.h"
#include "ProjectZ/Portal/Portal.h"
#include "ProjectZ/Portal/PortalActor.h"
#include "ProjectZ/Portal/PortalComponent.h"


// Sets default values
AFPS_Character::AFPS_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	//Create 3rd person mesh (used for death ragdoll)
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOnlyOwnerSee(false);
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
	Mesh3P->SetHiddenInGame(true);
	Mesh3P->SetSimulatePhysics(false);

	ClimbingComponent=CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));

	PortalComponent=CreateDefaultSubobject<UPortalComponent>(TEXT("PortalComponent"));

	DeathCameraTransform= CreateDefaultSubobject<UArrowComponent>(TEXT("Death Camera Transform"));
	DeathCameraTransform->SetupAttachment(GetCapsuleComponent());

	PhysicsHandle= CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	
	GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
}

// Called when the game starts or when spawned
void AFPS_Character::BeginPlay()
{
	Super::BeginPlay();

	//SpawnKnife();

	DefaultArmsTransform = Mesh1P->GetRelativeTransform();

	if (ADSCurve)
	{
		FOnTimelineFloat ADSTimelineFloat;
		ADSTimelineFloat.BindUFunction(this, FName(TEXT("SetFOV")));
		ADSTimeline.AddInterpFloat(ADSCurve, ADSTimelineFloat);

		FOnTimelineFloat CrouchTimelineFloat;
		CrouchTimelineFloat.BindUFunction(this,FName(TEXT("SetCrouchAlpha")));
		CrouchTimeline.AddInterpFloat(ADSCurve,CrouchTimelineFloat);
	}

	if (RotationCorrectionCurve)
	{
		FOnTimelineFloat CorrectRotFloat;
		CorrectRotFloat.BindUFunction(this, FName(TEXT("SetRotation")));
		CorrectRotTimeline.AddInterpFloat(ADSCurve, CorrectRotFloat);
	}

	if (EquipWeaponCurve)
	{
		FOnTimelineEvent TimeLineFinished;
		TimeLineFinished.BindUFunction(this, FName("EquipWeaponFinished"));
		EquipWeaponTimeLine.SetTimelineFinishedFunc(TimeLineFinished);

		FOnTimelineEvent WeaponSwitchTime;
		WeaponSwitchTime.BindUFunction(this, FName("WeaponSwitch"));
		EquipWeaponTimeLine.AddEvent(0.25, WeaponSwitchTime);

		FOnTimelineFloat TimeLineProgress;
		TimeLineProgress.BindUFunction(this, FName("SetAlpha"));
		EquipWeaponTimeLine.AddInterpFloat(EquipWeaponCurve, TimeLineProgress);
		EquipWeaponTimeLine.SetLooping(false);
	}
	
	if (PullDownCurve)
	{
		FOnTimelineEvent PullDownFinished;
		PullDownFinished.BindUFunction(this, FName("OnReloadPullDownFinished"));
		ReloadPullDownTimeLine.SetTimelineFinishedFunc(PullDownFinished);
		
		FOnTimelineFloat PullDownProgress;
		PullDownProgress.BindUFunction(this, FName("SetAlpha")); 
		ReloadPullDownTimeLine.AddInterpFloat(PullDownCurve, PullDownProgress);
		ReloadPullDownTimeLine.SetLooping(false);
	}
	
	if (PullUpCurve)
	{
		FOnTimelineEvent PullUpFinished;
		PullUpFinished.BindUFunction(this, FName("OnReloadPullUpFinished"));
		ReloadPullUpTimeline.SetTimelineFinishedFunc(PullUpFinished);
	 
		FOnTimelineFloat PullUpProgress;
		PullUpProgress.BindUFunction(this, FName("SetAlpha"));
		ReloadPullUpTimeline.AddInterpFloat(PullUpCurve, PullUpProgress);
		ReloadPullUpTimeline.SetLooping(false);
	}

	if (PeakCornerCurve)
	{
		FOnTimelineFloat PeakProgress;
		PeakProgress.BindUFunction(this,FName("SetPeakAlpha"));
		PeakCornerTimeline.AddInterpFloat(PeakCornerCurve,PeakProgress);
	}

	
	UGameplayStatics::GetPlayerController(GetWorld(),0)->SetViewTarget(this);

	CameraRelativeDefaultLoc=FirstPersonCameraComponent->GetRelativeLocation();

	UpdateWeaponHud.Broadcast();
}

void AFPS_Character::CorrectRotation()
{
	CorrectRotTimeline.PlayFromStart();
}

void AFPS_Character::LockTeleport()
{
	Mesh3P->SetHiddenInGame(true);
	
	bCanTeleport=false;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle,[&]
	{
		Mesh3P->SetHiddenInGame(false);
		bCanTeleport=true;
	},GetWorld()->GetDeltaSeconds() * 3,false);
}

// Called every frame
void AFPS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ReloadPullDownTimeLine.TickTimeline(DeltaTime);
	ReloadPullUpTimeline.TickTimeline(DeltaTime);
	EquipWeaponTimeLine.TickTimeline(DeltaTime);
	ADSTimeline.TickTimeline(DeltaTime);
	CrouchTimeline.TickTimeline(DeltaTime);
	PeakCornerTimeline.TickTimeline(DeltaTime);
	CorrectRotTimeline.TickTimeline(DeltaTime);

	if (GetMovementComponent()->IsFalling()&&ClimbingComponent->CanClimb())
	{
		ClimbingComponent->Climb();
	}
	
	if (bIsSliding)
	{
		//if we are sliding add force based on how steep the floor is
		GetCharacterMovement()->AddForce(CalculateFloorInfluence());
		
		if (GetCharacterMovement()->Velocity.Size()<CrouchWalkSpeed)
		{
			//if our velocity gets lower than crouch speed while sliding exit sliding
			StopSlide();
		}
	}

	if (bIsObjectGrabbed)
		UpdateGrabbedObjectLoc();

	if (bIsBodyGrabbed)
		UpdateDragBodyLocation();
		
	WeaponSway(DeltaTime);
	CheckIsNearWall();
	FindInteractable();
	GetMovementComponent()->Velocity.Z= FMath::Clamp(GetMovementComponent()->Velocity.Z,-3000.0f,3000.0f);
}

// Called to bind functionality to input
void AFPS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn", this, &AFPS_Character::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPS_Character::LookUp);
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPS_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPS_Character::MoveRight);

	PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &AFPS_Character::Jump);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed, this, &AFPS_Character::crouch);
	PlayerInputComponent->BindAction("WeaponSlot1", IE_Pressed, this,&AFPS_Character::EquipSlot1);
	PlayerInputComponent->BindAction("WeaponSlot2", IE_Pressed, this,&AFPS_Character::EquipSlot2);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPS_Character::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPS_Character::StopFire);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AFPS_Character::HoldThrowable);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &AFPS_Character::ThrowThrowable);
	PlayerInputComponent->BindAction("Melee",IE_Pressed,this,&AFPS_Character::Melee);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPS_Character::Reload);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFPS_Character::Interact);
	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AFPS_Character::ADSEnter);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AFPS_Character::ADSExit);
	PlayerInputComponent->BindAction("SwitchJumpMode",IE_Pressed,this,&AFPS_Character::SwitchJumpMode);
	PlayerInputComponent->BindAction("Hack",IE_Pressed,this,&AFPS_Character::Hack);
	PlayerInputComponent->BindAction("AttachSuppressor",IE_Pressed,this,&AFPS_Character::AttachSuppressor);
	PlayerInputComponent->BindAction("Sprint",IE_Pressed,this,&AFPS_Character::StartSprinting);
	PlayerInputComponent->BindAction("Sprint",IE_Released,this,&AFPS_Character::StopSprinting);
	PlayerInputComponent->BindAction("ForwardKeyDown",IE_Pressed,this,&AFPS_Character::ForwardKeyDown);
	PlayerInputComponent->BindAction("ForwardKeyDown",IE_Released,this,&AFPS_Character::ForwardKeyUp);
	PlayerInputComponent->BindAction("PortalA",IE_Pressed,this,&AFPS_Character::TrySpawnPortalA);
	PlayerInputComponent->BindAction("PortalB",IE_Pressed,this,&AFPS_Character::TrySpawnPortalB);
}

//Move forward and backwards
void AFPS_Character::MoveForward(float value)
{
	//if we are sliding dont add movement input
	if ( value!=0 &&!bIsSliding)
	{
		AddMovementInput(GetActorForwardVector(), value);
	}
}

//Move right and left
void AFPS_Character::MoveRight(float value)
{
	//if we are sliding dont add movement input
	if ( value!=0)
	{
		AddMovementInput(GetActorRightVector(), value);
	}
}

void AFPS_Character::Turn(float value)
{
	if (value!=0)
	{
		AddControllerYawInput(value);
		if (CurrentWeapon)
		{
			CurrentWeapon->PlayerYawInput += value;
		}
	}
}

void AFPS_Character::LookUp(float value)
{
	if (value!=0)
	{
		AddControllerPitchInput(value);
		if (CurrentWeapon)
		{
			CurrentWeapon->PlayerPitchInput += value;
		}
	}
}

void AFPS_Character::Jump()
{
	//if we are just crouching(not sliding) then stand up (dont jump)
	if (bIsCrouching&&!bIsSliding)
	{
		EndCrouch();
		return;
	}

	//if we are sliding stop crouching and sliding then jump
	if (bIsSliding)
	{
		EndCrouch();
		StopSlide();
	}
	
	switch (JumpMode)
	{
		
	case EJumpMode::NormalJump:
		
		if (!(GetCharacterMovement()->IsFalling()))
		{
			LaunchCharacter(FVector(0, 0, JumpHeight), false, true);
		}
		break;
		
	case EJumpMode::DoubleJump:
		
		if (!GetCharacterMovement()->IsFalling())
		{
			JumpsInARow=0;
		}

		if (JumpsInARow<=1)
		{
			LaunchCharacter(FVector(0, 0, JumpHeight), false, true);
			JumpsInARow++;
		}
		break;
		
	case EJumpMode::LongJump:

		if (!(GetCharacterMovement()->IsFalling()))
		{
			LaunchCharacter(FVector(0, 0, LongJumpHeight), false, true);
		}
		break;
	}
}

void AFPS_Character::StartCrouch()
{
	bIsCrouching=true;
	CrouchTimeline.Play();
	GetCharacterMovement()->MaxWalkSpeed=CrouchWalkSpeed;
}

void AFPS_Character::EndCrouch()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 1);
 
	TArray<AActor*> OutActors;

	//check collision to see if we can stand up
	const bool CanStand=!UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(),
	GetActorLocation()+FVector(0,0,CrouchHalfHeight),
	GetCapsuleComponent()->GetScaledCapsuleRadius(),StandingHalfHeight,
	ObjectTypes,nullptr,IgnoreActors,OutActors);
		
	if (CanStand)
	{
		//if we want to stand up end sliding
		if (bIsSliding)
		{
			StopSlide();
		}
		bIsCrouching=false;
		CrouchTimeline.Reverse();
		GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
	}
}

void AFPS_Character::crouch()
{
	//if we have near the sprint speed velocity and crouch is pressed start sliding
	if (GetVelocity().Size()>SprintWalkSpeed-10.0f&&bIsSprinting&&!bIsCrouching&&!bIsSliding&&!GetCharacterMovement()->IsFalling())
	{
		StartSlide();
		return;
	}

	//if we are sprinting but cant slide dont do anything
	if (bIsSprinting)
	{
		return;
	}

	//if already crouching stand up
	if (bIsCrouching)
	{
		EndCrouch();
	}
	else
	{
		StartCrouch();
	}
}

void AFPS_Character::ForwardKeyDown()
{
	bIsForwardKeyDown=true;

	//if sprint key is already down and we press forward key start sprinting
	if (bIsSprintKeyDown)
	{
		StartSprinting();
	}
}

void AFPS_Character::ForwardKeyUp()
{
	bIsForwardKeyDown=false;

	//if we are sprinting but let go of forward key stop sprinting
	if (bIsSprinting)
	{
		StopSprinting();
	}
}

void AFPS_Character::StartSlide()
{
	if (bCanSlide)
	{
		//Put sliding on a timer so player cant spam slide
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle,[&]
		{
			bCanSlide=true;
		},1.5f,false);

		SlideAudio= UGameplayStatics::SpawnSound2D(GetWorld(),SlideSound,0.4f);
		
		bCanSlide=false;
		//Reduce friction so we can slide
		bIsSliding=true;
		StopSprinting();
		GetCharacterMovement()->GroundFriction=0.0f;
		GetCharacterMovement()->BrakingDecelerationWalking=1000.0f;

		//Boost velocity when we start sliding
		GetCharacterMovement()->Velocity=GetLastMovementInputVector().GetSafeNormal()*MaxSlideSpeed;

		//while sliding we are in crouch mode
		StartCrouch();
	}
}

void AFPS_Character::StopSlide()
{
	if (bIsSliding)
	{
		SlideAudio->SetActive(false);
		
		//Set friction back to normal
		GetCharacterMovement()->GroundFriction=8.0f;
		GetCharacterMovement()->BrakingDecelerationWalking=2048.0f;
		bIsSliding=false;
	}
}

void AFPS_Character::ADSEnter()
{
	if (!bIsADSing&&!bIsReloading&&!bIsChangingWeapon&&!bIsSprinting)
	{
		if (CurrentWeapon)
		{
			if (bIsNearWall&&!bIsPeakingAbove&&!bIsPeakingCorner)
			{
				TryPeak();
				return;
			}
			
			if (!bIsCrouching)
			{
				GetCharacterMovement()->MaxWalkSpeed=CurrentWeapon->GetAdsWalkSpeed();
			}
			
			CurrentWeapon->SetBulletSpread(CurrentWeapon->GetAdsBulletSpread());
			
			bIsADSing = true;
			ADSTimeline.Play();
			EventADSEnter.Broadcast();

			//Hide weapon mesh if the weapon is a sniper
			if (CurrentWeapon->GetWeaponType() == EWeaponType::SniperRifle)
			{
				CurrentWeapon->GetGunMesh()->SetHiddenInGame(true);
				CurrentWeapon->GetAttachmentMesh()->SetHiddenInGame(true);
				Mesh1P->SetHiddenInGame(true);
				UGameplayStatics::PlaySound2D(GetWorld(),ZoomInSound);
			}
			else
			{
				UGameplayStatics::PlaySound2D(GetWorld(),ADSSound);
			}
		}
	}
}

void AFPS_Character::ADSExit()
{
	if (bIsPeakingAbove)
	{
		StopPeakAbove();
	}
	if (bIsPeakingCorner)
	{
		StopPeakCorner();
	}
	if (bIsADSing)
	{
		if (CurrentWeapon)
		{
			if (!bIsCrouching)
			{
				GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
			}
			
			CurrentWeapon->SetBulletSpread(CurrentWeapon->GetHipFireBulletSpread());
			
			bIsADSing = false;
			ADSTimeline.Reverse();
			EventADSExit.Broadcast();

			//Unhide weapon mesh if the weapon is a sniper rifle
			if (CurrentWeapon->GetWeaponType() == EWeaponType::SniperRifle)
			{
				CurrentWeapon->GetGunMesh()->SetHiddenInGame(false);
				if (CurrentWeapon->GetIsWeaponSuppressed())
				{
					CurrentWeapon->GetAttachmentMesh()->SetHiddenInGame(false);
				}
				Mesh1P->SetHiddenInGame(false);
				UGameplayStatics::PlaySound2D(GetWorld(),ZoomOutSound);
			}
			else
			{
				UGameplayStatics::PlaySound2D(GetWorld(),ADSSound);
			}
		}
	}
}

bool AFPS_Character::TryPeak()
{
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(CurrentWeapon);

	FCollisionObjectQueryParams TraceObjectParams;
	TraceObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	TraceObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel5);

	//A point in front of the character that we use as reference for doing line traces
	const FVector StartingPoint= GetActorLocation() + (GetActorForwardVector()*CoverCheckRange);

	//Top to Down line trace to find available above peaks
	FHitResult HitTop;
	const FVector TopStartLoc = StartingPoint +FVector(0,0,CoverCheckHeight);
	const FVector TopEndLoc = StartingPoint +FVector(0,0,-StandingHalfHeight);
	GetWorld()->LineTraceSingleByObjectType(HitTop,TopStartLoc,TopEndLoc,TraceObjectParams,CollisionParams);

	//Left to right trace to find available left peaks
	FHitResult HitLeft;
	const FVector LeftStartLoc=StartingPoint + GetActorRightVector()* -CoverCheckRightLeft;
	const FVector LeftEndLoc=StartingPoint;
	GetWorld()->LineTraceSingleByObjectType(HitLeft,LeftStartLoc,LeftEndLoc,TraceObjectParams,CollisionParams);

	//Right to left trace to find available left peaks
	FHitResult HitRight;
	const FVector RightStartLoc=StartingPoint + GetActorRightVector()* CoverCheckRightLeft;
	const FVector RightEndLoc=StartingPoint;
	GetWorld()->LineTraceSingleByObjectType(HitRight,RightStartLoc,RightEndLoc,TraceObjectParams,CollisionParams);

	/*DrawDebugLine(GetWorld(), TopStartLoc, TopEndLoc, FColor::Green, false, 5);
	DrawDebugBox(GetWorld(), HitTop.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 5);

	DrawDebugLine(GetWorld(), LeftStartLoc, LeftEndLoc, FColor::Green, false, 5);
	DrawDebugBox(GetWorld(), HitLeft.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 5);
	
	DrawDebugLine(GetWorld(), RightStartLoc, RightEndLoc, FColor::Green, false, 5);
	DrawDebugBox(GetWorld(), HitRight.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 5);*/


	//if there is a valid above hit and we are crouching then Check if we can peak
	//if yes then peak
	if (HitTop.IsValidBlockingHit()&&bIsCrouching)
	{
		//find the height difference between the cover and player and add that to current capsule height to get the target half height
		const float TargetHalfHeight=GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+(StartingPoint-HitTop.ImpactPoint).Size()/2;
		if (CanPeakAbove(TargetHalfHeight))
		{
			bIsPeakingAbove=true;
			PeakAbove(TargetHalfHeight);
			return true;
		}
	}

	//How much should camera offset for peaking the corner
	float CameraRightOffset= (StartingPoint-HitRight.ImpactPoint).Size()+ExtraOffset;
	float CameraLeftOffset= (StartingPoint-HitLeft.ImpactPoint).Size()+ExtraOffset; 

	//if both right and left peaks are available then choose the one closer to player
	if (HitLeft.IsValidBlockingHit()&&HitRight.IsValidBlockingHit())
	{
		if (HitLeft.Distance>HitRight.Distance &&CanPeakCorner(-CameraLeftOffset))
		{ 
			bIsPeakingCorner=true;
			PeakLeft(CameraLeftOffset);
			return true;
		}
		else if(CanPeakCorner(CameraRightOffset))
		{
			bIsPeakingCorner=true;
			PeakRight(CameraRightOffset);
			return true;
		}
	}
	
	if (HitLeft.IsValidBlockingHit()&&CanPeakCorner(-CameraLeftOffset))
	{
		bIsPeakingCorner=true;
		PeakLeft(CameraLeftOffset);
		return true;
	}

	if (HitRight.IsValidBlockingHit()&&CanPeakCorner(CameraRightOffset))
	{
		bIsPeakingCorner=true;
		PeakRight(CameraRightOffset);
		return true;
	}

	return false;
}

void AFPS_Character::PeakAbove(float TargetHalfHeight)
{
	GetWorldTimerManager().SetTimer(PeakHandle,[&,TargetHalfHeight]
	{
		float CurrentHeight=GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+3;
		CurrentHeight= FMath::Clamp(CurrentHeight,CrouchHalfHeight,TargetHalfHeight);
		GetCapsuleComponent()->SetCapsuleHalfHeight(CurrentHeight);
		
		if (CurrentHeight>=TargetHalfHeight)
		{
			GetWorldTimerManager().ClearTimer(PeakHandle);
			GetCharacterMovement()->DisableMovement();
			ADSEnter();
		}
	},GetWorld()->GetDeltaSeconds(),true);
}

void AFPS_Character::PeakRight(float Offset)
{
	CameraRelativePeakLoc=CameraRelativeDefaultLoc + FVector(0,Offset,0);
	PeakCornerTimeline.Play();
}

void AFPS_Character::PeakLeft(float Offset)
{
	CameraRelativePeakLoc=CameraRelativeDefaultLoc + FVector(0,-Offset,0);
	PeakCornerTimeline.Play();
}

void AFPS_Character::StopPeakAbove()
{
	GetCharacterMovement()->SetDefaultMovementMode();
	GetWorldTimerManager().SetTimer(PeakHandle,[&]
	{
		if (GetCapsuleComponent()->GetScaledCapsuleHalfHeight()>CrouchHalfHeight)
		{
			float Height=GetCapsuleComponent()->GetScaledCapsuleHalfHeight()-3;
			Height= FMath::Clamp(Height,CrouchHalfHeight,StandingHalfHeight);
			GetCapsuleComponent()->SetCapsuleHalfHeight(Height);
		}
		else
		{
			bIsPeakingAbove=false;
			GetWorldTimerManager().ClearTimer(PeakHandle);
		}
	},0.01,true);
}

void AFPS_Character::StopPeakCorner()
{
	bIsPeakingCorner=false;
	PeakCornerTimeline.Reverse();
}

bool AFPS_Character::CanPeakAbove(float TargetHalfHeight)
{
	FVector Startloc;
	FVector Endloc;

	Startloc = Mesh1P->GetSocketLocation(FName(TEXT("NearWallCheck_Socket")))+FVector(0,0,TargetHalfHeight-CrouchHalfHeight);
	Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(Mesh1P->GetSocketRotation (FName(TEXT("NearWallCheck_Socket")))) * NearWallCheckRange);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CurrentWeapon);
	CollisionParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams TraceObjectParams;
	TraceObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	TraceObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel5);

	FHitResult Hit;

	bool bCanPeak = !GetWorld()->LineTraceSingleByObjectType(Hit, Startloc, Endloc,
		TraceObjectParams,CollisionParams);

	return bCanPeak;
}

bool AFPS_Character::CanPeakCorner(float Offset)
{
	FVector Startloc;
	FVector Endloc;

	Startloc = Mesh1P->GetSocketLocation(FName(TEXT("NearWallCheck_Socket")))+GetActorRightVector()*Offset;
	Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(Mesh1P->GetSocketRotation (FName(TEXT("NearWallCheck_Socket")))) * NearWallCheckRange);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CurrentWeapon);
	CollisionParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams TraceObjectParams;
	TraceObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	TraceObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel5);

	FHitResult Hit;

	bool bCanPeak = !GetWorld()->LineTraceSingleByObjectType(Hit, Startloc, Endloc,
		TraceObjectParams,CollisionParams);

	return bCanPeak;
}

void AFPS_Character::SwitchJumpMode()
{
	int8 Index;
	if (static_cast<int32>(JumpMode)==2)
	{
		Index=0;
	}
	else
	{
		Index=static_cast<int32>(JumpMode)+1;
	}

	JumpMode= static_cast<EJumpMode>(Index);
}

void AFPS_Character::ResetReloadTimeline()
{
	ReloadPullUpTimeline.Stop();
	//ReloadPullUpTimeline.SetPlaybackPosition(0.0f,true);

	ReloadPullDownTimeLine.Stop();
	//ReloadPullDownTimeLine.SetPlaybackPosition(0.0f,true);

	bIsReloading=false;

	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
}

void AFPS_Character::SetFOV(float value) const
{
	const float FOV = FMath::Lerp(DefaultFOV, CurrentWeapon->GetAdsFov(), value);
	const FTransform& Transform = UKismetMathLibrary::TLerp(bIsADSing? Mesh1P->GetRelativeTransform() : DefaultArmsTransform ,
		CalculateAdsTransform(),value);

	Mesh1P->SetRelativeTransform(Transform);
	FirstPersonCameraComponent->SetFieldOfView(FOV);
}

void AFPS_Character::SetRotation(float value)
{
	GetController()->SetControlRotation(FMath::Lerp(GetControlRotation(), FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.0f), value));
}

void AFPS_Character::EquipSlot1()
{
	if (bFirstSlotFull)
	{
		if (CurrentWeapon!=WeaponSlot_01)
		{
			if (bIsADSing)
			{
				ADSExit();
			}

			EquipWeapon();
		}
	}
}

void AFPS_Character::EquipSlot2()
{
	if (bSecondSlotFull)
	{
		if (CurrentWeapon!=WeaponSlot_02)
		{
			if (bIsADSing)
			{
				ADSExit();
			}
			
			EquipWeapon();
		}
	}
}

void AFPS_Character::EquipWeapon()
{
	if (!bIsChangingWeapon)
	{
		if (bIsReloading)
		{
			bIsReloading = false;
		}
		
		GetWorldTimerManager().ClearTimer(CurrentWeapon->AutoFireHandle);

		ResetReloadTimeline();
		
		bIsChangingWeapon = true; 
		bCanFire = false;
		
		EquipWeaponTimeLine.PlayFromStart();
	}
}

void AFPS_Character::ShowWeapon() const
{
	if (CurrentWeapon==WeaponSlot_01)
	{
		WeaponSlot_01->SetActorHiddenInGame(false);

		if (WeaponSlot_02)
		{
			WeaponSlot_02->SetActorHiddenInGame(true);
		}
	}
	if (CurrentWeapon==WeaponSlot_02)
	{
		WeaponSlot_02->SetActorHiddenInGame(false);

		if (WeaponSlot_01)
		{
			WeaponSlot_01->SetActorHiddenInGame(true);
		}
	}
}

void AFPS_Character::PickUpWeapon(TSubclassOf<AWeaponBase> WeaponToSpawn, int32 AmmoInMag, int32 ReservedAmmo)
{
	if (KnifeRef)
	{
		KnifeRef->SetActorHiddenInGame(true);
	}
	
	switch (WeaponSlot)
	{
	case EWeaponSlot::FirstSlot:

		if (!bFirstSlotFull)
		{
			SpawnToSlot(WeaponToSpawn,EWeaponSlot::FirstSlot,AmmoInMag,ReservedAmmo);
			return;
		}
		else
		{
			if (!bSecondSlotFull)
			{
				WeaponSlot = EWeaponSlot::SecondSlot;
				PickUpWeapon(WeaponToSpawn,AmmoInMag,ReservedAmmo);
				return;
			}
			else
			{
				//If we hold first slot and both slots are full
				//Drop the current slot weapon and attach the new one
				DropWeapon();
				SpawnToSlot(WeaponToSpawn,EWeaponSlot::FirstSlot,AmmoInMag,ReservedAmmo);
				return;
			}
		}

	case EWeaponSlot::SecondSlot:

		if (!bSecondSlotFull)
		{
			bIsReloading=false;
			SpawnToSlot(WeaponToSpawn,EWeaponSlot::SecondSlot,AmmoInMag,ReservedAmmo);
			return;
		} 
		else
		{
			//If we hold second slot and both slots are full
			//Drop the current slot weapon and attach the new one
			DropWeapon();
			SpawnToSlot(WeaponToSpawn,EWeaponSlot::SecondSlot,AmmoInMag,ReservedAmmo);
			return;
		}

	default:

		return;
	}
}

bool AFPS_Character::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation,
	int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible,
	int32* UserData) const
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoreActor);
	FHitResult Hit;

	//Cast a line trace from the AI character to the player camera location
	const bool bHit= GetWorld()->LineTraceSingleByChannel(Hit,ObserverLocation,FirstPersonCameraComponent->GetComponentLocation(),ECC_Visibility,Params);

	//if line trace doesnt hit anything then we can see player
	if (!bHit || (Hit.Actor.IsValid()&&Hit.Actor->IsOwnedBy(this)))
	{
		OutSeenLocation=FirstPersonCameraComponent->GetComponentLocation();
		OutSightStrength=1.0f;
		return true;
	}

	OutSightStrength=0.0f;
	return false;
}

void AFPS_Character::DropWeapon() const
{
	if (CurrentWeapon)
	{
		CurrentWeapon->SpawnPickUp();
	}
}


void AFPS_Character::SpawnToSlot(TSubclassOf<AWeaponBase> WeaponToSpawn, EWeaponSlot Slot, int32 AmmoInMag, int32 ReservedAmmo)
{
	switch (Slot)
	{
		
	case EWeaponSlot::FirstSlot:
		
		WeaponSlot_01 = GetWorld()->SpawnActor<AWeaponBase>(WeaponToSpawn);
		WeaponSlot_01->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
		true), WeaponSlot_01->GetSocketName());
		bFirstSlotFull = true;
		CurrentWeapon = WeaponSlot_01;
		break;

	case EWeaponSlot::SecondSlot:
		
		WeaponSlot_02 = GetWorld()->SpawnActor<AWeaponBase>(WeaponToSpawn);
		WeaponSlot_02->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
		true), WeaponSlot_02->GetSocketName());
		bSecondSlotFull = true;
		CurrentWeapon = WeaponSlot_02;
		break;

		
	default:
		
		return;
	}
	
	CurrentWeapon->SetCurrentAmmoInMag(AmmoInMag);
	CurrentWeapon->SetCurrentReservedAmmo(ReservedAmmo);
	
	WeaponSlot = Slot;
	bHasWeapon = true;
	bCanFire=true;
	ShowWeapon();
	ResetReloadTimeline();
	CharacterWeaponSwitch.Broadcast();
	UpdateWeaponHud.Broadcast();

	UGameplayStatics::PlaySound2D(GetWorld(),CurrentWeapon->SpawnSound);
}

void AFPS_Character::SpawnThrowable(TSubclassOf<AThrowingObjects> ThrowableToSpawn)
{
	Throwable= GetWorld()->SpawnActor<AThrowingObjects>(ThrowableToSpawn);
	Throwable->AttachToComponent(Mesh1P,FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true)
		,FName("Throwable_Socket"));
}

void AFPS_Character::WeaponSwitch()
{
	if (WeaponSlot==EWeaponSlot::FirstSlot)
	{
		WeaponSlot=EWeaponSlot::SecondSlot;
		CurrentWeapon=WeaponSlot_02;
	}
	else
	{
		WeaponSlot=EWeaponSlot::FirstSlot;
		CurrentWeapon=WeaponSlot_01;
	}

	UpdateWeaponHud.Broadcast();
	
	ShowWeapon();
	bHasWeapon = true;
	ResetReloadTimeline();
	CurrentWeapon->SetIsReadyToFire(true);
	CharacterWeaponSwitch.Broadcast();

	UGameplayStatics::PlaySound2D(GetWorld(),CurrentWeapon->EquipSound);
}

void AFPS_Character::EquipWeaponFinished()
{
	bIsChangingWeapon = false;
	bCanFire = true;
}

//Check if player is near a wall or object
void AFPS_Character::CheckIsNearWall()
{
	if (!CurrentWeapon)
	{
		return;
	}

	FVector Startloc;
	FVector Endloc;

	Startloc = Mesh1P->GetSocketLocation(FName(TEXT("NearWallCheck_Socket")));
	Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(Mesh1P->GetSocketRotation
		(FName(TEXT("NearWallCheck_Socket")))) * NearWallCheckRange);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CurrentWeapon);
	CollisionParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams TraceObjectParams;
	TraceObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	TraceObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	TraceObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel5);

	FHitResult Hit;

	bIsNearWall = GetWorld()->LineTraceSingleByObjectType(Hit, Startloc, Endloc,
		TraceObjectParams,CollisionParams);

	if (bIsNearWall&&bIsADSing&&!bIsPeakingAbove&&!bIsPeakingCorner)
	{
		ADSExit();
	}

	/*DrawDebugLine(GetWorld(), Startloc, Endloc, FColor::Green, false, 0);
	DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 0);*/
}

void AFPS_Character::OnFire()
{
	if (bCanFire&&bHasWeapon&& !bIsNearWall&&!bIsSprinting)
	{
		if (CurrentWeapon->MagStatus().bHasAmmo)
		{
			CurrentWeapon->PlayerPitchInput=0.0f;
			CurrentWeapon->PlayerYawInput=0.0f;
			CurrentWeapon->SetRecoilAllAddedPitch(0.0f);
			CurrentWeapon->SetRecoilAllAddedYaw(0.0f);
			
			CurrentWeapon->WeaponFire();
		}
		else if (CurrentWeapon->HasReservedAmmo())
		{
			Reload();
		}
	}
}

void AFPS_Character::StopFire()
{
	if (CurrentWeapon&& CurrentWeapon->IsWeaponAuto())
	{
		CurrentWeapon->StopFire();
	}
}

void AFPS_Character::Reload()
{
	if (CurrentWeapon&&!bIsChangingWeapon && !bIsReloading && CurrentWeapon->HasReservedAmmo())
	{
		if (!CurrentWeapon->MagStatus().bMagFull)
		{
			if (bIsADSing)
			{
				ADSExit();
			}
			GetWorldTimerManager().ClearTimer(CurrentWeapon->AutoFireHandle);
			CurrentWeapon->SetIsWeaponFiring(false);
			bIsReloading = true;
			bCanFire = false;
			UpdateWeaponHud.Broadcast();
			ReloadPullDownTimeLine.PlayFromStart();
		}
	}
}

void AFPS_Character::Hack()
{
	if (bIsScanning)
	{
		FVector Startloc;
		FVector Endloc;

		Startloc = FirstPersonCameraComponent->GetComponentLocation();
		Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(FirstPersonCameraComponent->GetComponentRotation()) * HackRange);

		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		IgnoreActors.Add(CurrentWeapon);
		
		//Object types to trace by
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel6));

		FHitResult Hit;

		const bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),Startloc,Endloc,HackSphereTraceRadius,
		ObjectTypes,false,IgnoreActors,EDrawDebugTrace::ForDuration,Hit,true,FLinearColor::Red,FLinearColor::Green,5);
		

		if (bHit)
		{
			AActor* HitActor=Hit.GetActor();
			if (HitActor)
			{
				IHackable* HackableInterface= Cast<IHackable>(HitActor);
				if (HackableInterface)
				{
					HackableInterface->FriendlyMode();
				}
			}
		}
	}
}

void AFPS_Character::HoldThrowable()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	bCanFire=false;
	SpawnThrowable(ThrowableClass);

	GetWorldTimerManager().SetTimer(PredictPathHandle,this,&AFPS_Character::PredictThrowablePath,GetWorld()->GetDeltaSeconds(),true);
}

void AFPS_Character::ThrowThrowable()
{
	Throwable->ProjectileMovement->Velocity=FirstPersonCameraComponent->GetForwardVector()*Throwable->VelocityMultiplier;
	Throwable->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Throwable->ProjectileMovement->Activate();
	Throwable->SetLifeSpan(5.0f);

	bCanFire=true;
	
	UGameplayStatics::PlaySound2D(GetWorld(),ThrowSound);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(false);
	}
	
	GetWorldTimerManager().ClearTimer(PredictPathHandle);
}

void AFPS_Character::AttachSuppressor()
{
	if (CurrentWeapon&&CurrentWeapon->CanBeSuppressed&&!bIsADSing)
	{
		//if suppressor is already on detach it else attach it
		if (CurrentWeapon->GetIsWeaponSuppressed())
		{
			CurrentWeapon->GetAttachmentMesh()->SetHiddenInGame(true);
			CurrentWeapon->SetIsWeaponSuppressed(false);
		}
		else
		{
			CurrentWeapon->GetAttachmentMesh()->SetHiddenInGame(false);
			CurrentWeapon->SetIsWeaponSuppressed(true);
		}

		UGameplayStatics::PlaySound2D(GetWorld(),AttachmentSound);
	}
}


FVector AFPS_Character::CalculateFloorInfluence() const
{
	const FVector FloorNormal =GetCharacterMovement()->CurrentFloor.HitResult.Normal;

	//if floor doesnt have an angle just return a zero vector
	if (FloorNormal.Equals(FVector::UpVector)||!GetCharacterMovement()->CurrentFloor.HitResult.bBlockingHit)
	{
		return FVector::ZeroVector;
	}

	//Project the down vector to the floor plane to find the direction
	//Dot product the down vector and floor normal to find + 1 to find how steep the angle is
	//Multiply the project and dot and force multiplier
	return FVector::VectorPlaneProject(FVector::DownVector,FloorNormal).GetSafeNormal()*
		(FVector::DotProduct(FVector::DownVector,FloorNormal)+1.0f)*SteepnessForceMultiplier;
}

void AFPS_Character::StartSprinting()
{
	bIsSprintKeyDown=true;
	
	if (bIsForwardKeyDown&&!bIsSprinting&&!bIsADSing&&!GetCharacterMovement()->IsFalling())
	{
		//if sprint pressed while we are crouching stop crouching
		if (bIsCrouching)
		{
			EndCrouch();
		}

		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
		
		//if EndCrouch Failed and we are still crouching dont sprint
		if (!bIsCrouching)
		{
			bIsSprinting=true;
			GetCharacterMovement()->MaxWalkSpeed=SprintWalkSpeed;
		}
	}
}

void AFPS_Character::StopSprinting()
{
	bIsSprintKeyDown=false;
	
	if (bIsSprinting&&!bIsCrouching&&!bIsADSing)
	{
		bIsSprinting=false;
		GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
	}
}

void AFPS_Character::PredictThrowablePath()
{
	FPredictProjectilePathParams PredictParams(Throwable->Sphere->GetScaledSphereRadius(),Throwable->GetActorLocation(),
		FirstPersonCameraComponent->GetForwardVector()*Throwable->VelocityMultiplier,2,ECC_Visibility,this);
	PredictParams.DrawDebugType=EDrawDebugTrace::ForOneFrame;
	
	FPredictProjectilePathResult PredictResults;
	
	UGameplayStatics::PredictProjectilePath(GetWorld(),PredictParams,PredictResults);
}

void AFPS_Character::SpawnKnife()
{
	KnifeRef = GetWorld()->SpawnActor(KnifeClass);
	KnifeRef->AttachToComponent(Mesh1P,FAttachmentTransformRules::SnapToTargetIncludingScale,FName("Knife_Socket"));
	if (bFirstSlotFull)
	{
		KnifeRef->SetActorHiddenInGame(true);
	}
}

FTransform AFPS_Character::CalculateAdsTransform() const
{
	const FTransform& Transform =UKismetMathLibrary::MakeRelativeTransform(Mesh1P->GetComponentTransform(),
		CurrentWeapon->GetGunMesh()->GetSocketTransform(FName("ADS_Socket")));
	return Transform;
}

void AFPS_Character::SetAlpha(float value)
{
	WeaponPullAlpha = value;
}

void AFPS_Character::SetCrouchAlpha(float value)
{
	const float Height = FMath::Lerp(StandingHalfHeight,CrouchHalfHeight,value);
	GetCapsuleComponent()->SetCapsuleHalfHeight(Height);

	FirstPersonCameraComponent->PostProcessSettings.VignetteIntensity=value/2;
}

void AFPS_Character::SetPeakAlpha(float value)
{
	const FVector CameraCurrentLoc= FMath::Lerp(CameraRelativeDefaultLoc,CameraRelativePeakLoc,value);
	FirstPersonCameraComponent->SetRelativeLocation(CameraCurrentLoc);

	if (value==1)
	{
		GetCharacterMovement()->DisableMovement();
		ADSEnter();
	}

	if (value<=0.01f)
	{
		GetCharacterMovement()->SetDefaultMovementMode();
	}
}

void AFPS_Character::OnReloadPullDownFinished()
{
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AFPS_Character::ReloadPullUp,
		CurrentWeapon->GetReloadTime(), false);
}

void AFPS_Character::OnReloadPullUpFinished()
{
	bIsReloading = false;
	bCanFire = true;
	CurrentWeapon->SetIsReadyToFire(true);
	CurrentWeapon->Reload();

	UpdateWeaponHud.Broadcast();
}

void AFPS_Character::ReloadPullUp()
{
	ReloadPullUpTimeline.PlayFromStart();
	UGameplayStatics::PlaySound2D(GetWorld(),CurrentWeapon->ReloadSound);
}

//Called when taking melee damage
void AFPS_Character::TakeMeleeDamage(float Damage, const FHitResult& HitResult)
{
	if (!bIsDead)
	{
		UpdateHealth(Damage);
	}
}

void AFPS_Character::TakeDamage(float Damage)
{
	if (!bIsDead)
	{
		UpdateHealth(Damage);
	}
}

void AFPS_Character::TakeDamage(const FAmmoData& AmmoData, float CriticalHitModifier, const FHitResult& HitResult)
{
	if (!bIsDead)
	{
		UpdateHealth(AmmoData.Damage);
	}
}

void AFPS_Character::UpdateHealth(float Damage)
{
	//If we have enough armor use it to block damage. else block as much
	//as possible with armor and block the rest with health
	if (CurrentArmor>=Damage)
	{
		CurrentArmor-=Damage;
	}
	else
	{
		Damage-=CurrentArmor;
		CurrentArmor=0.0f;
		CurrentHealth-=Damage;

		//call regenerate health function on a loop with a timer
		GetWorldTimerManager().SetTimer(HealthRegenHandle,this,&AFPS_Character::RegenerateHealth,HealthRegenRate,true,HealthRegenDelay);

		CurrentHealth= FMath::Clamp(CurrentHealth,0.0f,MaxHealth);

		if (CurrentHealth<=0.0f)
		{
			//if player is dead ragdoll the 3rd person mesh
			bIsDead=true;
			EventPlayerDeath.Broadcast();

			HandleDeath();
				
			GetWorldTimerManager().ClearTimer(HealthRegenHandle);
		}
	}
}


//Regenerates health up to max health
void AFPS_Character::RegenerateHealth()
{
	CurrentHealth+=HealthRegenAmount;
	CurrentHealth=FMath::Clamp(CurrentHealth,0.0f,MaxHealth);

	if (CurrentHealth>=MaxHealth)
	{
		GetWorldTimerManager().ClearTimer(HealthRegenHandle);
	}
}


//Hides first person mesh and shows the 3rd person mesh and simulates ragdoll
//Also blends out to a different camera so we can see the ragdoll body
void AFPS_Character::HandleDeath()
{
	Mesh1P->SetHiddenInGame(true);
	Mesh3P->SetHiddenInGame(false);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh3P->SetSimulatePhysics(true);

	const auto& OldController = UGameplayStatics::GetPlayerController(GetWorld(),0);

	DeathCamera= GetWorld()->SpawnActor<ACameraActor>(DeathCameraTransform->GetComponentLocation(),DeathCameraTransform->GetComponentRotation());
	
	DeathCamera->GetCameraComponent()->SetConstraintAspectRatio(false);

	UGameplayStatics::GetPlayerController(GetWorld(),0)->UnPossess();
	UGameplayStatics::PlaySound2D(GetWorld(),PlayerDeathSound);
	
	if (CurrentWeapon)
	{
		const FAttachmentTransformRules TransformRules= FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		CurrentWeapon->AttachToComponent(Mesh3P,TransformRules,TEXT("WeaponPoint"));
	}

	const FAttachmentTransformRules TransformRules = FAttachmentTransformRules::KeepWorldTransform;
	DeathCamera->AttachToComponent(GetCapsuleComponent(),TransformRules);
	
	OldController->SetViewTargetWithBlend(DeathCamera,DeathCameraBlendTime);

}

void AFPS_Character::Melee()
{
	if (bCanMelee)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle,[&]
		{
			bCanMelee=true;
		},1.0f,false);

		bCanMelee=false;

		UGameplayStatics::PlaySound2D(GetWorld(),KnifeSound);
	
		//Object types to trace by
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		//Ignore self
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Init(this, 1);

		//Start loc and end loc (from Player to the distance specified)
		const FVector StartLoc= GetActorLocation();
		const FVector EndLoc= StartLoc + GetActorForwardVector()*MeleeRange;

		FHitResult Hit;
	
		//Sphere trace to apply damage if enemy is hit
		const bool bHit =UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),StartLoc,EndLoc,MeleeSphereRadius,ObjectTypes,false,IgnoreActors,EDrawDebugTrace::None,Hit,true);

		//Apply damage if the actor has a take damage interface
		if (bHit)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(HitActor);
				if (TakeDamageInterface)
				{
					TakeDamageInterface->TakeMeleeDamage(MeleeDamage,Hit);
				}
			}
		}
	}
}

void AFPS_Character::Interact()
{
	//If interactable actor exists if its an interactable object then interact with it, else try grabbing body
	if (InteractHitActor)
	{
		if (AInteractables* Interactable=Cast<AInteractables>(InteractHitActor))
		{
			const bool bSuccessful = Interactable->InteractAction(this);
			if (bSuccessful)
			{
				UGameplayStatics::PlaySound2D(GetWorld(),Interactable->InteractSound);
			}
		}
		else
		{
			//If we are already grabbing sth then detach it else try to grab it
			bIsBodyGrabbed ? DetachGrabbed() : TryGrabBody();
		}
	}
	else
	{
		//Detach body on interact button press (if there is one)
		DetachGrabbed();
	}
}

void AFPS_Character::GrabPortalActor(APortalActor* PortalActor)
{
	bIsObjectGrabbed=true;
	PortalActor->bIsGrabbed=true;
	PortalActor->Grabber=this;
	GrabbedObject=PortalActor;
			
	PhysicsHandle->GrabComponentAtLocationWithRotation(PortalActor->StaticMesh,NAME_None,PortalActor->GetActorLocation(),PortalActor->GetActorRotation());
}

void AFPS_Character::TryGrabBody()
{
	//Object types to trace by
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(TraceChannel_CharacterBody));

	//Ignore self
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 1);

	//Start loc and end loc (from Player to the distance specified)
	const FVector StartLoc= FirstPersonCameraComponent->GetComponentLocation();
	const FVector EndLoc= StartLoc +FirstPersonCameraComponent->GetForwardVector()*GrabRange;

	FHitResult Hit;
	
	//Sphere trace to check for a body
	const bool bHit =UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),StartLoc,EndLoc,GrabSphereRadius,ObjectTypes,false,IgnoreActors,EDrawDebugTrace::None,Hit,true);

	if (bHit)
	{
		const AAI_Character* Body= Cast<AAI_Character>(Hit.Actor); 
		if (Body&&Body->IsDead())
		{
			bIsBodyGrabbed=true;
			PhysicsHandle->GrabComponentAtLocation(Hit.GetComponent(),Hit.BoneName,Hit.ImpactPoint);
		}
		else if (APortalActor* PortalActor = Cast<APortalActor>(Hit.GetActor()))
		{
			GrabPortalActor(PortalActor);
		}
	}
	
}

void AFPS_Character::UpdateDragBodyLocation() const
{
	PhysicsHandle->SetTargetLocation(Mesh1P->GetSocketLocation(FName("WeaponPoint")));
}

void AFPS_Character::UpdateGrabbedObjectLoc()
{
	FHitResult Hit;
	
	const FVector UpdateLoc=FirstPersonCameraComponent->GetComponentLocation() + FirstPersonCameraComponent->GetForwardVector()*GrabbedObjectDistance
	+ FirstPersonCameraComponent->GetUpVector() * -GrabbedObjectHeight;

	const FRotator UpdateRot = UKismetMathLibrary::MakeRotFromX(FirstPersonCameraComponent->GetForwardVector());

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(CurrentWeapon);

	FCollisionObjectQueryParams ObjectsToTrace;
	ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_Portal);
	
	GetWorld()->LineTraceSingleByObjectType(Hit,FirstPersonCameraComponent->GetComponentLocation(),UpdateLoc,ObjectsToTrace,CollisionParams);

	/*DrawDebugLine(GetWorld(), FirstPersonCameraComponent->GetComponentLocation(), UpdateLoc, FColor::Green, false, 0);
	DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 0);*/
	
	if (bGrabThroughPortal)
	{
		const APortal* OtherPortal = PortalComponent->PortalA ==Hit.GetActor() ? PortalComponent->PortalB : PortalComponent->PortalA;
		
		const FVector RelLocation = UKismetMathLibrary::InverseTransformLocation(OtherPortal->LinkedPortal->RotatedTransform,UpdateLoc);
		const FRotator RelRotation = UKismetMathLibrary::InverseTransformRotation(OtherPortal->LinkedPortal->RotatedTransform,UpdateRot);

		const FVector FinalLocation = UKismetMathLibrary::TransformLocation(OtherPortal->GetTransform(),RelLocation);
		const FRotator FinalRotation = UKismetMathLibrary::TransformRotation(OtherPortal->GetTransform(),RelRotation);

		/*DrawDebugBox(GetWorld(), FinalLocation, FVector(5, 5, 5), FColor::Red, false, 0);*/

		PhysicsHandle->SetTargetLocation(FinalLocation);
		PhysicsHandle->SetTargetRotation(FinalRotation);
	}
	else
	{
		PhysicsHandle->SetTargetLocation(UpdateLoc);
		PhysicsHandle->SetTargetRotation(UpdateRot);
		
		const FVector GrabbedLoc = PhysicsHandle->GetGrabbedComponent()->GetComponentLocation();

		//If grabbed component is too far release it (like when it gets stuck somewhere)
		if ((UpdateLoc-GrabbedLoc).Size()>300.0f)
		{
			UE_LOG(LogTemp,Warning,TEXT("Too far"))
			DetachGrabbed();
		}
	}
}

AActor* AFPS_Character::DetachGrabbed()
{
	const auto Grabbed =PhysicsHandle->GetGrabbedComponent();
	if (IsValid(Grabbed))
	{
		Grabbed->GetOwner()->SetActorLocation(Grabbed->GetOwner()->GetActorLocation());
		Grabbed->SetEnableGravity(true);

		if (APortalActor* PortalActor = Cast<APortalActor>(Grabbed->GetOwner()))
		{
			PortalActor->bIsGrabbed=false;
			PortalActor->Grabber=nullptr;
			GrabbedObject=nullptr;
		}
	}
	
	PhysicsHandle->ReleaseComponent();
	bIsBodyGrabbed=false;
	bIsObjectGrabbed=false;

	GetWorldTimerManager().ClearTimer(GrabTimerHandle);

	if (IsValid(Grabbed))
	{
		return Grabbed->GetOwner();
	}

	return nullptr;
}

void AFPS_Character::WeaponSway(float DeltaTime) const
{
	if (!CurrentWeapon)
	{
		return;
	}
	if (bIsADSing)
	{
		//Calculate fina; rotation based on input
		FRotator FinalRot;
		FinalRot.Yaw=GetInputAxisValue(FName("Turn"))*MaxSwayDegree;
		FinalRot.Roll=GetInputAxisValue(FName("LookUp"))*MaxSwayDegree;
		FinalRot.Pitch=GetInputAxisValue(FName("Turn"))*MaxSwayDegree;

		//interp to the final rotation
		FRotator DeltaRot=FMath::RInterpTo(CurrentWeapon->GetGunMesh()->GetRelativeRotation(),FinalRot,DeltaTime,SwaySpeed);

		//clamp it to max degrees
		DeltaRot.Yaw=FMath::Clamp(DeltaRot.Yaw,-MaxSwayDegree,MaxSwayDegree);
		DeltaRot.Pitch=FMath::Clamp(DeltaRot.Pitch,-MaxSwayDegree,MaxSwayDegree);
		DeltaRot.Roll=FMath::Clamp(DeltaRot.Roll,-MaxSwayDegree,MaxSwayDegree);

		//Set the relative rotation
		CurrentWeapon->GetGunMesh()->SetRelativeRotation(DeltaRot);
	}
	else
	{
		//Calculate fina; rotation based on input
		FRotator FinalRot;
		FinalRot.Yaw=GetInputAxisValue(FName("Turn"))*MaxSwayDegree;
		FinalRot.Roll=GetInputAxisValue(FName("LookUp"))*MaxSwayDegree;
		FinalRot.Pitch=GetInputAxisValue(FName("Turn"))*MaxSwayDegree;

		//interp to the final rotation
		FRotator DeltaRot=FMath::RInterpTo(Mesh1P->GetRelativeRotation(),FinalRot,DeltaTime,SwaySpeed);

		//clamp it to max degrees
		DeltaRot.Yaw=FMath::Clamp(DeltaRot.Yaw,-MaxSwayDegree,MaxSwayDegree);
		DeltaRot.Pitch=FMath::Clamp(DeltaRot.Pitch,-MaxSwayDegree,MaxSwayDegree);
		DeltaRot.Roll=FMath::Clamp(DeltaRot.Roll,-MaxSwayDegree,MaxSwayDegree);
		DeltaRot.Yaw-=90.0f;

		//Set the relative rotation
		Mesh1P->SetRelativeRotation(DeltaRot);
	}
}

void AFPS_Character::FindInteractable()
{
	FVector Startloc;
	FVector Endloc;

	Startloc = FirstPersonCameraComponent->GetComponentLocation();
	Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(FirstPersonCameraComponent->GetComponentRotation()) * InteractRange);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(CurrentWeapon);
		
	//Object types to trace by
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(TraceChannel_Interactable));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(TraceChannel_CharacterBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	
	FHitResult Hit;

	UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),Startloc,Endloc,InteractSphereTraceRadius,
	ObjectTypes,false,IgnoreActors,EDrawDebugTrace::None,Hit,true,FLinearColor::Red,FLinearColor::Green);
	
	AActor* HitActor=Hit.GetActor();
	if (IsValid(HitActor))
	{
		if (HitActor==InteractHitActor)
		{
			return;
		}
		
		InteractHitActor=HitActor;
		UpdateMainHud.Broadcast();
		return;
	}

	//if interact hit actor is already null dont do anything
	if (!InteractHitActor)
	{
		return;
	}
	
	InteractHitActor=nullptr;
	UpdateMainHud.Broadcast();
}

void AFPS_Character::TrySpawnPortalA()
{
	PortalComponent->TrySpawnPortal(true);
}

void AFPS_Character::TrySpawnPortalB()
{
	PortalComponent->TrySpawnPortal(false);
}

