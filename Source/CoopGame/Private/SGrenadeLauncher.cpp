// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"
#include "Kismet/GameplayStatics.h"
#include "SGrenade.h"
#include "Components/SkeletalMeshComponent.h"
#include "SCharacter.h"
#include "Components/StaticMeshComponent.h"

ASGrenadeLauncher::ASGrenadeLauncher()
{
	SocketName = "MuzzleFlashSocket";
}

void ASGrenadeLauncher::Fire()
{
	FRotator SpawnRot = MeshComp->GetSocketTransform(SocketName).GetRotation().Rotator();
	FTransform SpawnTransform = FTransform(SpawnRot, MeshComp->GetSocketLocation(SocketName));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TheOwner = Super::GetOwner();
	SpawnParams.Owner = TheOwner;
	ASGrenade* TheGrenade = GetWorld()->SpawnActor<ASGrenade>(GrenadeClass, SpawnTransform, SpawnParams);
	ASCharacter* ShooterPawn = Cast<ASCharacter>(TheOwner);
	if (TheGrenade && ShooterPawn)
	{
		FVector EyeLoc;
		FRotator EyeRot; 
		ShooterPawn->GetActorEyesViewPoint(EyeLoc, EyeRot);
		FVector ShotImpulseDirection = /*EyeRot.Vector() + */(EyeRot.Vector() * 1000.f);
		TheGrenade->MeshComp->AddImpulse(ShotImpulseDirection, NAME_None, true);
	}
}
