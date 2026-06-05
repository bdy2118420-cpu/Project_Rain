// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyCheckCombo.h"
#include "MyCharacterBase.h"

void UAnimNotifyCheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

  
}
