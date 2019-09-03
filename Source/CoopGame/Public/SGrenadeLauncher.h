// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenade.h"
#include "SGrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()
public:
	ASGrenadeLauncher();
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ASGrenade> GrenadeClass;

	FName SocketName;
public:

	virtual void Fire() override;

};
