// Fill out your copyright notice in the Description page of Project Settings.

#include "TurretAIController.h"
#include "Turret.h"
#include "Perception/AIPerceptionComponent.h"
#include "AI_Character.h"
#include "../Player/FPS_Character.h"



ATurretAIController::ATurretAIController()
{
	AIPerception=CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	AIPerception->Activate();

	AIPerception->OnPerceptionUpdated.AddDynamic(this, &ATurretAIController::OnAIPerceptionUpdated);
}

void ATurretAIController::OnAIPerceptionUpdated(const TArray<AActor*>& DetectedActors)
{
	for (const auto Actor : DetectedActors)
	{
		if (!TurretRef->IsFriendly())
		{
			//if turret is not friendly and sees other actors than players then ignore them
			if (!Actor->ActorHasTag(FName("Player")))
			{
				continue;
			}
			
			//if the turret already has an alive focus target if lost sight then stop focusing otherwise dont do anything
			if (TurretRef&&TurretRef->FocusTarget)
			{
				const auto Target = Cast<AFPS_Character>(TurretRef->FocusTarget);
				if (Target&& !Target->IsDead())
				{
					FActorPerceptionBlueprintInfo Info;
					AIPerception->GetActorsPerception(Actor,Info);
	
					const bool WasSuccessfullySeen = Info.LastSensedStimuli[0].WasSuccessfullySensed();
					if (!WasSuccessfullySeen)
					{
						TurretRef->StopFocusTarget();
					}
					else
					{
						return;
					}
				}
			}

			//if the detected actor is dead just ignore it
			const auto DetectedActor= Cast<AFPS_Character>(Actor);
			if (DetectedActor && DetectedActor->IsDead())
			{
				TurretRef->StopFocusTarget();
				break;
			}
			SetTarget(Actor);
			break;
		}
		else
		{
			//if turret is friendly ignore player
			if (Actor->ActorHasTag(FName("Player")))
			{
				continue;
			}

			//if the turret already has an alive focus target and the detected actor is same as target then if we lost sight
			//stop focusing. So if not lost sight or detected actor is some other actor then just return
			if (TurretRef&&TurretRef->FocusTarget)
			{
				const auto Target = Cast<AAI_Character>(TurretRef->FocusTarget);
				if (Target&& !Target->IsDead())
				{
					if(TurretRef->FocusTarget==Actor)
					{
						FActorPerceptionBlueprintInfo Info;
						AIPerception->GetActorsPerception(Actor,Info);
	
						const bool WasSuccessfullySeen = Info.LastSensedStimuli[0].WasSuccessfullySensed();
						if (!WasSuccessfullySeen)
						{
							TurretRef->StopFocusTarget();
							continue;
						}
						else
						{
							return;
						}
					}
					return;
				}
			}
			
			//if the detected actor is dead exclude it from list of detected actors and call the current function again
			const auto DetectedActor= Cast<AAI_Character>(Actor);
			if (DetectedActor && DetectedActor->IsDead())
			{
				TArray<AActor*> AliveActors;
				for (const auto TheActor : DetectedActors)
				{
					const auto NewActor= Cast<AAI_Character>(TheActor);
					if ( NewActor&& !NewActor->IsDead() )
					{
						AliveActors.Push(NewActor);
					}
				}

				//if there are no alive actors left stop shooting and go back to normal
				if (AliveActors.Num()==0)
				{
					TurretRef->StopFocusTarget();
					return;
				}
				
				OnAIPerceptionUpdated(AliveActors);
				break;
			}
			SetTarget(Actor);
			break;
		}
	}
}

void ATurretAIController::SetTarget(AActor* Actor)
{
	//Get perception of each detected actor
	FActorPerceptionBlueprintInfo Info;
	AIPerception->GetActorsPerception(Actor,Info);
	
	const bool WasSuccessfullySeen = Info.LastSensedStimuli[0].WasSuccessfullySensed();
	
	if (WasSuccessfullySeen)
	{
		TurretRef->StartFocusTarget(Actor);
	}
	else
	{
		TurretRef->StopFocusTarget();
	}
}

void ATurretAIController::BeginPlay()
{
	Super::BeginPlay();
	TurretRef=Cast<ATurret>(GetPawn());
}
