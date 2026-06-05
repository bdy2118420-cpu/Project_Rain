// Fill out your copyright notice in the Description page of Project Settings.


#include "MageAnimInstance.h"
#include "MyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMageAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MageCharacter = Cast<AMyCharacterBase>(TryGetPawnOwner());
}

void UMageAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (MageCharacter)
	{
		FVector Velocity = MageCharacter->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
		Direction = CalculateDirection(Velocity, MageCharacter->GetActorRotation());
		bIsInAir = (MageCharacter->GetCharacterMovement()->MovementMode == MOVE_Flying);
		bIsOnGround = MageCharacter->GetCharacterMovement()->IsMovingOnGround();
		bIsAiming = MageCharacter->bIsAiming;
	}


}
