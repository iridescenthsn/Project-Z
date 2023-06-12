// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

UCLASS()
class PROJECTZ_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalActor();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	bool bIsFake=false;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	bool bIsGrabbed=false;

	//The other actor (Fake or real)
	UPROPERTY(VisibleAnywhere)
	APortalActor* OtherActor;

	UPROPERTY(VisibleAnywhere)
	class AFPS_Character* Grabber;

	UPROPERTY(VisibleAnywhere)
	class APortal* OverlappingPortal;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
