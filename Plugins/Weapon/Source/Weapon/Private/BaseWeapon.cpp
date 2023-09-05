// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "WeaponInterface.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/Vector.h"
#include "FHProjectCharacter.h"




// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting Static Mesh
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionProfileName("Weapon");
	StaticMesh->SetSimulatePhysics(true);

	StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::MeshBeginOverlap);

	//Server Replicate Setting
	bReplicates = true;
	SetReplicateMovement(true);

	//Initialize LeftClickCount, int Type
	LeftClickCount = 0;
	
	//Initialize ClickAttackDamage, int Type
	ClickAttackDamage = 0;
	SetClickAttackDamage(10);

	//Initialize ClickAttackDamage, float Type
	MaxRightClickDamage = 0;
	SetMaxRightClickDamage(GetClickAttackDamage() * 1.5f);

	//Set Weapon Attack Type
	bIsRangeWeapon = true;

	AttackRange = 1000.f;

	//Set Attack Socket Name
	//If you want to change Socket Name, Edit like this -> FName(TEXT("MySocketName"))
	AttackStartSocketName = FName(TEXT("Attack_Start"));
	AttackEndSocketName = FName(TEXT("Attack_End"));

	//Set Emitter Spawn Socket Name
	AttackEffectSocketName = FName(TEXT("Attack_Effect"));

	//Set Attack Effect Scale
	EffectScaleValue = 1.0f;

	//Don't touch this function
	SetAttackEffectScale(EffectScaleValue);

	//Set Sound Spawn Socket Name
	AttackSoundSocketName = FName(TEXT("Attack_Sound"));

	//Sphere Trace Setting
	//Set this Value by Weapon's StaticMesh Attack Parts
	TraceSphereRadius = 32.0f;
	bTraceComplex = false;
	bIgnoreSelf = true;

}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeapon, LeftClickCount);
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (HasAuthority() == true)
	{
		LeftClickCount = GetLeftClickCount();
	}*/

}

void ABaseWeapon::MeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap - Start"));

	// BaseWeapon -> ~Weapon(BaseWeapon) -> Begin Play -> Set Enum Type ->  Event(Enum Type) ->
	// Character(Interface Event) -> Enum Switch -> Cast -> Cast Target Event

	// BaseWeapon -> Weapon ex1 -> ex1.BeginPlay -> Set Enum Type ex1, Interface Event, Destroy
	// Character(Interface Event) -> ....

	//Check Weapon has Owner Character
	if (OwnerCharacter != nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::OwnerCharacter != nullptr"));
		return;
	}

	// If HasAuthority is false = return
	if (HasAuthority() == false)
	{
		UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::HasAuthority == false"));
		return;
	}

	AFHProjectCharacter* CharacterObj = Cast<AFHProjectCharacter>(OtherActor);

	if (CharacterObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::CharacterObj == nullptr"));
		return;
	}

	// Check Overlaped Actor has ( IWeaponInterface ) 
	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(CharacterObj);

	// If ( WeaponInterfaceObj ) is nullptr = return
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::WeaponInterfaceObj == nullptr"));
		return;
	}

	this->SetOwner(CharacterObj->GetController());

	UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::Execute_EventGetItem"));
	WeaponInterfaceObj->Execute_Event_GetItem(OtherActor, eWeaponType, this);

	// If Actor Destroyed, Character's AttachToComponent function doesn't work
	//Destroy();

	UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap::Character Name :: %s"), *OtherActor->GetName());
	UE_LOG(LogClass, Warning, TEXT("MeshBeginOverlap - End"));
}

void ABaseWeapon::Event_Test_Implementation()
{
	//Server
	Req_TestFunction();
}

void ABaseWeapon::Req_TestFunction_Implementation()
{
	//Test Function
	UE_LOG(LogClass, Warning, TEXT("Req_TestFunction::Event_Test - Start"));
	UE_LOG(LogClass, Warning, TEXT("Req_TestFunction::Event_Test - End"));
}

void ABaseWeapon::Event_AttachToComponent_Implementation(ACharacter* TargetCharacter, const FName& TargetSocketName)
{
	UE_LOG(LogClass, Warning, TEXT("Event_AttachToComponent - Start"));

	// Set Owner Character
	OwnerCharacter = TargetCharacter;

	// SetSimulatePhysics false Because StaticMesh will attach to Target Socket
	StaticMesh->SetSimulatePhysics(false);

	// And Attach to Target Component Name ( FName("weapon") ) on Character Mesh
	AttachToComponent(TargetCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);

	UE_LOG(LogClass, Warning, TEXT("Event_AttachToComponent - End"));

}

void ABaseWeapon::Event_DetachFromActor_Implementation(ACharacter* TargetCharacter)
{
	UE_LOG(LogClass, Warning, TEXT("Event_DetachFromActor - Start"));

	// Set Owner Character null
	OwnerCharacter = nullptr;

	// SetSimulatePhysics true Because StaticMesh will Detach from Target Socket
	StaticMesh->SetSimulatePhysics(true);

	// Detach from Owner Character
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	UE_LOG(LogClass, Warning, TEXT("Event_DetachFromActor - End"));
}

void ABaseWeapon::Event_LeftClickAttack_Implementation(bool IsPressed)
{
	//Character Left Click Input Event Active this function
	UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack - Start"));

	if (IsPressed == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::IsPressed true"));

		//Check Weapon has OwnerCharacter
		if (OwnerCharacter == nullptr)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::OwnerCharacter == nullptr"));
			return;
		}

		//Check Any Montage Playing
		if (OwnerCharacter->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying() == true)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::IsAnyMontagePlaying == true"));
			return;
		}

		// If AttackMontage Is Not Valid = return
		if (IsValid(AttackMontage) == false)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::IsValid(AttackMontage) == false"));
			return;
		}

		//Add Left Click Count
		AddLeftClickCount();
		UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::LeftClickCount :: %d"), LeftClickCount);

		PlayAttackAnimMontage(AttackMontage);

		//Left Click is true
		SetIsLeftClick(true);
	}
	else if (IsPressed == false)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack::IsPressed false"));
	}

	UE_LOG(LogClass, Warning, TEXT("Event_LeftClickAttack - End"));
}

void ABaseWeapon::Event_RightClickAttack_Implementation(bool IsPressed)
{
	//Character Right Click Input Event Active this function
	UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack - Start"));

	if (IsPressed == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack::IsPressed true"));

		//Check Weapon has OwnerCharacter
		if (OwnerCharacter == nullptr)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack::OwnerCharacter::nullptr"));
			return;
		}

		//Check Any Montage Playing
		if (OwnerCharacter->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying() == true)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack::IsAnyMontagePlaying::true"));
			return;
		}

		// If SpecialAttackMontage Is Not Valid = return
		if (IsValid(SpecialAttackMontage) == false)
		{
			UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack::IsValid::SpecialAttackMontage, false"));
			return;
		}

		//** Move This function ClickEvent **
		//float RightClickDamage;
		//RightClickDamage = GetCalculatedRightClickDamage();
		//UE_LOG(LogClass, Warning, TEXT("CalculatedRightClickDamage :: %d"), RightClickDamage);

		PlayAttackAnimMontage(SpecialAttackMontage);

		//Left Click is flase
		SetIsLeftClick(false);
	}
	else if (IsPressed == false)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack::IsPressed false"));
	}

	UE_LOG(LogClass, Warning, TEXT("Event_RightClickAttack - End"));
}

void ABaseWeapon::Event_ClickAttack_Implementation()
{
	UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack - Start"));
	//Click Event

	//Set Attack Start, End Location Value by Weapon Attack Type
	FVector AttackStartLocation;
	FVector AttackEndLocation;

	if (bIsRangeWeapon == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::IsRangeWeapon == true"));
		//Range Weapon
		//----------[ Start Calculate Attack Start, End Location ]----------
		
		APlayerController* FirstPlayerController = GetWorld()->GetFirstPlayerController();
		
		//Get Player's Camera Location
		FVector CameraLocation;
		CameraLocation = FirstPlayerController->PlayerCameraManager->GetCameraLocation();

		//UE_LOG(LogClass, Warning, TEXT("CameraLocation %s"), *CameraLocation.ToString());

		//Get Player's Camera Forward Vector
		FVector CameraForwardVector;
		CameraForwardVector = FirstPlayerController->PlayerCameraManager->GetActorForwardVector();

		//Get Distance to Player's Camera and StaticMesh's Attack Start Socket Location
		float Distance;
		Distance = FVector::Distance(CameraLocation, StaticMesh->GetSocketLocation(AttackStartSocketName));

		//Attack Start Location is
		AttackStartLocation = CameraLocation + (CameraForwardVector * Distance);

		//Attack End Location is
		AttackEndLocation = AttackStartLocation + (CameraForwardVector * AttackRange);

		//----------[ End Calculate Attack Start, End Location ]----------

		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::AttackStartLocation %s"), *AttackStartLocation.ToString());
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::AttackEndLocation %s"), *AttackEndLocation.ToString());

		//If Range Weapon, Spawn Emitter at Attack End Location
		//Rotation is Weapon StaticMesh's Rotation Value
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AttackEffect, AttackEndLocation, StaticMesh->GetRelativeRotation(), AttackEffectScale);
		//** Move this function Res_SpawnEmitterAtTargetLocation **
		//Because After Trace, Client can't see Effect
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::IsRangeWeapon == false"));
		//Not Range Weapon

		//Set Start, End Point Location Vector by Socket Location, Target is Weapon Mesh's Socket
		AttackStartLocation = StaticMesh->GetSocketLocation(AttackStartSocketName);
		AttackEndLocation = StaticMesh->GetSocketLocation(AttackEndSocketName);

		//Spawn Target Emitter by Weapon Type
		//If not Range Weapon, Spawn Emitter at AttackEffectSocket's Location, Rotation
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AttackEffect, StaticMesh->GetSocketLocation(AttackEffectSocketName), StaticMesh->GetSocketRotation(AttackEffectSocketName), AttackEffectScale);
	}

	//Spawn Target Sound AttackSoundSocket's Location
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), AttackSound, StaticMesh->GetSocketLocation(AttackSoundSocketName));

	//----------[ UNetDirver Error Start ]----------
	//Check Has Authority
	/*if (HasAuthority() == false)
	{
		UE_LOG(LogClass, Warning, TEXT("HasAuthority == false"));
		return;
	}*/

	//Check Weapon's OwnerCharacter is First Index Player
	if (UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) != OwnerCharacter)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::GetPlayerCharacter != OwnerCharacter"));
		return;
	}
	//----------[ UNetDirver Error End ]----------


	//Active Event by Left Click Value
	//Use Start, End Location is Same, Difference is only Damage
	if (GetIsLeftClick() == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::GetIsLeftClick == true"));

		//Left Click Damage Event Use Default Damage
		Req_ApplyDamageToTargetActor(AttackStartLocation, AttackEndLocation, GetClickAttackDamage());
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::GetIsLeftClick == false"));

		//Right Click Damage Event Use Calculated Damage
		Req_ApplyDamageToTargetActor(AttackStartLocation, AttackEndLocation, GetCalculatedRightClickDamage());
	
		UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack::Initialize LeftClickCount :: %d"), LeftClickCount);
	}

	UE_LOG(LogClass, Warning, TEXT("Event_ClickAttack - End"));
}

void ABaseWeapon::Req_InitializeLeftClickCount_Implementation()
{
	//Server
	UE_LOG(LogClass, Warning, TEXT("Req_InitializeLeftClickCount"));
	Res_InitializeLeftClickCount();
}

void ABaseWeapon::Res_InitializeLeftClickCount_Implementation()
{
	//Client
	UE_LOG(LogClass, Warning, TEXT("Res_InitializeLeftClickCount"));
	LeftClickCount = 0;
}

float ABaseWeapon::GetCalculatedRightClickDamage()
{
	UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage - Start"));
	//Calculate Right Click Damage

	//Create Return Value
	float RightClickDamage;
	RightClickDamage = 0;

	// ** Right Click Damage = Default Damage + ( Default Damage * (Click Count * 0.1f)) **
	// Default Damage = Click Attack Damage
	// (Click Count * 0.1f) -> 1 = 0.1, 2 = 0.2, 3 = 0.3,,,

	//RightClickDamage increase by LeftClickCount Value
	//Each Left Click increase Damage Value 10%
	UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage::LeftClickCount :: %d"), GetLeftClickCount());
	RightClickDamage = GetClickAttackDamage() + (GetClickAttackDamage() * (GetLeftClickCount() * 0.1f));

	//Check Calculated Damage Value
	if (RightClickDamage > GetMaxRightClickDamage())
	{
		UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage::RightClickDamage > GetMaxRightClickDamage"));
		UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage::RightClickDamage Value Before Change:: %f"), RightClickDamage);
		//If Calculated Value bigger than Max Value, Set Calculated Value to Max Value
		RightClickDamage = GetMaxRightClickDamage();
	}
	
	//Check Calculated Value
	UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage::RightClickDamage :: %f"), RightClickDamage);

	UE_LOG(LogClass, Warning, TEXT("GetCalculatedRightClickDamage - End"));

	return RightClickDamage;
}

void ABaseWeapon::PlayAttackAnimMontage(UAnimMontage* TargetAttackMontage)
{
	UE_LOG(LogClass, Warning, TEXT("PlayAttackAnimMontage - Start"));
	//Play Attack AnimMontage

	OwnerCharacter->PlayAnimMontage(TargetAttackMontage);

	UE_LOG(LogClass, Warning, TEXT("PlayAttackAnimMontage - End"));
}

void ABaseWeapon::RangeAttack()
{
	UE_LOG(LogClass, Warning, TEXT("RangeAttack - Start"));

}

void ABaseWeapon::CloseAttack()
{
	UE_LOG(LogClass, Warning, TEXT("CloseAttack - Start"));

}

void ABaseWeapon::Res_SpawnEmitterAtTargetLocation_Implementation(FVector TargetLocation, FRotator TargetRotation)
{
	UE_LOG(LogClass, Warning, TEXT("SpawnEmitterAtTargetLocation - Start"));
	//Range Weapon Use this function

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), AttackEffect, TargetLocation, TargetRotation, AttackEffectScale);

	UE_LOG(LogClass, Warning, TEXT("SpawnEmitterAtTargetLocation - End"));
}

void ABaseWeapon::Req_ApplyDamageToTargetActor_Implementation(FVector StartLocation, FVector EndLocation, float Damage)
{
	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor - Start"));

	//Check Damage Value
	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::Apply Damage :: %f"), Damage);

	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::StartLocation %s"), *StartLocation.ToString());
	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::EndLocation %s"), *EndLocation.ToString());

	//Trace Result Value
	bool bIsHit;
	bIsHit = true;

	//Set Collision for Trace Function
	FHitResult AttackHitResult;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);

	//Start Trace by Weapon Type
	//Range Weapon is LineTrace, else Weapon SphereTrace
	if (bIsRangeWeapon == true)
	{
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::IsRangeWeapon == true"));

		//----------[ Add Ignore Actor ]----------
		FCollisionQueryParams QueryParamsIgnoredActor;
		QueryParamsIgnoredActor.AddIgnoredActor(OwnerCharacter);
		QueryParamsIgnoredActor.AddIgnoredActor(this);

		//Trace by Location Value, Collision
		bIsHit = GetWorld()->LineTraceSingleByObjectType(AttackHitResult, StartLocation, EndLocation, QueryParams, QueryParamsIgnoredActor);

		//DrawDebugLine for Check LineTrace Function is Working
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 5.0f);

		//If LineTrace hit anything, Spawn Emitter at Trace Blocking Location
		//Not hit anything, Spawn Emitter at Trace End Location
		if (AttackHitResult.bBlockingHit == true)
		{
			UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::BlockingHit == true"));
			Res_SpawnEmitterAtTargetLocation(AttackHitResult.Location, StaticMesh->GetRelativeRotation());
		}
		else
		{
			UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::BlockingHit == false"));
			Res_SpawnEmitterAtTargetLocation(EndLocation, StaticMesh->GetRelativeRotation());
		}

	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::IsRangeWeapon == false"));

		//----------[ Add Ignore Actor ]----------
		TArray<AActor*> IgnoreActors;
		//----------[ Check Ignore Character Here ]----------
		IgnoreActors.Add(OwnerCharacter);
		IgnoreActors.Add(this);

		//Start SphereTrace
		//Color Red = Hit false, Green = Hit true
		bIsHit = UKismetSystemLibrary::SphereTraceSingle
		(
			GetWorld(),
			StartLocation,
			EndLocation,
			TraceSphereRadius,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_OverlapAll_Deprecated),
			bTraceComplex,
			IgnoreActors,
			EDrawDebugTrace::ForDuration,
			AttackHitResult,
			bIgnoreSelf,
			FColor::Red,
			FColor::Green,
			5.f
		);

	}

	//If Trace(Attack) can't hit Anything, return
	if (bIsHit == false)
	{
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::IsHit == false"));

		//Initialize LeftClickCount 0;
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::InitializeLeftClickCount"));
		Req_InitializeLeftClickCount();

		return;
	}

	//Get Hit Actor
	AActor* HitTargetObj = Cast<AActor>(AttackHitResult.GetActor());

	//Check Hit Actor nullptr
	if (HitTargetObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::HitTargetObj == nullptr"));
		return;
	}
	
	//Check Hit Actor's Name
	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::Hit Actor :: %s"), *FString(HitTargetObj->GetName()));

	//Apply Damage to Hit Actor, This function Active Target's TakeDamage
	UGameplayStatics::ApplyDamage(HitTargetObj, Damage, OwnerCharacter->GetController(), this, UDamageType::StaticClass());

	//Check Click was Right Click
	if (GetIsLeftClick() == false)
	{
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::GetIsLeftClick == false"));

		//Initialize LeftClickCount 0;
		UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::InitializeLeftClickCount"));
		Req_InitializeLeftClickCount();
	}

	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor::ApplyDamage"));

	//Check ApplyDamage End
	//UE_LOG(LogClass, Warning, TEXT("ApplyDamage End"));

	UE_LOG(LogClass, Warning, TEXT("ApplyDamageToTargetActor - End"));

}
