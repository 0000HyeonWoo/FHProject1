// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponInterface.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"


enum class EItemType : uint8;

UCLASS()
class WEAPON_API ABaseWeapon : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void MeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	//Interface Event
	//Test Function
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_Test();

	virtual void Event_Test_Implementation() override;

	UFUNCTION(Server, Reliable)
	void Req_TestFunction();


	//Get Item and Attach Item to Target Character's Socket
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_AttachToComponent(ACharacter* TargetCharacter, const FName& TargetSocketName);

	virtual void Event_AttachToComponent_Implementation(ACharacter* TargetCharacter, const FName& TargetSocketName) override;

	//Detach Item From Target Character's Socket
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_DetachFromActor(ACharacter* TargetCharacter);

	virtual void Event_DetachFromActor_Implementation(ACharacter* TargetCharacter) override;

	// Event Left Click Attack
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_LeftClickAttack(bool IsPressed);

	virtual void Event_LeftClickAttack_Implementation(bool IsPressed) override;

	// Event Right Click Attack
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_RightClickAttack(bool IsPressed);

	virtual void Event_RightClickAttack_Implementation(bool IsPressed) override;

	// Event Click Attack
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Event_ClickAttack();

	virtual void Event_ClickAttack_Implementation() override;


protected:
	//Owner Character
	ACharacter* OwnerCharacter;

	//----------[ Value ]----------
	//Add Count When Completed Left Click Attack, Reset Count When Right Click Attack
	UPROPERTY(Replicated)
	int32 LeftClickCount;

	//Attack Effect Scale Value
	FVector AttackEffectScale;

	//Set Effect Scale Value use this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Setting")
	float EffectScaleValue;

	//Set When Click Event Acive
	bool bIsLeftClick;


	//----------[ Damage ]----------
	//Damage Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Setting")
	int32 ClickAttackDamage;

	//Right Click Damage Limit Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Setting")
	float MaxRightClickDamage;


	//----------[ Socket Name ]----------
	//Attack Point Socket Name
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Socket Name")
	FName AttackStartSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Socket Name")
	FName AttackEndSocketName;

	//Attack Effect Spawn Socket Name
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Socket Name")
	FName AttackEffectSocketName;

	//Attack Sound Spawn Socket Name
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Socket Name")
	FName AttackSoundSocketName;


	//----------[ Sphere Trace ]----------
	//Use When SphereTrace Function
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere Trace Setting")
	float TraceSphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere Trace Setting")
	bool bTraceComplex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere Trace Setting")
	bool bIgnoreSelf;


	//----------[ Range Weapon ]----------
	//Check Weapon Attack Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range Weapon Setting")
	bool bIsRangeWeapon;

	//Use This Value Only Range Attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range Weapon Setting")
	float AttackRange;


public:
	//Return OwnerCharacter
	ACharacter* GetOwnerCharacter() { return OwnerCharacter; };


	//----------[ Left Click ]----------
	//Return LeftClickCount
	int32 GetLeftClickCount() { return LeftClickCount; };

	//Add LeftClickCount
	void AddLeftClickCount() { LeftClickCount += 1; };

	//Initialize LeftClickCount
	//void InitializeLeftClickCount() { LeftClickCount = 0; };

	UFUNCTION(Server, Reliable)
	void Req_InitializeLeftClickCount();

	UFUNCTION(NetMulticast, Reliable)
	void Res_InitializeLeftClickCount();


	//Return IsLeftClick Value
	bool GetIsLeftClick() { return bIsLeftClick; };

	void SetIsLeftClick(bool NewValue) { bIsLeftClick = NewValue; };


	//----------[ Attack Damage ]----------
	//Get Click Attack Damage
	int32 GetClickAttackDamage() { return ClickAttackDamage; };
	
	//Set Click Attack Damage
	void SetClickAttackDamage(int32 NewClickAttackDamage) { ClickAttackDamage = NewClickAttackDamage; };

	//Set Max Right Click Damage
	void SetMaxRightClickDamage(float NewMaxRightClickDamage) { MaxRightClickDamage = NewMaxRightClickDamage; };

	//Return MaxRightClickDamage
	float GetMaxRightClickDamage() { return MaxRightClickDamage; };


	//----------[ Effect Scale ]----------
	//Set Attack Effect Scale Function (Effect Scale Value is Vector)
	void SetAttackEffectScale(float NewScaleValue) { AttackEffectScale = { NewScaleValue, NewScaleValue, NewScaleValue }; };


public:
	//Return Calculated Right Click Damage
	float GetCalculatedRightClickDamage();

	//Play AnimMontage, Target is Weapon's OwnerCharacter
	void PlayAttackAnimMontage(UAnimMontage* TargetAttackMontage);

	void RangeAttack();

	void CloseAttack();

	//Apply Damage to Actor Class
	UFUNCTION(Server, Reliable)
	void Req_ApplyDamageToTargetActor(FVector StartLocation, FVector EndLocation, float Damage);

	//Spawn Emitter At Location
	UFUNCTION(NetMulticast, Reliable)
	void Res_SpawnEmitterAtTargetLocation(FVector TargetLocation, FRotator TargetRotation);



public:
	// BaseWeapon has StaticMesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMeshComponent* StaticMesh;

	// Enum Weapon Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemType")
	EItemType eWeaponType;

	// Use When Attack - Left Click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* AttackMontage;

	// Use When Special Attack - Right Click
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* SpecialAttackMontage;

	// Sound Cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	USoundBase* AttackSound;

	// Attack Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	UParticleSystem* AttackEffect;

};
