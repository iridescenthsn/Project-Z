// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalActor.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class APortal_Wall;
class AFPS_Character;
class UBoxComponent;

UCLASS()
class PROJECTZ_API APortal : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BorderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* PortalBox;
	

	UPROPERTY(VisibleAnywhere,Category="Property")
	bool bPortalA=false;

	//Called when we overlap teleport box collision and only the actors that overlapped here are the one we check for teleporting
	UFUNCTION()
	virtual void OnPortalBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//Called when end we overlap teleport box collision
	UFUNCTION()
	virtual void OnPortalEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Sets default values for this actor's properties
	APortal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* SceneCapture;

	void InitializePortal(bool PortalA, APortal* OtherPortal, APortal_Wall* PortalWall);

	//Connect two portals together
	void LinkPortals(APortal* OtherPortal);

	//Spawns a fake actor for actors near portal
	void AddFakeActor(APortalActor* RealActor);

	//Spawn fake player for players near portal
	void AddFakePlayer(AFPS_Character* RealPlayer);

	//Remove the fake Player
	void RemoveFakePlayer(const AFPS_Character* RealPlayer);

	//Destroys the fake actor
	void RemoveFakeActor(APortalActor* RealActor);

	//Swaps the fake and real actors
	void SwapFakeAndReal(APortalActor* PortalActor);

	UPROPERTY(Transient)
	APortal_Wall* CurrentWall;

	UPROPERTY(Transient)
	APortal* LinkedPortal;

	//Actors in teleport range (very close to portal)
	UPROPERTY(VisibleAnywhere,Category="Teleporting")
	TArray<AActor*> ActorsInPortal;

	//Actors in teleport range (very close to portal)
	UPROPERTY(VisibleAnywhere,Category="Teleporting")
	TArray<APortalActor*> PortalActors;

	//Fake players spawned by this portal
	UPROPERTY(VisibleAnywhere,Category="Teleporting")
	TArray<AFPS_Character*> FakePlayers;

	//Players in teleport range (very close to portal)
	UPROPERTY(VisibleAnywhere, Category = "Teleporting")
	TArray<AFPS_Character*> PlayersInPortal;
	
	//We use this by finding the relative transform of actors to it (so it gives us the relative transform of the actors to the linked portal when they are teleported)
	FTransform RotatedTransform;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void TeleportPlayer(AFPS_Character* Player);
	
	void TeleportActor(AActor* PortalActor) const;

	UPROPERTY(EditDefaultsOnly,Category="Material")
	UMaterialInstance* PortalABorderMat;

	UPROPERTY(EditDefaultsOnly,Category="Material")
	UMaterialInstance* PortalBBorderMat;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	UMaterialInstance* PortalAMeshMat;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	UMaterialInstance* PortalBMeshMat;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	UMaterialInstance* PortalDark;

	UPROPERTY(EditDefaultsOnly,Category="Material")
	UTextureRenderTarget2D* TextureRenderTargetA;

	UPROPERTY(EditDefaultsOnly,Category="Material")
	UTextureRenderTarget2D* TextureRenderTargetB;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
