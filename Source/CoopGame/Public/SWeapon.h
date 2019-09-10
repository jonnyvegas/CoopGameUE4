// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class UDamageType> DmgType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	class UParticleSystem* MuzzleFlashParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* TraceSmokeParticle;

	UPROPERTY(VisibleDefaultsOnly)
	FName MuzzleSocketName;

	UPROPERTY()
	FVector TracerEndPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDmg;
	
	UFUNCTION()
	void PlayFireEffects(FVector EndPoint);

	UFUNCTION()
	void PlayImpactEffects(EPhysicalSurface ImpactSurfaceType, FVector ImpactPoint);

	virtual void Fire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();
	void ServerFire_Implementation();
	bool ServerFire_Validate();

	UFUNCTION()
	void Reload();

	FTimerHandle TimerHandle_Reload;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireRate;

	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 CurrentAmmoAmount;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 ClipSize;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 RemainingAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 TotalAmmo;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:	
	

	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void StartReload();

	UPROPERTY()
	bool bReloading;
};
