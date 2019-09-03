// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGrenade.generated.h"

UCLASS()
class COOPGAME_API ASGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGrenade();

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY()
	FTimerHandle TimerHandle_Explode;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ExplosionParticle;

	UPROPERTY()
	TArray<AActor*> OverlappedActorArray;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> DummySubclass;

	UFUNCTION()
	void Explode();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
