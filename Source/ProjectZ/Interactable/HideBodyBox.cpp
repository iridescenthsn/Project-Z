// Fill out your copyright notice in the Description page of Project Settings.


#include "HideBodyBox.h"

#include "../Player/FPS_Character.h"
#include "Kismet/GameplayStatics.h"

bool AHideBodyBox::InteractAction(AFPS_Character* PlayerRef)
{
	const auto Body = PlayerRef->DetachGrabbed();
	if (IsValid(Body))
	{
		Body->Destroy();
		return true;
	}
	return false;
}
