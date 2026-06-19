// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterDirector.generated.h"

UCLASS()
class PROJECT_RAIN_API AMonsterDirector : public AActor
{
	GENERATED_BODY()
	
public:
	AMonsterDirector();

protected:
	virtual void BeginPlay() override;

	// 스폰을 담당하는 함수
	void SpawnMonster();

	// 블루프린트에서 설정할 스폰할 몬스터 클래스 (지상, 비행 등)
	UPROPERTY(EditAnywhere, Category = "Director")
	TSubclassOf<class ABaseMonster> MonsterClassToSpawn;

	// 스폰 주기 (몇 초마다 스폰할 것인가?)
	UPROPERTY(EditAnywhere, Category = "Director")
	float SpawnInterval = 3.0f;

	// 플레이어를 기준으로 스폰될 반경 (이 안에서 랜덤으로 생성됨)
	UPROPERTY(EditAnywhere, Category = "Director")
	float SpawnRadius = 2000.0f;

	FTimerHandle SpawnTimerHandle;
};
