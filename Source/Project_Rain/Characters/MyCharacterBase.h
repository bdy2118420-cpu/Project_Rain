// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <EnhancedInputLibrary.h>

#include "InputAction.h"
#include "MyCharacterBase.generated.h"



class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;
class UDecalComponent;
class UHealthComponent;

// 우리가 만든 스킬 컴포넌트들 전방 선언
class UFlameBoltComponent;
class UNanoBombComponent;
class USnapFreezeComponent;
class UFlameThrowerComponent;

UCLASS()
class PROJECT_RAIN_API AMyCharacterBase : public ACharacter
{
	GENERATED_BODY()
public:
	AMyCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	const float WalkSpeed = 300.f;
	const float SprintSpeed = 600.f;
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float DecelerationInertia = 5.0f;

	void StartSprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(bool bNewSprint);

	// --- 카메라 & 입력 ---
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> UtilityAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SpecialAttackAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void HoverUp(const FInputActionValue& Value);
	void StartHover();
	void StopHover();

	// --- 시각적 이펙트 컴포넌트 (스킬 컴포넌트들이 가져다 쓸 껍데기들) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ChargeVisualComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UNiagaraComponent> FlamethrowerLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UNiagaraComponent> FlamethrowerRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDecalComponent> SnapFreezeIndicator;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComp;


	// 공통 변수 (애니메이션 등에서 참조)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;

	// --- 스킬 컴포넌트 부착 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
	TObjectPtr<UFlameBoltComponent> FlameBoltComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
	TObjectPtr<UNanoBombComponent> NanoBombComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
	TObjectPtr<USnapFreezeComponent> SnapFreezeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills")
	TObjectPtr<UFlameThrowerComponent> FlamethrowerComp;

protected:
	// 스킬 키 입력과 컴포넌트를 연결해줄 래퍼(Wrapper) 함수들
	void Input_StartAttack();

	void Input_StartSubAttack();
	void Input_StopSubAttack();

	void Input_StartUtility();
	void Input_ExecuteUtility();

	void Input_StartSpecial();
	void Input_StopSpecial();
};
