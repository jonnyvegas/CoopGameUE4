// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenade.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "SDummy.h"

// Sets default values
ASGrenade::ASGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetWorldScale3D(FVector(0.1f, 0.1f, 0.1f));
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetSphereRadius(200.f);

}

// Called when the game starts or when spawned
void ASGrenade::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ASGrenade::Explode, 1.0f);
}

void ASGrenade::Explode()
{
	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation());
	}
	TArray<AActor*> IgnoredActorArray;
	IgnoredActorArray.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, 100.f, GetActorLocation(), 100.f, DamageType, IgnoredActorArray, this);
	SphereComp->GetOverlappingActors(OverlappedActorArray, DummySubclass);
	for (AActor* TheActor : OverlappedActorArray)
	{
		ASDummy* DummyActor = Cast<ASDummy>(TheActor);
		if (DummyActor)
		{
			DummyActor->SkeletalMeshComp->AddRadialImpulse(SphereComp->GetComponentLocation(), SphereComp->GetUnscaledSphereRadius(), 1000.f, ERadialImpulseFalloff::RIF_Constant, true);
		}
		//TheActor->GetRootComponent()->
			//->AddRadialImpulse(SphereComp->GetComponentLocation(), SphereComp->GetUnscaledSphereRadius(), 1000.f, ERadialImpulseFalloff::RIF_Constant, true);
	}
	Destroy();
}

// Called every frame
void ASGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

