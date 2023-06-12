// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalComponent.generated.h"

class APortal;
class APortal_Wall;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZ_API UPortalComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class AFPS_Character* PawnRef;

	UPROPERTY(EditAnywhere,Category="Portal")
	float PortalSpawnRange=10000.0f;

	UPROPERTY(EditAnywhere,Category="Debug")
	bool bDebugLines=false;

	static void HandleIntersect(bool bPortalA, APortal* OldPortal, FVector NewPortalRelLoc, const APortal_Wall* PortalWall);

public:

	UPROPERTY(VisibleAnywhere,Transient)
	APortal* PortalA;

	UPROPERTY(VisibleAnywhere,Transient)
	APortal* PortalB;
	
	// Sets default values for this component's properties
	UPortalComponent();

	//The other player (the fake player if this one is real and wise versa)
	UPROPERTY(VisibleAnywhere, Category = "Teleporting")
	AFPS_Character* OtherPlayer;

	void TrySpawnPortal(bool bPortalA);

	static FVector ClampPortalToWall(const APortal_Wall* PortalWall, const FVector& NewPortalRelLoc);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly,Category="Portal")
	TSubclassOf<APortal> PortalClass;

	UPROPERTY(EditDefaultsOnly,Category="Sound")
	USoundBase* SpawnSound;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
