// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <EnhancedInputLibrary.h>

#include "InputAction.h"
#include "MyCharacterBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmoChangedDelegate, int32, NewAmmo);

class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;

class AMyProjectileBase;

UCLASS()
class PROJECT_RAIN_API AMyCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacterBase();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Ammo")
	FOnAmoChangedDelegate OnAmmoChanged;

protected:
	virtual void BeginPlay() override;

	// 평소 속도와 가속 속도
	const float WalkSpeed = 300.f;
	const float SprintSpeed = 600.f;

	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DecelerationInertia = 5.0f;


	virtual void Tick(float DeltaTime) override;

	void StartSprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(bool bNewSprint);

	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spring")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HoverAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SubAttackAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);


	void HoverUp(const FInputActionValue& Value);
	void StartHover();
	void StopHover();


	//Attack
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false;

	UFUNCTION(Server, Reliable)
	void Server_PlayAttack(bool bIsRight);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttack(bool bIsRight);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class UAnimMontage> ShootLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class UAnimMontage> ShootRightMontage;



	bool bIsRightHandNext = false;


	FTimerHandle AimingTimerHandle;

	void StartAttack();
	void StopAiming();

	//bolt
	int32 MaxAmmo = 4;
	int32 CurrentAmmo = 4;
	float RechargeTime = 1.3f;
	FTimerHandle RechargeTimerHandle;

	void RechargeAmmo();

	UFUNCTION(BlueprintCallable)
	float GetRechargeProgress() const;


	UPROPERTY(EditDefaultsOnly, Category = "Combat|Projectile")
	TSubclassOf<AMyProjectileBase> ProjectileClassBolt;


	UPROPERTY()
	TArray<TObjectPtr<AMyProjectileBase>> ProjectilePoolBolt;


	int32 PoolIndexBolt = 0;

	const int32 MaxPoolSizeBolt = 6;

	//nanobomb
	

	void StartSubAttack();
	void StopSubAttack(); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|NanoBomb")
	float MaxChargeTime = 3.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|NanoBomb")
	float NanoBombCooldown = 5.0f; 

	bool bIsChargingNanoBomb = false;
	bool bIsNanoBombReady = true;   
	float ChargeStartTime = 0.0f;   

	FTimerHandle NanoBombCooldownTimer;
	void RechargeNanoBomb(); 

	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Projectile")
	TSubclassOf<AMyProjectileBase> ProjectileClassNanoBomb;

	UPROPERTY()
	TArray<TObjectPtr<AMyProjectileBase>> ProjectilePoolNanoBomb;

	int32 PoolIndexNanoBomb = 0;
	int32 MaxPoolSizeNanoBomb = 2;

	// 서버 발사 RPC (차지 비율을 서버로 같이 넘깁니다)
	UFUNCTION(Server, Reliable)
	void Server_FireNanoBomb(float ChargeRatio);


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> ChargeVisualComponent;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;
};
