#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "../Characters/BaseMonster.h"
#include "GameFramework/Character.h" 

AMonsterAIController::AMonsterAIController()
{
	
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));


	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f; 
	SightConfig->LoseSightRadius = 2000.0f; 
	SightConfig->PeripheralVisionAngleDegrees = 90.0f; 
	SightConfig->SetMaxAge(5.0f); 

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();


	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetDetected);
}


void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


	ABaseMonster* Monster = Cast<ABaseMonster>(InPawn);
	if (Monster && Monster->BehaviorTree)
	{
		
		BlackboardComponent->InitializeBlackboard(*(Monster->BehaviorTree->BlackboardAsset));
		BehaviorTreeComponent->StartTree(*(Monster->BehaviorTree));
	}
}

void AMonsterAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	
	ACharacter* PlayerCharacter = Cast<ACharacter>(Actor);
	if (PlayerCharacter && PlayerCharacter != GetPawn())
	{
		
		if (Stimulus.WasSuccessfullySensed())
		{
		
			BlackboardComponent->SetValueAsObject(FName("TargetActor"), PlayerCharacter);
		}
		else
		{
		}
	}
}
