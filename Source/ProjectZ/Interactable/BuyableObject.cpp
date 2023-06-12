// Fill out your copyright notice in the Description page of Project Settings.


#include "BuyableObject.h"
#include "../Player/FPS_Character.h"

bool ABuyableObject::InteractAction(AFPS_Character* PlayerRef)
{
	//If player has enough points perform buy action
	if (PlayerRef->GetPoints()>=Cost)
	{
		PlayerRef->SetPoints(PlayerRef->GetPoints()-Cost);
		PlayerRef->UpdateMainHud.Broadcast();
		BuyAction(PlayerRef);
		return true;
	}
	return false;
}

void ABuyableObject::BuyAction(AFPS_Character* PlayerRef)
{
}
