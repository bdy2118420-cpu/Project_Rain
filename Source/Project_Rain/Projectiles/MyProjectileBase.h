// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
UCLASS()
class PROJECT_RAIN_API AMyProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyProjectileBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	FTimerHandle LifeTimeTimerHandle;
	float MaxLifeTime = 3.0f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = false;

	UFUNCTION()
	void OnRep_IsActive();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float BaseDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	float MinChargeDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	float MaxChargeDamage = 50.0f;

	float CurrentExplosionDamage = 15.0f; 
	bool bHasExploded = false; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	bool bIsMultiHitExplosion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	int32 MaxExplosionHits = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	float ExplosionInterval = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Explosion")
	float ExplosionRadius = 400.0f;

	int32 CurrentHitCount = 0;
	FTimerHandle ExplosionTimerHandle;

	void StartExplosion();
	void ExecuteExplosionHit();

public:
	void ActivateProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	void DeactivateProjectile();
	void SetChargeScale(float ChargeRatio);

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void ReceiveOnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void ReceiveOnDeactivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void ReceiveOnExplosionHit();
};
