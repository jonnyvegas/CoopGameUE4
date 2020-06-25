// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USHealthComp* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UAudioComponent* AudioComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* BuddySphereComp;

	UPROPERTY()
	FVector NextPoint;

	UPROPERTY()
	FVector ForceDir;

	UPROPERTY()
	float DistanceToTarget;

	UPROPERTY(EditDefaultsOnly)
	float MovementForce;

	UPROPERTY(EditDefaultsOnly)
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly)
	bool bUseVelocityChange;

	UPROPERTY()
	UMaterialInstanceDynamic* MatInstDyn;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* ExplosionEffect;

	UPROPERTY()
	bool bExploded;

	UPROPERTY()
	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly)
	class USoundBase* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* ExplosionSound;

	UPROPERTY()
	float CurrentVelocityLength;

	UPROPERTY()
	FVector2D MapInRange;

	UPROPERTY()
	FVector2D MapOutRange;

	UPROPERTY()
	float ClampedVolumeMultiplier;

	UPROPERTY(BlueprintReadOnly)
	uint8 NumBuddies;

	UPROPERTY(BlueprintReadOnly)
	uint8 MaxNumBuddies;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ASTrackerBot> TrackerBotClass;

	UPROPERTY()
	FTimerHandle TimerHandle_RefreshPath;

	UFUNCTION()
	void UpdateMaterial();

	UFUNCTION()
	FVector GetNextPathPoint();

	UFUNCTION()
	void HandleTakeDamage(USHealthComp* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void SelfDestruct();

	UPROPERTY()
	FTimerHandle TimerHandle_DamageSelf;

	UFUNCTION()
	void DamageSelf();

	UFUNCTION()
	void UpdateMovement();

	UFUNCTION()
	void AdjustRollingVolume();

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void RefreshPath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
