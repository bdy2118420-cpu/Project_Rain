#include "SnapFreezeArea.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ASnapFreezeArea::ASnapFreezeArea()
{
	PrimaryActorTick.bCanEverTick = false;

	
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));


	CollisionBox->SetBoxExtent(FVector(100.f, 200.f, 50.f));

	NiagaraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraEffect"));
	NiagaraEffect->SetupAttachment(RootComponent);
}

void ASnapFreezeArea::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
	
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASnapFreezeArea::OnOverlapBegin);
		CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ASnapFreezeArea::OnOverlapEnd);

		
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &ASnapFreezeArea::ApplyPeriodicDamage, DamageInterval, true);

		
		SetLifeSpan(LifeTime);
	}
}

void ASnapFreezeArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (OtherActor && OtherActor != GetInstigator())
	{
		ActorsInArea.Add(OtherActor);
	}
}

void ASnapFreezeArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
	if (OtherActor)
	{
		ActorsInArea.Remove(OtherActor);
	}
}

void ASnapFreezeArea::ApplyPeriodicDamage()
{
	for (AActor* Target : ActorsInArea)
	{
		if (IsValid(Target))
		{
			
			FHitResult HitResult;
			FVector StartLoc = GetActorLocation() + FVector(0.f, 0.f, 50.f); 
			FVector EndLoc = Target->GetActorLocation();

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(GetInstigator()); 

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, Params);

			
			if (!bHit || HitResult.GetActor() == Target)
			{
				UGameplayStatics::ApplyDamage(Target, BaseDamage, GetInstigatorController(), this, UDamageType::StaticClass());
			}
		}
	}
}