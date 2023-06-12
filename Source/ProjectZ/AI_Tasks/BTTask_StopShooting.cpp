// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StopShooting.h"

#include "AIController.h"
#include "../AI/AI_Character.h"

EBTNodeResult::Type UBTTask_StopShooting::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Cast<AAI_Character>(OwnerComp.GetAIOwner()->GetPawn())->StopFire();
	return EBTNodeResult::Succeeded;
}
