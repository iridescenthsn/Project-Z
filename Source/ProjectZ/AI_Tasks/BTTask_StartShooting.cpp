// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StartShooting.h"

#include "AIController.h"
#include "../AI/AI_Character.h"

EBTNodeResult::Type UBTTask_StartShooting::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Cast<AAI_Character>(OwnerComp.GetAIOwner()->GetPawn())->StartFire();
	return EBTNodeResult::Succeeded;
}


