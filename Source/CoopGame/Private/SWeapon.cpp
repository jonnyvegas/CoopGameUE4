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
#include "Net/UnrealNetwork.h"

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
	SetReplicates(true);
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.0f;
	BulletSpread = 2.0f;
}

void ASWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
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

			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

			FVector TraceEnd = EyeLoc + (ShotDirection * 10000.f);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			TracerEndPoint = TraceEnd;

			FHitResult TheHit;

			EPhysicalSurface SurfaceType = SurfaceType_Default;
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
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(TheHit.PhysMaterial.Get());
					float ActualDamage = BaseDmg;
					if (SurfaceType == SURFACE_FLESHVULNERABLE)
					{
						ActualDamage *= 4.f;
					}
					PlayImpactEffects(SurfaceType, TheHit.ImpactPoint);
					TracerEndPoint = TheHit.ImpactPoint;
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, TheHit, MyOwner->GetInstigatorController(), MyOwner, DmgType);
				}
			}
			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
			}
			PlayFireEffects(HitScanTrace.TraceTo);
			LastFireTime = GetWorld()->GetTimeSeconds();
			CurrentAmmoAmount--;
		}
		// else play a you-don't-have-ammo sound.
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
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
			RemainingAmmo -= ClipSize - CurrentAmmoAmount;
			CurrentAmmoAmount = ClipSize;
		}
		else
		{
			CurrentAmmoAmount = RemainingAmmo;
			ClipSize = 0;
		}
	}
	bReloading = false;
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
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

void ASWeapon::PlayFireEffects(FVector EndPoint)
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
			TracerComp->SetVectorParameter("Target", EndPoint);
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

void ASWeapon::PlayImpactEffects(EPhysicalSurface ImpactSurfaceType, FVector ImpactPoint)
{

	UParticleSystem* SelectedEffect = nullptr;
	switch (ImpactSurfaceType)
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
	//if (DefaultImpactEffect)
	//{
	//	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultImpactEffect, TheHit.ImpactPoint, TheHit.ImpactNormal.Rotation());
	//}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}