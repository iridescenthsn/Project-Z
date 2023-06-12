// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

#include "../CollisionChannels.h"
#include "PortalActor.h"
#include "PortalComponent.h"
#include "Portal_Wall.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Player/FPS_Character.h"
#include "../Projectiles/Projectile.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent=SceneRoot;

	PortalMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Portal mesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->CastShadow=false;

	PortalBox = CreateDefaultSubobject<UBoxComponent>(FName("Portal box"));
	PortalBox->SetupAttachment(RootComponent);
	
	BorderMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Portal border"));
	BorderMesh->SetupAttachment(RootComponent);
	BorderMesh->CastShadow=false;

	SceneCapture=CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture"));
	SceneCapture->SetupAttachment(RootComponent);
}

void APortal::InitializePortal(bool PortalA, APortal* OtherPortal, APortal_Wall* PortalWall)
{
	bPortalA = PortalA;
	CurrentWall = PortalWall;

	SceneCapture->HiddenActors=CurrentWall->IgnoredWalls;

	if (PortalA)
	{
		BorderMesh->SetMaterial(0, PortalABorderMat);
		SceneCapture->TextureTarget = TextureRenderTargetA;
	}
	else
	{
		BorderMesh->SetMaterial(0, PortalBBorderMat);
		SceneCapture->TextureTarget = TextureRenderTargetB; 
	}
}

void APortal::LinkPortals(APortal* OtherPortal)
{
	LinkedPortal = OtherPortal;
	
	if (bPortalA)
	{
		IsValid(OtherPortal) ? PortalMesh->SetMaterial(0, PortalBMeshMat) : PortalMesh->SetMaterial(0, PortalDark);
	}
	else
	{
		IsValid(OtherPortal) ? PortalMesh->SetMaterial(0, PortalAMeshMat) : PortalMesh->SetMaterial(0, PortalDark);
	}
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
	PortalBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &APortal::OnPortalBeginOverlap);
	PortalBox->OnComponentEndOverlap.AddUniqueDynamic(this, &APortal::OnPortalEndOverlap);

	//Finds the rotated transform
	//We use this by finding the relative transform of actors to it (so it gives us the relative transform of the actors to the linked portal when they are teleported)
	RotatedTransform=UKismetMathLibrary::ComposeTransforms(FTransform(FRotator(0,180,0)),GetActorTransform());
}

void APortal::OnPortalBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFPS_Character* Player = Cast<AFPS_Character>(OtherActor))
	{
		//Ignore fake player
		if (Player->bIsFake)
		{
			return;
		}
		
		if (IsValid(LinkedPortal)) 
		{
			Player->GetCapsuleComponent()->SetCollisionResponseToChannel(TraceChannel_PortalWall,ECR_Ignore);
		}

		if (IsValid(LinkedPortal))
		{
			AddFakePlayer(Player);
		}
		
		PlayersInPortal.AddUnique(Player); 
	}
	else
	{
		if (IsValid(LinkedPortal))
		{
			OtherComp->SetCollisionResponseToChannel(TraceChannel_PortalWall,ECR_Ignore);
		}

		if (APortalActor* PortalActor = Cast<APortalActor>(OtherActor)) 
		{
			//Ignore if its a fake actor
			if (PortalActor->bIsFake)
			{
				return;
			}

			if (IsValid(LinkedPortal))
			{
				AddFakeActor(PortalActor);
			}
			
			PortalActor->OverlappingPortal=this;
			PortalActors.AddUnique(PortalActor);
		}
		else
		{
			ActorsInPortal.AddUnique(OtherActor);
		}
	}
}

void APortal::OnPortalEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AFPS_Character* Player = Cast<AFPS_Character>(OtherActor))
	{
		//Ignore fake player
		if (Player->bIsFake)
		{
			return;
		}
		
		if (IsValid(LinkedPortal))
		{
			Player->GetCapsuleComponent()->SetCollisionResponseToChannel(TraceChannel_PortalWall,ECR_Block);
		}

		RemoveFakePlayer(Player);
		PlayersInPortal.Remove(Player);
	}
	else 
	{
		if (IsValid(LinkedPortal))
		{
			OtherComp->SetCollisionResponseToChannel(TraceChannel_PortalWall,ECR_Block);
		}

		if (APortalActor* PortalActor = Cast<APortalActor>(OtherActor)) 
		{
			//Ignore if its a fake actor
			if (PortalActor->bIsFake)
			{
				return;
			}

			RemoveFakeActor(PortalActor);
			
			PortalActor->OverlappingPortal=nullptr;
			PortalActors.Remove(PortalActor);
		}
		else
		{
			ActorsInPortal.Remove(OtherActor);
		}
	}
}

void APortal::AddFakeActor(APortalActor* RealActor)
{
	//Spawn a new Fake actor
	APortalActor* FakeActor =GetWorld()->SpawnActor<APortalActor>(RealActor->GetClass());
	
	//Set references to each other
	RealActor->OtherActor=FakeActor;
	FakeActor->OtherActor=RealActor;
	
	//Set the new spawned actor to be a fake
	FakeActor->bIsFake=true;  
}

void APortal::AddFakePlayer(AFPS_Character* RealPlayer)
{
	//Spawn fake player
	AFPS_Character* FakePlayer = GetWorld()->SpawnActor<AFPS_Character>(RealPlayer->GetClass());
	FakePlayers.AddUnique(FakePlayer);

	//Fake player doesnt have collision
	FakePlayer->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FakePlayer->GetMesh3P()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Set view target to real player
	UGameplayStatics::GetPlayerController(GetWorld(),0)->SetViewTarget(RealPlayer);

	//if a fake player already exists destroy it
	if (IsValid(RealPlayer->GetPortalComponent()->OtherPlayer))
	{
		//We remove it by calling it from linked portal because every portal handles their own fake players
		LinkedPortal->RemoveFakePlayer(RealPlayer);
	}

	//Set references to each other
	RealPlayer->GetPortalComponent()->OtherPlayer=FakePlayer;
	FakePlayer->GetPortalComponent()->OtherPlayer=RealPlayer;

	//SceneCapture should ignore fake actor
	LinkedPortal->SceneCapture->HiddenActors.AddUnique(FakePlayer);

	FakePlayer->bIsFake=true; 
}

void APortal::RemoveFakePlayer(const AFPS_Character* RealPlayer)
{
	AFPS_Character* FakePlayer = RealPlayer->GetPortalComponent()->OtherPlayer;

	//If fake actor is valid and its spawned by this portal then destroy it
	if (IsValid(FakePlayer) && FakePlayers.Contains(FakePlayer))
	{
		FakePlayers.Remove(FakePlayer);
		FakePlayer->Destroy();
		RealPlayer->GetPortalComponent()->OtherPlayer=nullptr;
	}
	
}

void APortal::RemoveFakeActor(APortalActor* RealActor)
{
	APortalActor* FakeActor = RealActor->OtherActor;
	if (IsValid(FakeActor))
	{
		if (FakeActor->bIsGrabbed)
		{
			AFPS_Character* Grabber = FakeActor->Grabber;
		
			Grabber->DetachGrabbed();
			Grabber->GrabPortalActor(RealActor);
			Grabber->bGrabThroughPortal=!Grabber->bGrabThroughPortal;

			UE_LOG(LogTemp,Warning,TEXT("New grab"))
		}
		
		FakeActor->Destroy();
		RealActor->OtherActor=nullptr;
	}
}

void APortal::SwapFakeAndReal(APortalActor* PortalActor)
{
	APortalActor* NewRealActor =PortalActor->OtherActor;

	//Swap the fake with the real actor
	PortalActor->bIsFake=true;
	NewRealActor->bIsFake=false;

	//Remove the actors from this portals arrays
	PortalActors.Remove(PortalActor);

	//Set the relative velocity of the real actor to the fake actor
	const FVector RelativeVelocity = UKismetMathLibrary::InverseTransformDirection(RotatedTransform, PortalActor->GetVelocity());
	const FVector TeleportVelocity = UKismetMathLibrary::TransformDirection(LinkedPortal->GetTransform(), RelativeVelocity);
	Cast<UPrimitiveComponent>(NewRealActor->GetRootComponent())->SetPhysicsLinearVelocity(TeleportVelocity);
				
	//The linked portal will now handle the actors
	LinkedPortal->PortalActors.AddUnique(NewRealActor);
	NewRealActor->OverlappingPortal=LinkedPortal;
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsValid(LinkedPortal))
		return;
	
	//Calculate the relative transform from this portal to the player camera and apply that to the linked portal
	const FTransform CameraTransform = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransform().GetRelativeTransform(RotatedTransform);
	LinkedPortal->SceneCapture->SetRelativeTransform(CameraTransform);
	LinkedPortal->SceneCapture->FOVAngle=UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetFOVAngle();
		
	//Set the clip plane base and normal to the portals location and normal so it doesnt render anything behind the portal
	SceneCapture->ClipPlaneBase = GetActorLocation();
	SceneCapture->ClipPlaneNormal = SceneCapture->GetForwardVector();

	//Resize the scene capture target to the player viewport size to have the correct view (get rid of stretched view)
	if (SceneCapture->TextureTarget)
	{
		int32 SizeX, SizeY;
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetViewportSize(SizeX, SizeY);

		SceneCapture->TextureTarget->ResizeTarget(SizeX, SizeY);
	}


	//Checks if we can teleport actors (if actors are behind portal)
	for (int32 i=0 ; i<ActorsInPortal.Num() ; i++)
	{
		AActor* Actor=ActorsInPortal[i];
		
		//Calculate where the actor will be in the next frame
		FVector NextFrameLoc = Actor->GetActorLocation() + GetWorld()->GetDeltaSeconds() * Actor->GetVelocity();
		const float DotProduct = FVector::DotProduct(GetActorForwardVector(),(NextFrameLoc - GetActorLocation()).GetSafeNormal());
		
		if (DotProduct<=0.0f)
		{
			TeleportActor(Actor);
		}
	}

	//Checks if we can swap fake and real actors
	for (int32 i=0 ; i<PortalActors.Num() ; i++)
	{
		APortalActor* PortalActor = PortalActors[i];
		
		//Calculate where the actor will be in the next frame
		FVector NextFrameLoc = PortalActor->GetActorLocation() + GetWorld()->GetDeltaSeconds() * PortalActor->GetVelocity();
		const float DotProduct = FVector::DotProduct(GetActorForwardVector(),(NextFrameLoc - GetActorLocation()).GetSafeNormal());
		
		if (DotProduct<=0.0f)
		{
			SwapFakeAndReal(PortalActor);
		}
	}

	//Sets the location and rotation for the fake actor
	for (int32 i=0 ; i<PortalActors.Num() ; i++)
	{
		APortalActor* FakeActor = PortalActors[i]->OtherActor;
		
		if (FakeActor && FakeActor->bIsGrabbed)
		{
			//Find relative transform of the actor to this portal
			const FTransform RelActorTransform =FakeActor->GetTransform().GetRelativeTransform(LinkedPortal->RotatedTransform);

			//Transform that relative transform to world space using the linked portal transform
			const FVector RealActorLocation = UKismetMathLibrary::TransformLocation(GetTransform(),RelActorTransform.GetLocation());
			const FRotator RealActorRotation = UKismetMathLibrary::TransformRotation(GetTransform(),RelActorTransform.Rotator());
			
			PortalActors[i]->SetActorLocationAndRotation(RealActorLocation,RealActorRotation);
		}
		else
		{
			//Find relative transform of the actor to this portal
			const FTransform RelActorTransform = PortalActors[i]->GetTransform().GetRelativeTransform(RotatedTransform);

			//Transform that relative transform to world space using the linked portal transform
			const FVector FakeActorLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetTransform(),RelActorTransform.GetLocation());
			const FRotator FakeActorRotation = UKismetMathLibrary::TransformRotation(LinkedPortal->GetTransform(),RelActorTransform.Rotator());

			if (IsValid(FakeActor))
			{
				FakeActor->SetActorLocationAndRotation(FakeActorLocation,FakeActorRotation);
			}
		}
	}

	//Sets location, Control rotation and velocity for fake player
	for (int32 i=0 ; i<PlayersInPortal.Num() ; i++)
	{
		AFPS_Character* FakePlayer = PlayersInPortal[i]->GetPortalComponent()->OtherPlayer;
		
		//Find relative transform of the player to this portal
		const FVector RelPlayerLocation = UKismetMathLibrary::InverseTransformLocation(RotatedTransform,PlayersInPortal[i]->GetActorLocation());
		const FRotator RelPlayerRotation = UKismetMathLibrary::InverseTransformRotation(RotatedTransform,PlayersInPortal[i]->GetControlRotation());
		const FVector RelPlayerVelocity = UKismetMathLibrary::InverseTransformDirection(RotatedTransform,PlayersInPortal[i]->GetMovementComponent()->Velocity);
 
		//Transform that relative transform to world space using the linked portal transform
		const FVector FakeActorLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetTransform(),RelPlayerLocation);
		const FRotator FakeActorRotation = UKismetMathLibrary::TransformRotation(LinkedPortal->GetTransform(),RelPlayerRotation);
		const FVector FakePlayerVelocity = UKismetMathLibrary::TransformDirection(LinkedPortal->GetTransform(),RelPlayerVelocity);
		
		if (IsValid(FakePlayer))
		{
			FakePlayer->SetActorLocation(FakeActorLocation);
			FakePlayer->GetController()->SetControlRotation(FakeActorRotation);
			FakePlayer->SetActorRotation(FRotator(0,FakeActorRotation.Yaw,0));
			FakePlayer->GetMovementComponent()->Velocity=FakePlayerVelocity;
		}
	}

	//Checks if we can teleport the player
	for (int32 i = 0 ; i<PlayersInPortal.Num() ; i++)
	{
		AFPS_Character* Player = PlayersInPortal[i];

		//Calculates where the player camera will be in the next frame based on character velocity
		const FVector NextFrameLocVel = Player->FirstPersonCameraComponent->GetComponentLocation() + GetWorld()->GetDeltaSeconds() * Player->GetVelocity();

		//Calculates where the player camera will be in the next frame based on last movement input vector
		const FVector NextFrameLocInput = Player->FirstPersonCameraComponent->GetComponentLocation() + Player->GetPendingMovementInputVector();
		
		const float DotProductVel = FVector::DotProduct(GetActorForwardVector(), (NextFrameLocVel - GetActorLocation()).GetSafeNormal());
		const float DotProductInput = FVector::DotProduct(GetActorForwardVector(), (NextFrameLocInput - GetActorLocation()).GetSafeNormal());
		
		//If the player camera is behind the portal next frame then teleport
		if (Player->bCanTeleport && (DotProductInput <=0.0f || DotProductVel<=0.0f))
		{
			TeleportPlayer(Player);
		}
	}
}

void APortal::TeleportPlayer(AFPS_Character* Player) 
{
	//Calculates relative location, Rotation and velocity of the player to the this portal
	const FVector RelativeLocation = UKismetMathLibrary::InverseTransformLocation(RotatedTransform, Player->GetActorLocation());
	const FRotator RelativeRotation = UKismetMathLibrary::InverseTransformRotation(RotatedTransform, Player->GetControlRotation());
	const FVector RelativeVelocity = UKismetMathLibrary::InverseTransformDirection(RotatedTransform, Player->GetVelocity());

	//Transforms the location, rotation and velocity of the player in the context of the linked portal
	const FVector TeleportLocation= UKismetMathLibrary::TransformLocation(LinkedPortal->GetTransform(), RelativeLocation);
	const FRotator TeleportRotation = UKismetMathLibrary::TransformRotation(LinkedPortal->GetTransform(), RelativeRotation);
	const FVector TeleportVelocity = UKismetMathLibrary::TransformDirection(LinkedPortal->GetTransform(), RelativeVelocity);
	
	Player->GetController()->SetControlRotation(TeleportRotation);
	Player->SetActorLocation(TeleportLocation + GetActorForwardVector()*2);
	Player->LockTeleport();
	Player->GetMovementComponent()->Velocity=TeleportVelocity;
			
	Player->CorrectRotation();

	Player->bGrabThroughPortal=false;

	UE_LOG(LogTemp,Warning,TEXT("Teleport %s"),*GetDebugName(this))
}

void APortal::TeleportActor(AActor* PortalActor) const
{
	//Calculates relative location, Rotation and velocity of the player to the this portal
	const FVector RelativeLocation = UKismetMathLibrary::InverseTransformLocation(RotatedTransform, PortalActor->GetActorLocation());
	const FRotator RelativeRotation = UKismetMathLibrary::InverseTransformRotation(RotatedTransform, PortalActor->GetActorRotation());
	FVector RelativeVelocity;

	const AProjectile* Projectile = Cast<AProjectile>(PortalActor);
	if (Projectile)
		RelativeVelocity = UKismetMathLibrary::InverseTransformDirection(RotatedTransform, Projectile->ProjectileMovement->Velocity);
	else
		RelativeVelocity = UKismetMathLibrary::InverseTransformDirection(RotatedTransform, PortalActor->GetVelocity());

	//Transforms the location, rotation and velocity of the player in the context of the linked portal
	const FVector TeleportLocation= UKismetMathLibrary::TransformLocation(LinkedPortal->GetTransform(), RelativeLocation);
	const FRotator TeleportRotation = UKismetMathLibrary::TransformRotation(LinkedPortal->GetTransform(), RelativeRotation);
	const FVector TeleportVelocity = UKismetMathLibrary::TransformDirection(LinkedPortal->GetTransform(), RelativeVelocity);
			
	PortalActor->SetActorRotation(TeleportRotation);
	PortalActor->SetActorLocation(TeleportLocation);
	
	if(Projectile)
		Projectile->ProjectileMovement->Velocity=TeleportVelocity;
	else
	{
		Cast<UPrimitiveComponent>(PortalActor->GetRootComponent())->SetPhysicsLinearVelocity(TeleportVelocity);
	}
}




