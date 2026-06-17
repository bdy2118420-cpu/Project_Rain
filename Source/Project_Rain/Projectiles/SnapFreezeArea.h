// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnapFreezeArea.generated.h"

class UBoxComponent;
class UNiagaraComponent;

UCLASS()
class PROJECT_RAIN_API ASnapFreezeArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnapFreezeArea();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraEffect;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapFreeze|Stats")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapFreeze|Stats")
	float DamageInterval = 1.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapFreeze|Stats")
	float LifeTime = 5.0f; 

	FTimerHandle DamageTimerHandle;

	
	UPROPERTY()
	TSet<AActor*> ActorsInArea;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 초당 데미지 적용 함수
	void ApplyPeriodicDamage();
};
