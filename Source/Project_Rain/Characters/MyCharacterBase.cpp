#include "MyCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"

#include "../Component/FlameBoltComponent.h"
#include "../Component/NanoBombComponent.h"
#include "../Component/SnapFreezeComponent.h"
#include "../Component/FlameThrowerComponent.h"

AMyCharacterBase::AMyCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 70.f);
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	GetMesh()->SetRelativeLocation(FVector(0.f, -20.f, -50.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
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

	FlameBoltComp = CreateDefaultSubobject<UFlameBoltComponent>(TEXT("FlameBoltComponent"));
	NanoBombComp = CreateDefaultSubobject<UNanoBombComponent>(TEXT("NanoBombComponent"));
	SnapFreezeComp = CreateDefaultSubobject<USnapFreezeComponent>(TEXT("SnapFreezeComponent"));
	FlamethrowerComp = CreateDefaultSubobject<UFlameThrowerComponent>(TEXT("FlamethrowerComponent"));
}

void AMyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	float CurrentMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (!FMath::IsNearlyEqual(CurrentMaxSpeed, TargetSpeed, 0.1f))
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(CurrentMaxSpeed, TargetSpeed, DeltaTime, DecelerationInertia);
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	}

}

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

		// 입력 신호를 스킬 래퍼 함수로 연결
		UIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::Input_StartAttack);
		UIC->BindAction(SubAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::Input_StartSubAttack);
		UIC->BindAction(SubAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::Input_StopSubAttack);
		UIC->BindAction(UtilityAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::Input_StartUtility);
		UIC->BindAction(UtilityAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::Input_ExecuteUtility);
		UIC->BindAction(SpecialAttackAction, ETriggerEvent::Started, this, &AMyCharacterBase::Input_StartSpecial);
		UIC->BindAction(SpecialAttackAction, ETriggerEvent::Completed, this, &AMyCharacterBase::Input_StopSpecial);
	}
}


void AMyCharacterBase::Input_StartAttack()
{
	if (FlameBoltComp)
	{
		FlameBoltComp->StartSkill();
	}
}

void AMyCharacterBase::Input_StartSubAttack()
{
	if (NanoBombComp)
	{
		NanoBombComp->StartSkill();
	}
}
void AMyCharacterBase::Input_StopSubAttack()
{
	if (NanoBombComp)
	{
		NanoBombComp->StopSkill();
	}
}

void AMyCharacterBase::Input_StartUtility() 
{ 
	if (SnapFreezeComp)
	{
		SnapFreezeComp->StartSkill();
	}
}
void AMyCharacterBase::Input_ExecuteUtility() 
{
	if (SnapFreezeComp)
	{
		SnapFreezeComp->ExecuteSkill();
	}
}

void AMyCharacterBase::Input_StartSpecial()
{ 
	if (FlamethrowerComp)
	{
		FlamethrowerComp->StartSkill();
	}
}
void AMyCharacterBase::Input_StopSpecial()
{ 
	if (FlamethrowerComp)
	{
		FlamethrowerComp->StopSkill();
	}
}


void AMyCharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MovementVector.Y);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MovementVector.X);
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
	if (CanJump()) Jump();
	else if (GetCharacterMovement()->IsFalling()) GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AMyCharacterBase::HoverUp(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling()) GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	if (GetCharacterMovement()->MovementMode == MOVE_Flying) AddMovementInput(FVector::UpVector, 0.5f);
}

void AMyCharacterBase::StopHover()
{
	StopJumping();
	if (GetCharacterMovement()->MovementMode == MOVE_Flying) GetCharacterMovement()->SetMovementMode(MOVE_Falling);
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