// Fill out your copyright notice in the Description page of Project Settings.


#include "ArmorBuy.h"
#include "../Player/FPS_Character.h"

void AArmorBuy::BuyAction(AFPS_Character* PlayerRef)
{
	PlayerRef->SetCurrentArmor(PlayerRef->GetMaxArmor());
}

bool AArmorBuy::InteractAction(AFPS_Character* PlayerRef)
{
	if (PlayerRef->GetPoints()>=Cost&&!(PlayerRef->GetCurrentArmor()==PlayerRef->GetMaxArmor()))
	{
		PlayerRef->SetPoints(PlayerRef->GetPoints()-Cost);
		PlayerRef->UpdateMainHud.Broadcast();
		BuyAction(PlayerRef);
		return true;
	}
	return false;
}
