// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USHealthComp* HealthComp;

	UPROPERTY()
	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0, ClampMax = 100))
	float ZoomInterpSpeed;

	UPROPERTY()
	float DefaultFOV;

	UFUNCTION()
	void BeginZoom();

	UFUNCTION()
	void EndZoom();

	UPROPERTY(Replicated)
	class ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> WeaponClass;

	UPROPERTY(EditDefaultsOnly)
	FName AttachSocketName;

	UFUNCTION()
	void Fire();

	
	UFUNCTION()
	void Reload();

	UFUNCTION()
	void OnHealthChanged(USHealthComp* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Player")
	bool bPawnDied;

	

	UFUNCTION()
	void CheckJump();

	UFUNCTION()
	void EndJump();

public:	

	UPROPERTY(BlueprintReadWrite)
	bool bCanDoubleJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	uint8 NumJumpsAllowed;

	UPROPERTY(BlueprintReadOnly)
	uint8 NumJumpsSoFar;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	UFUNCTION(BlueprintCallable)
	void StartFire();

	UFUNCTION(BlueprintCallable)
	void StopFire();


	/** Returns	Pawn's eye location */
	virtual FVector GetPawnViewLocation() const override;

	virtual void Landed(const FHitResult& Hit) override;
};
