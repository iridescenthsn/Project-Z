// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "ClimbingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTZ_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class AFPS_Character* PawnRef;

	//helper function to check for collision in climbing location
	bool CanClimbHit(const FHitResult& Hit ,class UCapsuleComponent* CapsuleRef);

	//location to start climbing from
	FVector StartingLocation;

	//location to end climbing in
	FVector EndingLocation;

	FTimeline ClimbTimeline;

	UFUNCTION()
	void SetClimbAlpha(float value);

	UFUNCTION()
	void ClimbFinished();

public:	
	// Sets default values for this component's properties
	UClimbingComponent();

	//Checks if character can climb to a surface
	bool CanClimb();
	
	void Climb();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Climbing")
	bool bIsClimbing=false;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//how close character needs to be to trigger climbing
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Climbing")
	float ClimbingRange=70.0f;

	//min height in which climbing will happen
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Climbing")
	float MinClimbingHeight;

	//max height character can climb
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Climbing")
	float MaxClimbingHeight;

	//How high the surface checking will start
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Climbing")
	float ClimbCheckHeight=100.0f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly,Category="Climbing")
	UCurveFloat* ClimbCurve;
};


