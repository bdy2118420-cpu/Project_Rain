// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkillComponent.h"
#include "FlameBoltComponent.generated.h"

/**
 * 
 */
class AMyProjectileBase;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChangedSignature, int32, NewAmmo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_RAIN_API UFlameBoltComponent : public UBaseSkillComponent
{
	GENERATED_BODY()
public:
	UFlameBoltComponent();

	UPROPERTY(BlueprintAssignable, Category = "Skill|UI")
	FOnAmmoChangedSignature OnAmmoChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void StartSkill() override;

protected:
	// --- 탄창 시스템 ---
	UPROPERTY(EditAnywhere, Category = "FlameBolt|Stats")
	int32 MaxAmmo = 4;

	UPROPERTY(BlueprintReadOnly, Category = "FlameBolt|Stats")
	int32 CurrentAmmo = 4;

	UPROPERTY(EditAnywhere, Category = "FlameBolt|Stats")
	float RechargeTime = 1.3f;

	FTimerHandle RechargeTimerHandle;
	void RechargeAmmo();

	UFUNCTION(BlueprintCallable, Category = "FlameBolt")
	float GetRechargeProgress() const;

	// --- 애니메이션 & 풀링 ---
	UPROPERTY(EditAnywhere, Category = "FlameBolt|Animation")
	TObjectPtr<UAnimMontage> ShootLeftMontage;

	UPROPERTY(EditAnywhere, Category = "FlameBolt|Animation")
	TObjectPtr<UAnimMontage> ShootRightMontage;

	bool bIsRightHandNext = false;
	FTimerHandle AimingTimerHandle;
	void StopAiming();

	UPROPERTY(EditDefaultsOnly, Category = "FlameBolt|Projectile")
	TSubclassOf<AMyProjectileBase> ProjectileClass;

	UPROPERTY()
	TArray<TObjectPtr<AMyProjectileBase>> ProjectilePool;

	int32 PoolIndex = 0;
	UPROPERTY(EditDefaultsOnly, Category = "FlameBolt|Projectile")
	int32 MaxPoolSize = 6;

	UFUNCTION(Server, Reliable)
	void Server_PlayAttack(bool bIsRight);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttack(bool bIsRight);
};
