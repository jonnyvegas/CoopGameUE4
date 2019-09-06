// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplodingBarrel.h"
#include "Components/SHealthComp.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplodingBarrel::ASExplodingBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComp = CreateDefaultSubobject<USHealthComp>(TEXT("HealthComp"));
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = StaticMeshComp;
	StaticMeshComp->SetSimulatePhysics(true);
	StaticMeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	ExplosionHeight = 10.f;
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->ImpulseStrength = 100.f;
	RadialForceComp->Radius = 200.f;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bIgnoreOwningActor = true;
	//StaticMeshComp->SetupAttachment(RootComponent);
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplodingBarrel::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplodingBarrel::HealthChanged);
}

void ASExplodingBarrel::HealthChanged(USHealthComp* HealthCompCurrent, float Health, float HealthDelta, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f && !bExploded)
	{
		bExploded = true;
		if (Role == ROLE_Authority)
		{
			OnRep_Exploded();
		}
		Explode();
	}
}

void ASExplodingBarrel::Explode()
{
	// Launch up
	FVector ExplosionDirection = FVector(0.f, 0.f, ExplosionHeight);
	StaticMeshComp->AddImpulse(ExplosionDirection, NAME_None, true);
	// Radial force push nearby actors who need to be simulating physics or they won't do jack.
	RadialForceComp->FireImpulse();
}

void ASExplodingBarrel::OnRep_Exploded()
{
	// Play explosion particle
	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionParticle, GetActorLocation());

	}
	// Change material
	if (ExplodedMaterial)
	{
		StaticMeshComp->SetMaterial(0, ExplodedMaterial);
	}
}

// Called every frame
void ASExplodingBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASExplodingBarrel ::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplodingBarrel, bExploded);
}
