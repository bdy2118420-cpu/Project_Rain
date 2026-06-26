#include "PlayerStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerStatComponent::UPlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


	SetIsReplicatedByDefault(true);
}

void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStatComponent, DamageMultiplier);
	DOREPLIFETIME(UPlayerStatComponent, MoveSpeedMultiplier);
	DOREPLIFETIME(UPlayerStatComponent, MaxHealthMultiplier);
	DOREPLIFETIME(UPlayerStatComponent, CooldownReduction);
}

void UPlayerStatComponent::AddDamageMultiplier(float Amount)
{

	if (!GetOwner()->HasAuthority()) return;

	DamageMultiplier += Amount;
}

void UPlayerStatComponent::AddMoveSpeedMultiplier(float Amount)
{
	if (!GetOwner()->HasAuthority()) return;

	MoveSpeedMultiplier += Amount;


	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
	{
	
		float BaseWalkSpeed = 300.0f;
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * MoveSpeedMultiplier;
	}
}

void UPlayerStatComponent::AddCooldownReduction(float Amount)
{
	if (!GetOwner()->HasAuthority()) return;


	CooldownReduction = FMath::Clamp(CooldownReduction + Amount, 0.0f, 0.7f);
}