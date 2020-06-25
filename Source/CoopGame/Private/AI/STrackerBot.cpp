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
#include "Components/AudioComponent.h"
#include "Engine.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	BuddySphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("BuddySphereComp"));
	RootComponent = MeshComp;

	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);

	SphereComp->SetSphereRadius(200.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	HealthComp = CreateDefaultSubobject<USHealthComp>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	AudioComp->SetupAttachment(RootComponent);

	BuddySphereComp->SetupAttachment(RootComponent);

	BuddySphereComp->SetSphereRadius(1000.f);


	bUseVelocityChange = false;
	bStartedSelfDestruction = false;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;
	ExplosionRadius = 350.f;
	ExplosionDamage = 60.f;
	CurrentVelocityLength = 0.f;
	MapInRange.X = 10.f;
	MapInRange.Y = 1000.f;
	MapOutRange.X = 0.1f;
	MapOutRange.Y = 2.f;
	NumBuddies = 0;

	BuddySphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASTrackerBot::BeginOverlap);
	BuddySphereComp->OnComponentEndOverlap.AddDynamic(this, &ASTrackerBot::EndOverlap);
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> BotActorArray;
	if (TrackerBotClass)
	{
		UGameplayStatics::GetAllActorsOfClass(this, TrackerBotClass, BotActorArray);
		MaxNumBuddies = BotActorArray.Num() - 1;
	}
	if (Role == ROLE_Authority)
	{
		NextPoint = GetNextPathPoint();
	}
}

void ASTrackerBot::UpdateMaterial()
{
	if (!MatInstDyn)
	{
		MatInstDyn = MeshComp->CreateDynamicMaterialInstance(0, MeshComp->GetMaterial(0));
	}
	if (MatInstDyn && MaxNumBuddies > 0)
	{
		MatInstDyn->SetScalarParameterValue("PowerLevelAlpha", ((float)NumBuddies / (float)MaxNumBuddies));
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || USHealthComp::IsFriendly(TestPawn, this))
		{
			continue;
		}
		USHealthComp* TestPawnHealthComp = Cast<USHealthComp>(TestPawn->GetComponentByClass(USHealthComp::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.f)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if (Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget)
	{

		UNavigationPath * NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);
		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);
		if (NavPath && NavPath->PathPoints.Num() > 1)
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
		if (ExplosionEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		}
		if (ExplosionSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());
		}

		MeshComp->SetVisibility(false, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Role == ROLE_Authority)
		{
			TArray<AActor*> IgnoredActors;
			IgnoredActors.Add(this);
			UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage * (NumBuddies + 1), GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
			//Destroy();
			SetLifeSpan(2.0f);
		}
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.f, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::UpdateMovement()
{
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

void ASTrackerBot::AdjustRollingVolume()
{
	if (AudioComp->Sound)
	{
		CurrentVelocityLength = GetVelocity().Size();
		ClampedVolumeMultiplier = FMath::GetMappedRangeValueClamped(MapInRange, MapOutRange, CurrentVelocityLength);
		AudioComp->SetVolumeMultiplier(ClampedVolumeMultiplier);
	}
}

void ASTrackerBot::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ASTrackerBot* OverlappedBot = Cast<ASTrackerBot>(OtherActor);
	if (OverlappedBot)
	{
		NumBuddies++;

		UpdateMaterial();
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, (TEXT("Num buddies: %s"), *FString::SanitizeFloat(NumBuddies)));
		}*/

	}
}

void ASTrackerBot::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASTrackerBot* OverlappedBot = Cast<ASTrackerBot>(OtherActor);
	if (OverlappedBot)
	{
		if (NumBuddies - 1 < 0)
		{
			NumBuddies = 0;
		}
		else
		{
			NumBuddies--;
		}
		UpdateMaterial();
	}
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, (TEXT("Num buddies: %s"), *FString::SanitizeFloat(NumBuddies)));
	}*/
}

void ASTrackerBot::RefreshPath()
{
	NextPoint = GetNextPathPoint();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Role == ROLE_Authority && !bExploded)
	{
		UpdateMovement();
	}
	AdjustRollingVolume();
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComp::IsFriendly(OtherActor, this))
		{
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ASTrackerBot::DamageSelf, 0.25f, true);
			}
			bStartedSelfDestruction = true;
			if (SelfDestructSound)
			{
				UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
			}
		}
	}
}
