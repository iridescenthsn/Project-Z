// Fill out your copyright notice in the Description page of Project Settings.


#include "FindNextPatrollPoint.h"

#include "AIController.h"
#include "../AI/AI_Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/TargetPoint.h"

EBTNodeResult::Type UFindNextPatrollPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const auto AICharacter = Cast<AAI_Character>(OwnerComp.GetAIOwner()->GetPawn());

	auto PatrolPoints= AICharacter->GetPatrolPoints();
	if (PatrolPoints.Num()==0)
	{
		UE_LOG(LogTemp,Warning,TEXT("No patrol points"))
		return EBTNodeResult::Failed;
	}

	if (AICharacter->GetCurrentPatrolPoint() > PatrolPoints.Num())
	{
		AICharacter->SetCurrentPatrolPoint(1);
	}
	
	const ATargetPoint* NextTargetPoint= PatrolPoints[AICharacter->GetCurrentPatrolPoint()-1];
	
	AICharacter->SetCurrentPatrolPoint(AICharacter->GetCurrentPatrolPoint()+1);
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(FName(TEXT("Target Location")),NextTargetPoint->GetActorLocation());
	
	return EBTNodeResult::Succeeded;
}


