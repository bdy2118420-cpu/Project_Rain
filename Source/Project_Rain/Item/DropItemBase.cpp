#include "DropItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

#include "../Core/MyGameState.h" 
#include "../Core/MyPlayerState.h"   

ADropItemBase::ADropItemBase()
{

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(50.0f);
	SphereComp->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SphereComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bShouldBounce = true; 
	ProjectileMovementComp->Bounciness = 0.5f;    
	ProjectileMovementComp->Friction = 0.3f;      
	ProjectileMovementComp->ProjectileGravityScale = 1.0f;
}

void ADropItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADropItemBase, ItemType);
	DOREPLIFETIME(ADropItemBase, ItemAmount);
}

void ADropItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ADropItemBase::OnSphereOverlap);
	}
}

void ADropItemBase::InitializeDrop(EDropItemType NewType, float NewAmount)
{
	if (HasAuthority())
	{
		ItemType = NewType;
		ItemAmount = NewAmount;

		FVector PopUpVelocity = FVector(FMath::RandRange(-200.f, 200.f), FMath::RandRange(-200.f, 200.f), FMath::RandRange(400.f, 600.f));
		ProjectileMovementComp->Velocity = PopUpVelocity;
	}
}

void ADropItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (PlayerCharacter && PlayerCharacter->ActorHasTag(TEXT("Player")))
	{
		if (ItemType == EDropItemType::Gold)
		{
			AMyPlayerState* PS = PlayerCharacter->GetPlayerState<AMyPlayerState>();
			if (PS) PS->AddGold(FMath::TruncToInt(ItemAmount));
		}
		else if (ItemType == EDropItemType::EXP)
		{
			AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
			if (GS) GS->AddGlobalEXP(ItemAmount);
		}

		Destroy();
	}
}