// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "SCharacter.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponDrawing
(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug lines for weapons"),
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MuzzleSocketName = "MuzzleFlashSocket";
	BaseDmg = 20.f;
	FireRate = 600.f;
	TotalAmmo = 100;
	ClipSize = 20;
	CurrentAmmoAmount = ClipSize;
	RemainingAmmo = TotalAmmo - CurrentAmmoAmount;
}

void ASWeapon::Fire()
{
	// Trace from the socket of the weapon deal-y (or pawn eyes) to crosshair location in
	// center of screen.
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		if (CurrentAmmoAmount > 0)
		{
			FVector EyeLoc;
			FRotator EyeRot;
			GetOwner()->GetActorEyesViewPoint(EyeLoc, EyeRot);
			FVector ShotDirection = EyeRot.Vector();

			FVector TraceEnd = EyeLoc + (ShotDirection * 10000.f);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			TracerEndPoint = TraceEnd;

			FHitResult TheHit;
			// Did we hit anything?
			if (GetWorld()->LineTraceSingleByChannel(TheHit, EyeLoc, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
				if (DebugWeaponDrawing > 0)
				{
					DrawDebugLine(GetWorld(), EyeLoc, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
				}
				// HA! Time to do some damage!
				AActor* HitActor = TheHit.GetActor();
				if (HitActor)
				{
					EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(TheHit.PhysMaterial.Get());
					float ActualDamage = BaseDmg;
					if (SurfaceType == SURFACE_FLESHVULNERABLE)
					{
						ActualDamage *= 4.f;
					}
					TracerEndPoint = TheHit.ImpactPoint;
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, TheHit, MyOwner->GetInstigatorController(), this, DmgType);
					if (DefaultImpactEffect)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultImpactEffect, TheHit.ImpactPoint, TheHit.ImpactNormal.Rotation());
					}


					UParticleSystem* SelectedEffect = nullptr;
					switch (SurfaceType)
					{
					case SurfaceType_Default:
						break;
					case SURFACE_FLESHDEFAULT:
					case SURFACE_FLESHVULNERABLE:
						SelectedEffect = FleshImpactEffect;
						break;
					default:
						SelectedEffect = DefaultImpactEffect;
						break;
					}

					if (SelectedEffect)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, TheHit.ImpactPoint, TheHit.ImpactNormal.Rotation());
					}
				}
			}

			PlayFireEffects();
			LastFireTime = GetWorld()->GetTimeSeconds();
			CurrentAmmoAmount--;
		}
		// else play a you-don't-have-ammo sound.
	}
}

void ASWeapon::StartReload()
{
	GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &ASWeapon::Reload, 2.5);
	bReloading = true;
}

void ASWeapon::Reload()
{
	if (RemainingAmmo > 0)
	{
		if (RemainingAmmo > 20)
		{
			RemainingAmmo -= ClipSize;
			CurrentAmmoAmount = ClipSize;
		}
		else
		{
			CurrentAmmoAmount = ClipSize;
			ClipSize = 0;
		}
	}
	bReloading = false;
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	//Fire();
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / FireRate;
}

void ASWeapon::PlayFireEffects()
{
	if (MuzzleFlashParticle)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlashParticle, MeshComp, MuzzleSocketName);
	}
	if (TraceSmokeParticle)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceSmokeParticle, MeshComp->GetSocketLocation(MuzzleSocketName));
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", TracerEndPoint);
		}
	}

	ASCharacter* MyOwner = Cast<ASCharacter>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
		float RandPitch = FMath::RandRange(-1.f, -0.1f);
		float RandYaw = FMath::RandRange(-0.5f, 0.5f);
		MyOwner->AddControllerPitchInput(RandPitch);
		MyOwner->AddControllerYawInput(RandYaw);
	}
}