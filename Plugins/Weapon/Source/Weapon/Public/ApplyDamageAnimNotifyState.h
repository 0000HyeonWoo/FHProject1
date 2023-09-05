// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ApplyDamageAnimNotifyState.generated.h"

class AFHProjectCharacter;
class ABaseWeapon;
/**
 * 
 */
UCLASS()
class WEAPON_API UApplyDamageAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	

public:
	UApplyDamageAnimNotifyState(const FObjectInitializer& ObjectInitializer);

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	//Set Pointer Value When Notify Begin
	//AFHProjectCharacter* FHProjectCharacterObj;
	//AActor* EquipWeapon;
	//ABaseWeapon* BaseWeaponObj;
};
