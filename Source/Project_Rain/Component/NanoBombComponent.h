// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkillComponent.h"
#include "NanoBombComponent.generated.h"

/**
 * 
 */
class AMyProjectileBase;
class UAnimMontage;


UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent) )
class PROJECT_RAIN_API UNanoBombComponent : public UBaseSkillComponent
{
	GENERATED_BODY()
public:
	UNanoBombComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void StartSkill() override;
	virtual void StopSkill() override;
	virtual void ExecuteSkill() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "NanoBomb|Projectile")
	TSubclassOf<AMyProjectileBase> ProjectileClass;

	UPROPERTY()
	TArray<TObjectPtr<AMyProjectileBase>> ProjectilePool;

	int32 PoolIndex = 0;

	UPROPERTY(EditDefaultsOnly, Category = "NanoBomb|Projectile")
	int32 MaxPoolSize = 2;

	UPROPERTY(EditAnywhere, Category = "NanoBomb|Stats")
	float MaxChargeTime = 3.0f;

	bool bIsCharging = false;
	float ChargeStartTime = 0.0f;
	float CurrentChargeRatio = 0.0f;

	UPROPERTY(EditAnywhere, Category = "NanoBomb|Animation")
	TObjectPtr<UAnimMontage> ChargeMontage;

	UPROPERTY(EditAnywhere, Category = "NanoBomb|Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	UFUNCTION(Server, Reliable)
	void Server_StartCharge();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartCharge();

	UFUNCTION(Server, Reliable)
	void Server_Fire(float ChargeRatio);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire();
};
