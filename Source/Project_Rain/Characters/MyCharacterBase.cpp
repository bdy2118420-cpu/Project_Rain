// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "../Projectiles/MyProjectileBase.h"


// Sets default values
AMyCharacterBase::AMyCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 70.f);
	SpringArm->bUsePawnControlRotation = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);


	GetMesh()->SetRelativeLocation(FVector(0.f, -20.f, -50.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -50.f, 0.f));
	GetCapsuleComponent()->SetCapsuleHalfHeight(70.f);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// Called when the game starts or when spawned
void AMyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	//오브젝트 풀링
	if (HasAuthority() && ProjectileClassBolt)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		for (int32 i = 0; i < MaxPoolSizeBolt; ++i)
		{
			
			AMyProjectileBase* SpawnedProj = GetWorld()->SpawnActor<AMyProjectileBase>(
				ProjectileClassBolt,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (SpawnedProj)
			{
				ProjectilePoolBolt.Add(SpawnedProj);
			}
		}
	}
}


// Called to bind functionality to input
void AMyCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	UEnhancedInputComponent* UIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (UIC)
	{
		UIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacterBase::Move);
		UIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacterBase::Look);
		UIC->BindAction(HoverAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartHover);
		UIC->BindAction(HoverAction, ETriggerEvent::Triggered, this, &AMyCharacterBase::HoverUp);
		UIC->BindAction(HoverAction, ETriggerEvent::Completed, this, &AMyCharacterBase::StopHover);

		UIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartSprint);
		UIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMyCharacterBase::StopSprint);

		UIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartAttack);

		UIC->BindAction(SubAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartSubAttack);
	}

}

void AMyCharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	float CurrentMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (!FMath::IsNearlyEqual(CurrentMaxSpeed, TargetSpeed, 0.1f))
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(
			CurrentMaxSpeed,
			TargetSpeed,
			DeltaTime,
			DecelerationInertia
		);
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	}
}


void AMyCharacterBase::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput((LookAxisVector.X) * 0.3f);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMyCharacterBase::StartHover()
{
	if (CanJump())
	{
		Jump();
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
}

void AMyCharacterBase::HoverUp(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
	
	if (GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		AddMovementInput(FVector::UpVector, 0.5f);
	}
}

void AMyCharacterBase::StopHover()
{
	StopJumping();

	if (GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void AMyCharacterBase::StartSprint()
{
	bIsSprinting = true;
	Server_SetSprinting(true);
}

void AMyCharacterBase::StopSprint()
{
	bIsSprinting = false;
	Server_SetSprinting(false); 
}


void AMyCharacterBase::Server_SetSprinting_Implementation(bool bNewSprint)
{
	bIsSprinting = bNewSprint;
}

void AMyCharacterBase::StartAttack()
{
	if (CurrentAmmo <= 0)
	{
		return;
	}

	CurrentAmmo--;
	OnAmmoChanged.Broadcast(CurrentAmmo);

	bIsAiming = true;
	GetWorldTimerManager().ClearTimer(AimingTimerHandle);

	UAnimMontage* TargetMontage = bIsRightHandNext ? ShootRightMontage : ShootLeftMontage;
	if (TargetMontage)
	{
		if (TargetMontage == ShootLeftMontage)
		{
			PlayAnimMontage(TargetMontage, 2.0f);
		}
		else
		{
			PlayAnimMontage(TargetMontage, 1.0f);
		}
		
	}
	Server_PlayAttack(bIsRightHandNext);

	bIsRightHandNext = !bIsRightHandNext;

	GetWorldTimerManager().SetTimer(AimingTimerHandle, this, &AMyCharacterBase::StopAiming, 1.5f, false);

	if (!GetWorldTimerManager().IsTimerActive(RechargeTimerHandle))
	{
		GetWorldTimerManager().SetTimer(RechargeTimerHandle, this, &AMyCharacterBase::RechargeAmmo, RechargeTime, true);
	}
}

void AMyCharacterBase::StopAiming()
{
	bIsAiming = false;
}

void AMyCharacterBase::StartSubAttack()
{
}

void AMyCharacterBase::Server_PlayAttack_Implementation(bool bIsRight)
{
	Multicast_PlayAttack(bIsRight);

	if (ProjectilePoolBolt.Num() > 0)
	{
		AMyProjectileBase* ProjectileToFire = ProjectilePoolBolt[PoolIndexBolt];

		if (ProjectileToFire)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f;
			FRotator SpawnRotation = GetControlRotation();

			ProjectileToFire->ActivateProjectile(SpawnLocation, SpawnRotation);
		}

		PoolIndexBolt = (PoolIndexBolt + 1) % MaxPoolSizeBolt;
	}
}

void AMyCharacterBase::Multicast_PlayAttack_Implementation(bool bIsRight)
{
	if (IsLocallyControlled())
	{
		return;
	}

	bIsAiming = true;
	GetWorldTimerManager().ClearTimer(AimingTimerHandle);

	UAnimMontage* TargetMontage = bIsRight ? ShootRightMontage : ShootLeftMontage;
	if (TargetMontage)
	{
		PlayAnimMontage(TargetMontage, 1.0f);
	}

	GetWorldTimerManager().SetTimer(AimingTimerHandle, this, &AMyCharacterBase::StopAiming, 1.5f, false);
}

void AMyCharacterBase::RechargeAmmo()
{
	if (CurrentAmmo < MaxAmmo)
	{
		CurrentAmmo++;
		OnAmmoChanged.Broadcast(CurrentAmmo);
	}

	if (CurrentAmmo >= MaxAmmo)
	{
		GetWorldTimerManager().ClearTimer(RechargeTimerHandle);
	}
}

float AMyCharacterBase::GetRechargeProgress() const
{
	if (CurrentAmmo >= MaxAmmo)
	{
		return 0.0f;
	}

	if (GetWorldTimerManager().IsTimerActive(RechargeTimerHandle))
	{
		float RemainingTime = GetWorldTimerManager().GetTimerRemaining(RechargeTimerHandle);
		return RemainingTime / RechargeTime;
	}
	return 0.0f;
}