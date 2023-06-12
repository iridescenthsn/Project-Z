// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TurretAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API ATurretAIController : public AAIController
{
	GENERATED_BODY()

	ATurretAIController();
	
	UFUNCTION()
	void OnAIPerceptionUpdated(const TArray<AActor*>&DetectedActors);

	void SetTarget(AActor* Actor);

	UPROPERTY(Transient)
	class ATurret* TurretRef;

	virtual void BeginPlay() override;

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIPerception", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

};
