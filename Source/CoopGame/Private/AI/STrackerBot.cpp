// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/SHealthComp.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	HealthComp = CreateDefaultSubobject<USHealthComp>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	bUseVelocityChange = false;
	bStartedSelfDestruction = false;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;
	ExplosionRadius = 200.f;
	ExplosionDamage = 40.f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	NextPoint = GetNextPathPoint();
	
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerCharacter)
	{
		UNavigationPath * NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerCharacter);
		if (NavPath->PathPoints.Num() > 1)
		{
			return NavPath->PathPoints[1];
		}
	}
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(USHealthComp* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!MatInstDyn)
	{
		MatInstDyn = MeshComp->CreateDynamicMaterialInstance(0, MeshComp->GetMaterial(0));
	}
	if (MatInstDyn)
	{
		MatInstDyn->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}
	if (Health <= 0.f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (!bExploded)
	{
		bExploded = true;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		if (ExplosionEffect)
		{
			UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		}
		if (ExplosionSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());
		}
		Destroy();
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.f, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Where the bot currently is
	DistanceToTarget = (GetActorLocation() - NextPoint).Size();

	// @TODO: Fix this because it isn't working for whatever reason.

	// The bot is not close enough
	if (DistanceToTarget > RequiredDistanceToTarget)
	{
		ForceDir = NextPoint - GetActorLocation();
		ForceDir.Normalize();
		ForceDir *= MovementForce;
		MeshComp->AddForce(ForceDir, NAME_None, bUseVelocityChange);
	}
	// The bot has reached the required point within the tolerance
	else
	{
		NextPoint = GetNextPathPoint();
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bStartedSelfDestruction)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ASTrackerBot::DamageSelf, 0.5f, true);
			bStartedSelfDestruction = true;
			if (SelfDestructSound)
			{
				UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
			}
		}
	}
}
