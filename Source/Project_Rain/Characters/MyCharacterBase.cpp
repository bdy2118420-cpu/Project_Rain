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
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "NiagaraFunctionLibrary.h"

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

	ChargeVisualComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeVisual"));
	ChargeVisualComponent->SetupAttachment(GetMesh(), FName("hand_r"));
	ChargeVisualComponent->bAutoActivate = false;

	FlamethrowerLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlamethrowerLeft"));
	FlamethrowerLeft->SetupAttachment(GetRootComponent());
	FlamethrowerLeft->bAutoActivate = false;

	FlamethrowerRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlamethrowerRight"));
	FlamethrowerRight->SetupAttachment(GetRootComponent());
	FlamethrowerRight->bAutoActivate = false;

	SnapFreezeIndicator = CreateDefaultSubobject<UDecalComponent>(TEXT("SnapFreezeIndicator"));
	SnapFreezeIndicator->SetupAttachment(GetRootComponent());
	SnapFreezeIndicator->DecalSize = FVector(100.f, 200.f, 50.f);
	SnapFreezeIndicator->SetVisibility(false);

	
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
	if (HasAuthority() && ProjectileClassNanoBomb)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		for (int32 i = 0; i < MaxPoolSizeNanoBomb; ++i)
		{
			AMyProjectileBase* SpawnedProj = GetWorld()->SpawnActor<AMyProjectileBase>(
				ProjectileClassNanoBomb, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (SpawnedProj)
			{
				ProjectilePoolNanoBomb.Add(SpawnedProj);
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
		UIC->BindAction(SubAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::StopSubAttack);

		UIC->BindAction(UtilityAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartSnapFreeze);
		UIC->BindAction(UtilityAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::ExecuteSnapFreeze);

		UIC->BindAction(SpecialAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::StartFlamethrower);
		UIC->BindAction(SpecialAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::StopFlamethrower);
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

	if (bIsChargingNanoBomb && ChargeVisualComponent)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ChargeRatio = FMath::Clamp((CurrentTime - ChargeStartTime) / MaxChargeTime, 0.0f, 1.0f);

		float VisualScale = FMath::Lerp(1.0f, 2.0f, ChargeRatio);
		ChargeVisualComponent->SetWorldScale3D(FVector(VisualScale));
	}


	if (bIsAimingSnapFreeze && Camera && SnapFreezeIndicator)
	{
		FVector StartLocation = Camera->GetComponentLocation();
		FVector ForwardVector = Camera->GetForwardVector();
		FVector EndLocation = StartLocation + (ForwardVector * MaxSnapFreezeRange);

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this); 

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

		if (bHit)
		{
			SnapFreezeIndicator->SetWorldLocation(HitResult.ImpactPoint);

			SnapFreezeIndicator->SetWorldRotation(FRotator(-90.f, Camera->GetComponentRotation().Yaw, 0.f));
		}
		else
		{
			SnapFreezeIndicator->SetWorldLocation(EndLocation);
		}
	}

	if (bIsUsingFlamethrower && Camera)
	{
		FVector StartLocation = Camera->GetComponentLocation();
		FVector ForwardVector = Camera->GetForwardVector();

		float AimUpOffset = 50.f;

		FVector TargetLocation = StartLocation + (ForwardVector * 5000.f) + (Camera->GetUpVector() * AimUpOffset);;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, TargetLocation, ECC_Visibility, CollisionParams))
		{
			TargetLocation = HitResult.ImpactPoint;
		}

		if (FlamethrowerLeft)
		{
		
			FVector LeftHandLoc = GetMesh()->GetSocketLocation(FName("hand_l"));
			FRotator LeftLookAt = UKismetMathLibrary::FindLookAtRotation(LeftHandLoc, TargetLocation);

			FlamethrowerLeft->SetWorldLocation(LeftHandLoc); 
			FlamethrowerLeft->SetWorldRotation(LeftLookAt);  
		}

		if (FlamethrowerRight)
		{
			FVector RightHandLoc = GetMesh()->GetSocketLocation(FName("hand_r"));
			FRotator RightLookAt = UKismetMathLibrary::FindLookAtRotation(RightHandLoc, TargetLocation);

			FlamethrowerRight->SetWorldLocation(RightHandLoc);
			FlamethrowerRight->SetWorldRotation(RightLookAt);
		}
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
	if (!bIsNanoBombReady)
	{
		return;
	}

	bIsChargingNanoBomb = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	if (ChargeVisualComponent)
	{
		ChargeVisualComponent->SetVisibility(true);
		ChargeVisualComponent->SetWorldScale3D(FVector(1.0f));
		ChargeVisualComponent->Activate(true);
	}

	if (NanoBombChargeMontage)
	{
		PlayAnimMontage(NanoBombChargeMontage, 1.0f);
	}

	Server_StartNanoBombCharge();
}

void AMyCharacterBase::StopSubAttack()
{
	if (!bIsChargingNanoBomb || !bIsNanoBombReady)
	{
		return;
	}

	bIsChargingNanoBomb = false;
	bIsNanoBombReady = false; 

	if (ChargeVisualComponent)
	{
		ChargeVisualComponent->DeactivateImmediate();
		ChargeVisualComponent->SetVisibility(false);
	}


	if (NanoBombChargeMontage)
	{
		StopAnimMontage(NanoBombChargeMontage);
	}

	if (NanoBombFireMontage)
	{
		PlayAnimMontage(NanoBombFireMontage, 1.5f);
	}


	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ChargeDuration = CurrentTime - ChargeStartTime;
	float ChargeRatio = FMath::Clamp(ChargeDuration / MaxChargeTime, 0.0f, 1.0f);

	Server_FireNanoBomb(ChargeRatio);

	GetWorldTimerManager().SetTimer(NanoBombCooldownTimer, this, &AMyCharacterBase::RechargeNanoBomb, NanoBombCooldown, false);

	if (OnNanoBombCooldownStarted.IsBound())
	{
		OnNanoBombCooldownStarted.Broadcast(NanoBombCooldown);
	}
}

void AMyCharacterBase::RechargeNanoBomb()
{
	bIsNanoBombReady = true;

	if (OnNanoBombReady.IsBound())
	{
		OnNanoBombReady.Broadcast();
	}
}

void AMyCharacterBase::Multicast_FireNanoBomb_Implementation()
{
	if (IsLocallyControlled()) return;

	if (ChargeVisualComponent)
	{
		ChargeVisualComponent->DeactivateImmediate();
		ChargeVisualComponent->SetVisibility(false);
	}

	if (NanoBombChargeMontage)
	{
		StopAnimMontage(NanoBombChargeMontage);
	}

	if (NanoBombFireMontage)
	{
		PlayAnimMontage(NanoBombFireMontage, 1.5f);
	}
}

void AMyCharacterBase::Multicast_StartNanoBombCharge_Implementation()
{
	if (IsLocallyControlled())
	{
		return;
	}

	// 다른 사람들의 화면에서 내 캐릭터가 기를 모으는 모션과 이펙트를 켬
	if (ChargeVisualComponent)
	{
		ChargeVisualComponent->SetVisibility(true);
		ChargeVisualComponent->SetWorldScale3D(FVector(1.0f));
		ChargeVisualComponent->Activate(true);
	}

	if (NanoBombChargeMontage)
	{
		PlayAnimMontage(NanoBombChargeMontage, 1.0f);
	}
}

void AMyCharacterBase::Server_StartNanoBombCharge_Implementation()
{
	Multicast_StartNanoBombCharge();
}


void AMyCharacterBase::Server_FireNanoBomb_Implementation(float ChargeRatio)
{
	Multicast_FireNanoBomb();

	if (ProjectilePoolNanoBomb.Num() > 0)
	{
		AMyProjectileBase* ProjectileToFire = ProjectilePoolNanoBomb[PoolIndexNanoBomb];
		if (ProjectileToFire)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 70.f; // 양손 앞
			FRotator SpawnRotation = GetControlRotation();

			ProjectileToFire->SetChargeScale(ChargeRatio);
			ProjectileToFire->ActivateProjectile(SpawnLocation, SpawnRotation);
		}
		PoolIndexNanoBomb = (PoolIndexNanoBomb + 1) % MaxPoolSizeNanoBomb;
	}
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
			FVector CameraLocation = Camera->GetComponentLocation();
			FVector TargetLocation = CameraLocation + (Camera->GetForwardVector() * 5000.f);

			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TargetLocation, ECC_Visibility, CollisionParams))
			{
				TargetLocation = HitResult.ImpactPoint;
			}

			FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);

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

void AMyCharacterBase::StartSnapFreeze()
{
	if (!bIsSnapFreezReady)
	{
		return;
	}
	bIsAimingSnapFreeze = true;

	if (SnapFreezeIndicator)
	{
		SnapFreezeIndicator->SetVisibility(true);
	}
}

void AMyCharacterBase::ExecuteSnapFreeze()
{
	if (!bIsAimingSnapFreeze || !bIsSnapFreezReady)
	{
		return;
	}

	bIsAimingSnapFreeze = false;
	bIsSnapFreezReady = false;

	if (SnapFreezeIndicator)
	{
		SnapFreezeIndicator->SetVisibility(false);

		FVector SpawnLocation = SnapFreezeIndicator->GetComponentLocation();
		FRotator SpawnRotation = FRotator(0.f, SnapFreezeIndicator->GetComponentRotation().Yaw, 0.f);

		Server_ExecuteSnapFreeze(SpawnLocation, SpawnRotation);

		GetWorldTimerManager().SetTimer(SnapFreezCooldownTimer,this,&AMyCharacterBase::RechargeSnapFreez,SnapFreezCooldown,false);

		if (OnSnapFreezeCooldownStarted.IsBound())
		{
			OnSnapFreezeCooldownStarted.Broadcast(SnapFreezCooldown);
		}
	}
}

void AMyCharacterBase::RechargeSnapFreez()
{
	bIsSnapFreezReady = true;

	if (OnSnapFreezeReady.IsBound())
	{
		OnSnapFreezeReady.Broadcast();
	}
}
void AMyCharacterBase::Server_ExecuteSnapFreeze_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	Multicast_ExecuteSnapFreeze(SpawnLocation, SpawnRotation);
}

void AMyCharacterBase::Multicast_ExecuteSnapFreeze_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (SnapFreezeEffectAsset)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SnapFreezeEffectAsset, SpawnLocation, SpawnRotation);
	}
}
void AMyCharacterBase::StartFlamethrower()
{
	if (!bIsFlamethrowerReady)
	{
		return;
	}

	if (FlamethrowerMontage)
	{
		PlayAnimMontage(FlamethrowerMontage, 1.0f);
	}

	bIsUsingFlamethrower = true;

	if (FlamethrowerLeft)
	{
		FlamethrowerLeft->Activate(true);
	}
	if (FlamethrowerRight)
	{
		FlamethrowerRight->Activate(true);
	}

	GetWorldTimerManager().SetTimer(FlamethrowerDurationTimer, this, &AMyCharacterBase::StopFlamethrower, MaxFlamethrowerDuration, false);

	Server_StartFlamethrower();
}

void AMyCharacterBase::StopFlamethrower()
{
	if (!bIsUsingFlamethrower) return;

	bIsUsingFlamethrower = false;
	bIsFlamethrowerReady = false;

	GetWorldTimerManager().ClearTimer(FlamethrowerDurationTimer);

	if (FlamethrowerLeft)
	{
		FlamethrowerLeft->DeactivateImmediate();
		
	}
	if (FlamethrowerRight)
	{
		FlamethrowerRight->DeactivateImmediate();
	}

	if (FlamethrowerMontage)
	{
		StopAnimMontage(FlamethrowerMontage);
	}

	GetWorldTimerManager().SetTimer(FlamethrowerCooldownTimer,this,&AMyCharacterBase::RechargeFlamethrower,FlamethrowerCooldown,false);


	if (OnFlamethrowerCooldownStarted.IsBound())
	{
		OnFlamethrowerCooldownStarted.Broadcast(FlamethrowerCooldown);
	}

	if (IsLocallyControlled())
	{
		Server_StopFlamethrower();
	}
}

void AMyCharacterBase::RechargeFlamethrower()
{
	bIsFlamethrowerReady = true;

	if (OnFlamethrowerReady.IsBound())
	{
		OnFlamethrowerReady.Broadcast();
	}
}

void AMyCharacterBase::Server_StartFlamethrower_Implementation()
{
	Multicast_StartFlamethrower();
}

void AMyCharacterBase::Multicast_StartFlamethrower_Implementation()
{
	if (IsLocallyControlled()) return;

	if (FlamethrowerMontage) PlayAnimMontage(FlamethrowerMontage, 1.0f);
	bIsUsingFlamethrower = true;
	if (FlamethrowerLeft) FlamethrowerLeft->Activate(true);
	if (FlamethrowerRight) FlamethrowerRight->Activate(true);
}

void AMyCharacterBase::Server_StopFlamethrower_Implementation()
{
	Multicast_StopFlamethrower();
}

void AMyCharacterBase::Multicast_StopFlamethrower_Implementation()
{
	// 내 화면(Local)에서는 이미 껐으므로 무시
	if (IsLocallyControlled()) return;

	// 다른 사람의 캐릭터에서 불 뿜는 모션과 이펙트를 끔
	bIsUsingFlamethrower = false;
	if (FlamethrowerLeft) FlamethrowerLeft->DeactivateImmediate();
	if (FlamethrowerRight) FlamethrowerRight->DeactivateImmediate();
	if (FlamethrowerMontage) StopAnimMontage(FlamethrowerMontage);
}