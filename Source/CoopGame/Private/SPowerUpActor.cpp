// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
	PowerUpInterval = 0.f;
	TotalNumTicks = 0;
	bIsPowerUpActive = false;
	SetReplicates(true);
}

void ASPowerUpActor::OnTickPowerUp()
{
	TicksProcessed++;
	OnPowerUpTicked();
	if (TicksProcessed >= TotalNumTicks)
	{
		OnExpired();
		bIsPowerUpActive = false;
		OnRep_PowerUpActive();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
	}
}

void ASPowerUpActor::OnRep_PowerUpActive()
{
	OnPowerUpStateChanged(bIsPowerUpActive);
}

void ASPowerUpActor::ActivatePowerUp(AActor* OtherActor)
{
	OnActivated(OtherActor);
	bIsPowerUpActive = true;
	OnRep_PowerUpActive();
	if (PowerUpInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &ASPowerUpActor::OnTickPowerUp, PowerUpInterval, true);
	}
	else
	{
		// Adding a comment to test Live++.
		OnTickPowerUp();
	}
}

void ASPowerUpActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerUpActor, bIsPowerUpActive);
}
