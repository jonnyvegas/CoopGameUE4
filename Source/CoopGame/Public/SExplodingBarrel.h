// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplodingBarrel.generated.h"

UCLASS()
class COOPGAME_API ASExplodingBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplodingBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(VisibleDefaultsOnly, Category = "Health")
	class USHealthComp* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class UStaticMeshComponent* StaticMeshComp;

	//UPROPERTY(EditDefaultsOnly, Category = "Materials")
	//class UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionHeight;

	UPROPERTY(VisibleDefaultsOnly)
	class URadialForceComponent* RadialForceComp;

	UFUNCTION()
	void HealthChanged(USHealthComp* HealthCompCurrent, float Health, float HealthDelta, const class UDamageType* DamageType, 
	class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void Explode();

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
