// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplyDamageAnimNotifyState.h"
#include "DrawDebugHelpers.h"
#include "FHProjectCharacter.h"
#include "BaseWeapon.h"


UApplyDamageAnimNotifyState::UApplyDamageAnimNotifyState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Do not Active Notify State in UE Editor(In Animation Montage Window)
	//If Do not this, UE Editor Maybe crashed
	bShouldFireInEditor = false;
}

void UApplyDamageAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	UE_LOG(LogClass, Warning, TEXT("NotifyBegin - Start"));

	//Get Mesh's Owner
	AFHProjectCharacter* FHProjectCharacterObj = Cast<AFHProjectCharacter>(MeshComp->GetOwner());

	//Check Character is nullptr
	if (FHProjectCharacterObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("NotifyBegin::FHProjectCharacterObj == nullptr"));
		return;
	}
	
	//EquipWeapon is Character's Weapon
	AActor* EquipWeapon = FHProjectCharacterObj->GetEquipWeapon();

	//Check Weapon
	ABaseWeapon* BaseWeaponObj = Cast<ABaseWeapon>(EquipWeapon);
	if (BaseWeaponObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("NotifyBegin::BaseWeaponObj == nullptr"));
		return;
	}

	//Active Event
	BaseWeaponObj->Execute_Event_ClickAttack(EquipWeapon);

	UE_LOG(LogClass, Warning, TEXT("NotifyBegin - End"));
}

void UApplyDamageAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	//UE_LOG(LogClass, Warning, TEXT("NotifyTick"));
}

void UApplyDamageAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	//UE_LOG(LogClass, Warning, TEXT("NotifyEnd"));
}
