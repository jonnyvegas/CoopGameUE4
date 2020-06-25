// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComp.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "SGameMode.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

// Sets default values for this component's properties
USHealthComp::USHealthComp()
{

	// ...
	DefaultHealth = 100.f;

	// Only hook on server.
	SetIsReplicated(true);
	bIsDead = false;
	TeamNum = 255;
}


// Called when the game starts
void USHealthComp::BeginPlay()
{
	Super::BeginPlay();

	// Make sure we are only setting this up on the server.
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComp::OnTakeAnyDamage);
		}
	}
	Health = DefaultHealth;
}

void USHealthComp::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, (Health - OldHealth), nullptr, nullptr, nullptr);
}

void USHealthComp::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0)
	{
		if (DamageCauser == DamagedActor || !IsFriendly(DamagedActor, DamageCauser))
		{
			Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
			OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
			bIsDead = Health <= 0.f;
			if (bIsDead)
			{
				ASGameMode* GameMode = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
				if (GameMode)
				{
					GameMode->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
				}
			}
		}
	}
}

float USHealthComp::GetHealth() const
{
	return Health;
}

void USHealthComp::Heal(float HealAmount)
{
	if (HealAmount > 0 && Health > 0)
	{
		Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);
		OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
	}
}

bool USHealthComp::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA && ActorB)
	{
		USHealthComp* HealthCompA = Cast<USHealthComp>(ActorA->GetComponentByClass(USHealthComp::StaticClass()));
		USHealthComp* HealthCompB = Cast<USHealthComp>(ActorB->GetComponentByClass(USHealthComp::StaticClass()));
		if (HealthCompA && HealthCompB)
		{
			return HealthCompA->TeamNum == HealthCompB->TeamNum;
		}
	}
	return false;
}

void USHealthComp::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComp, Health);
}
