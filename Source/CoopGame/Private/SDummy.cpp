// Fill out your copyright notice in the Description page of Project Settings.


#include "SDummy.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASDummy::ASDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetSimulatePhysics(true);
	SkeletalMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComp->SetGenerateOverlapEvents(true);
	SkeletalMeshComp->SetCollisionObjectType(ECC_Pawn);
	RootComponent = SkeletalMeshComp;
}

// Called when the game starts or when spawned
void ASDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

