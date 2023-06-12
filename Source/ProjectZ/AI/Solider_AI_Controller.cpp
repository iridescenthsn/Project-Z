// Fill out your copyright notice in the Description page of Project Settings.


#include "Solider_AI_Controller.h"
#include "Perception/AIPerceptionComponent.h"
#include "AI_Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASolider_AI_Controller::ASolider_AI_Controller()
{
	AIPerception=CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	AIPerception->OnPerceptionUpdated.AddDynamic(this, &ASolider_AI_Controller::OnAIPerceptionUpdated);
}

void ASolider_AI_Controller::OnAIPerceptionUpdated(const TArray<AActor*>& DetectedActors)
{
	if (!GetBlackboardComponent())
	{
		return;
	}
	
	for (const auto& Actor:DetectedActors)
	{
		if (!Actor->ActorHasTag(FName("Player")))
		{
			const auto AiCharacter= Cast<AAI_Character>(Actor);
			if (AiCharacter)
			{
				if (AiCharacter->IsDead()&&!Actor->ActorHasTag(FName("Seen")))
				{
					AiCharacter->Tags.Add(FName("Seen"));
					ChangeAiState(EAiState::Investigating);
					GetBlackboardComponent()->SetValueAsVector(FName("Target Location"),Actor->GetActorLocation());
				}
			}
			continue;
		}
		
		//reset this value every time
		GetBlackboardComponent()->SetValueAsBool(FName("TookDamage"),false);
		
		//Get perception of each detected actor
		FActorPerceptionBlueprintInfo Info;
		AIPerception->GetActorsPerception(Actor,Info);

		//Check if we seen player or lost sight
		const bool WasSuccessfullySeen = Info.LastSensedStimuli[0].WasSuccessfullySensed();
		
		//Set the player key and chase if we see player
		//Otherwise Investigate
		if (WasSuccessfullySeen)
		{
			ChangeAiState(EAiState::Chasing);
			GetBlackboardComponent()->SetValueAsObject(FName("Player"),Actor);
			GetBlackboardComponent()->SetValueAsBool((FName("Knows Player Location")),true);
			GetBlackboardComponent()->SetValueAsBool((FName("Can see player")),true);
			SetFocus(Actor);
			continue;
		}

		//If we already saw player but now we cant see player then we lost sight
		if (GetBlackboardComponent()->GetValueAsBool(FName("Can see player"))&&!WasSuccessfullySeen)
		{
			ChangeAiState(EAiState::Investigating);
			GetBlackboardComponent()->ClearValue(FName("Player"));
			GetBlackboardComponent()->SetValueAsVector(FName("Target Location"),Info.LastSensedStimuli[0].StimulusLocation);
			GetBlackboardComponent()->SetValueAsBool((FName("Knows Player Location")),false);
			GetBlackboardComponent()->SetValueAsBool((FName("Can see player")),false);
			ClearFocus(EAIFocusPriority::Gameplay);
			continue;
		}
		
		//Investigate sound location
		const bool WasSuccessFullyHeard = Info.LastSensedStimuli[1].WasSuccessfullySensed();
		const FName& Tag = Info.LastSensedStimuli[1].Tag;
		if (WasSuccessFullyHeard && Tag.IsEqual(FName("AI_Noise")))
		{
			ChangeAiState(EAiState::Patrolling);
			ChangeAiState(EAiState::Investigating);
			
			GetBlackboardComponent()->SetValueAsBool(FName("TookDamage"),false);
			GetBlackboardComponent()->SetValueAsVector(FName("Target Location"),Info.LastSensedStimuli[1].StimulusLocation);
			continue;
		}
		
		//Investigate around Own location
		const bool WasSuccessfullyDamaged=Info.LastSensedStimuli[2].WasSuccessfullySensed();
		if (WasSuccessfullyDamaged)
		{
			ChangeAiState(EAiState::Patrolling);
			ChangeAiState(EAiState::Investigating);
			GetBlackboardComponent()->SetValueAsBool(FName("TookDamage"),true);
			continue;
		}
	}
}

void ASolider_AI_Controller::BeginPlay()
{
	Super::BeginPlay();

	PawnRef=Cast<AAI_Character>(GetPawn());

	RunBehaviorTree(TheTree);
}

void ASolider_AI_Controller::ChangeAiState(EAiState AiState)
{
	GetBlackboardComponent()->SetValueAsEnum(FName("AIState"),static_cast<uint8>(AiState));

	PawnRef=Cast<AAI_Character>(GetPawn());
	
	switch (AiState)
	{
	case EAiState::Chasing:
		PawnRef->GetCharacterMovement()->MaxWalkSpeed=ChaseSpeed;
		break;

	case EAiState::Investigating:
		PawnRef->GetCharacterMovement()->MaxWalkSpeed=InvestigatingSpeed;
		break;

	case EAiState::Patrolling:
		PawnRef->GetCharacterMovement()->MaxWalkSpeed=PatrollingSpeed;
		break;
	}
	
}
