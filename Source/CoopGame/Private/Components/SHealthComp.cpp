// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComp.h"

// Sets default values for this component's properties
USHealthComp::USHealthComp()
{

	// ...
	DefaultHealth = 100.f;
}


// Called when the game starts
void USHealthComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComp::OnTakeAnyDamage);
	}
	Health = DefaultHealth;
}

void USHealthComp::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0)
	{
		Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
		OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}
