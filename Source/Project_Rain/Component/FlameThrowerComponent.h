// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseSkillComponent.h"
#include "FlameThrowerComponent.generated.h"

/**
 * 
 */
class UNiagaraComponent;
class UAnimMontage;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_RAIN_API UFlameThrowerComponent : public UBaseSkillComponent
{
	GENERATED_BODY()
public:
	UFlameThrowerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void StartSkill() override;
	virtual void StopSkill() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Flamethrower")
	float MaxDuration = 5.0f;

	FTimerHandle DurationTimerHandle;
	bool bIsUsing = false;

	UPROPERTY(EditAnywhere, Category = "Flamethrower")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> NiagaraRefs;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flamethrower|Stats")
	float DamageInterval = 0.25f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flamethrower|Stats")
	float FlameRange = 1000.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flamethrower|Stats")
	float FlameRadius = 150.0f; 

	FTimerHandle DamageTimerHandle;

	void ApplyFlamethrowerDamage();

	UFUNCTION(Server, Reliable)
	void Server_Start();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Start();

	UFUNCTION(Server, Reliable)
	void Server_Stop();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Stop();
};
