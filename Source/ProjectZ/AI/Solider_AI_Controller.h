// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie_AI_Controller.h"
#include "Solider_AI_Controller.generated.h"

UENUM(BlueprintType)
enum class EAiState : uint8
{
	Patrolling,
	Chasing,
	Investigating,
};

/**
 * 
 */
UCLASS()
class PROJECTZ_API ASolider_AI_Controller : public AAIController
{
	GENERATED_BODY()

	ASolider_AI_Controller();

	UFUNCTION()
	void OnAIPerceptionUpdated(const TArray<AActor*>&DetectedActors);

	virtual void BeginPlay() override;
	
	UPROPERTY()
	class AAI_Character* PawnRef;

	
protected:

	UPROPERTY(EditAnywhere,Category="AiWalkSpeed")
	float ChaseSpeed;

	UPROPERTY(EditAnywhere,Category="AiWalkSpeed")
	float PatrollingSpeed;

	UPROPERTY(EditAnywhere,Category="AiWalkSpeed")
	float InvestigatingSpeed;

	UPROPERTY(EditDefaultsOnly)
	UBehaviorTree* TheTree;
	
public:
	
	UFUNCTION(BlueprintCallable)
	void ChangeAiState(EAiState AiState);

	void SetPawnRef(AAI_Character* const PawnToSet) {PawnRef = PawnToSet;}

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	FVector FirstGoalLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIPerception", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;
};
