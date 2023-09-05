// Copyright Epic Games, Inc. All Rights Reserved.

#include "FHProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "BaseWeapon.h"
#include "WeaponInterface.h"



//////////////////////////////////////////////////////////////////////////
// AFHProjectCharacter

AFHProjectCharacter::AFHProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set CapsuleComponent Visible
	GetCapsuleComponent()->bHiddenInGame = false;
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate


	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	// Set JumpZVelocity 700.f(Default) -> 350.f
	GetCharacterMovement()->JumpZVelocity = 350.f;
	GetCharacterMovement()->AirControl = 0.35f;
	//----------[ Default Speed Setting Here ]----------
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// ----------[ Add Setting ]----------
	// Set Oreint Rotation to Movement false to true
	UCharacterMovementComponent* const MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = true;
	}

	// C++ Rotate Pitch, Yaw, Roll
	// UE Rotate Roll, Pitch, Yaw

	// Set Character Mesh Location to Start Point of Capsule Collsion, Mesh looklike Standing on Ground
	// If do not this, Character Mesh floating in air
	FVector NewMeshLocation = { 0.0f, 0.0f, -90.0f };
	GetMesh()->SetRelativeLocation(NewMeshLocation);

	// Set Character Mesh Rotation, Make Character Mesh Look Forward
	// If do not this, Character Mesh always look right
	FRotator NewMeshRotation = { 0.0f, 270.0f, 0.0f };
	GetMesh()->SetRelativeRotation(NewMeshRotation);

	// Set Character Can Crouch
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//Set Weapon Socket Name
	//If you want to change Socket Name, Edit like this -> FName(TEXT("MySocketName"))
	WeaponSocketName = FName(TEXT("Weapon"));

}

// Network Setting
void AFHProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFHProjectCharacter, PlayerRotation);
}

void AFHProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AFHProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If Has Authority, Set PlayerRotation Uproperty(Replicated)
	if (HasAuthority() == true)
	{
		PlayerRotation = GetControlRotation();
	}
}

//----------[ Test function Start ]----------
void AFHProjectCharacter::Req_Test_Implementation(int32 Value)
{
	Res_Test(Value);
}

void AFHProjectCharacter::Res_Test_Implementation(int32 Value)
{
	UE_LOG(LogClass, Warning, TEXT("Test Function - Start"));

	if (Value == NULL)
	{
		UE_LOG(LogClass, Warning, TEXT("Test Function::Value Is NULL"));
		Value = 0;
	}

	UE_LOG(LogClass, Warning, TEXT("Test Function::Print Test :: %d"), Value);
	UE_LOG(LogClass, Warning, TEXT("Test Function - End"));
}
//----------[ Test function End ]----------

void AFHProjectCharacter::Req_DoRollMove_Implementation()
{
	//Client
	Res_DoRollMove();
}

void AFHProjectCharacter::Res_DoRollMove_Implementation()
{
	UE_LOG(LogClass, Warning, TEXT("DoRollMove - Start"));

	// Play Target AnimMontage When Target AnimMontage Is not Playing
	if (bIsMontagePlaying() == true)
	{
		UE_LOG(LogClass, Warning, TEXT("DoRollMove::IsMontagePlaying == true"));
		return;
	}

	// If Character Is Falling = return
	if (GetCharacterMovement()->IsFalling() == true)
	{
		UE_LOG(LogClass, Warning, TEXT("DoRollMove::IsFalling == true"));
		return;
	}

	// if Character Is Crouched = return
	if (bIsCrouched == true)
	{
		UE_LOG(LogClass, Warning, TEXT("DoRollMove::IsCrouched == true"));
		return;
	}

	// If StandToRollMontage Is Not Valid = return
	if (IsValid(StandToRollMontage) == false)
	{
		UE_LOG(LogClass, Warning, TEXT("DoRollMove::IsValid(StandToRollMontage) == false"));
		return;
	}

	// If RunToRollMontage Is Not Valid = return
	if (IsValid(RunToRollMontage) == false)
	{
		UE_LOG(LogClass, Warning, TEXT("DoRollMove::IsValid(RunToRollMontage) == false"));
		return;
	}

	// Check Max Speed And Play AnimMontage by Speed Value
	if (GetCharacterMovement()->GetMaxSpeed() <= 500.0f)
	{
		// Check Montage Is Playing
		//bIsMontagePlaying = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

		UE_LOG(LogClass, Warning, TEXT("DoRollMove::PlayAnimMontage - StandToRollMontage"));
		PlayAnimMontage(StandToRollMontage);
	}
	else if (GetCharacterMovement()->GetMaxSpeed() > 500.0f)
	{
		// Check Montage Is Playing
		//bIsMontagePlaying = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

		UE_LOG(LogClass, Warning, TEXT("DoRollMove::PlayAnimMontage - RunToRollMontage"));
		PlayAnimMontage(RunToRollMontage);
	}

	UE_LOG(LogClass, Warning, TEXT("DoRollMove - End"));
}


void AFHProjectCharacter::Req_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	//Sprint and StopSprint Action Use This Function
	//Default Value 500.f
	//Walk = 500.0f, Sprint 750.0f
	UE_LOG(LogClass, Warning, TEXT("SetMaxWalkSpeed - Start"));

	// Set MaxWalkSpeed New Speed - Server
	Res_SetMaxWalkSpeed(NewSpeed);

	UE_LOG(LogClass, Warning, TEXT("SetMaxWalkSpeed - End"));
}

void AFHProjectCharacter::Res_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	// Set MaxWalkSpeed New Speed - Client
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AFHProjectCharacter::Res_AttachToWeaponSocket_Implementation(AActor* Item)
{
	UE_LOG(LogClass, Warning, TEXT("Res_AttachToWeaponSocket - Start"));

	// EquipWeapon is Target Item
	EquipWeapon = Item;

	// Check EquipWeapon Is Valid
	if (IsValid(EquipWeapon) == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_AttachToWeaponSocket::IsValid(EquipWeapon) == true"));
	}

	// Cast WeaponInterface - Item
	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(Item);

	// Cast WeaponInterface pointer is nullptr = return
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_AttachToWeaponSocket::WeaponInterfaceObj == nullptr"));
		return;
	}

	// Item's Event_AttachToComponent, Attach Target Character is Self
	WeaponInterfaceObj->Execute_Event_AttachToComponent(Item, this, WeaponSocketName);

	UE_LOG(LogClass, Warning, TEXT("Res_AttachToWeaponSocket - End"));
}

void AFHProjectCharacter::Req_GetItem_Implementation()
{
	UE_LOG(LogClass, Warning, TEXT("Req_GetItem - Start"));
	AActor* Weapon = FindWeapon();

	if(Weapon == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Req_GetItem::Weapon == nullptr"));
		return;
	}

	Weapon->SetOwner(GetController());

	Res_GetItem(Weapon);
	UE_LOG(LogClass, Warning, TEXT("Req_GetItem - End"));
}

void AFHProjectCharacter::Res_GetItem_Implementation(AActor* Item)
{
	UE_LOG(LogClass, Warning, TEXT("Res_GetItem - Start"));

	if (IsValid(EquipWeapon) == true)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_GetItem::IsValid(EquipWeapon) == true"));
		return;
	}

	EquipWeapon = Item;

	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(Item);
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_GetItem::WeaponInterfaceObj == nullptr"));
		return;
	}

	WeaponInterfaceObj->Execute_Event_AttachToComponent(Item, this, WeaponSocketName);

	UE_LOG(LogClass, Warning, TEXT("Res_GetItem - End"));
}

void AFHProjectCharacter::Req_DropItem_Implementation()
{
	//Server
	Res_DropItem();
}

void AFHProjectCharacter::Res_DropItem_Implementation()
{
	//Client
	UE_LOG(LogClass, Warning, TEXT("Res_DropItem - Start"));

	// Cast WeaponInterface - EquipWeapon
	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	// Cast WeaponInterface pointer is nullptr = return
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_DropItem::WeaponInterfaceObj == nullptr"));
		return;
	}

	// Item's Event_DetachFromActor, Detach Target Character is Self
	WeaponInterfaceObj->Execute_Event_DetachFromActor(EquipWeapon, this);

	// Set EquipWeapon null
	EquipWeapon = nullptr;

	UE_LOG(LogClass, Warning, TEXT("Res_DropItem - End"));
}

void AFHProjectCharacter::Req_LeftClickAttack_Implementation(bool IsPressed)
{
	//Client
	Res_LeftClickAttack(IsPressed);
}

void AFHProjectCharacter::Res_LeftClickAttack_Implementation(bool IsPressed)
{
	UE_LOG(LogClass, Warning, TEXT("Res_LeftClickAttack - Start"));

	// Cast WeaponInterface - EquipWeapon
	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	// Cast WeaponInterface pointer is nullptr = return
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_LeftClickAttack::WeaponInterfaceObj == nullptr"));
		return;
	}

	WeaponInterfaceObj->Execute_Event_LeftClickAttack(EquipWeapon, IsPressed);

	UE_LOG(LogClass, Warning, TEXT("Res_LeftClickAttack - End"));
}

void AFHProjectCharacter::Req_RightClickAttack_Implementation(bool IsPressed)
{
	//Client
	Res_RightClickAttack(IsPressed);
}

void AFHProjectCharacter::Res_RightClickAttack_Implementation(bool IsPressed)
{
	UE_LOG(LogClass, Warning, TEXT("Res_RightClickAttack - Start"));

	// Cast WeaponInterface - EquipWeapon
	IWeaponInterface* WeaponInterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	// Cast WeaponInterface pointer is nullptr = return
	if (WeaponInterfaceObj == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("Res_RightClickAttack::WeaponInterfaceObj == nullptr"));
		return;
	}

	WeaponInterfaceObj->Execute_Event_RightClickAttack(EquipWeapon, IsPressed);

	UE_LOG(LogClass, Warning, TEXT("Res_RightClickAttack - End"));
}

void AFHProjectCharacter::Event_GetItem_Implementation(EItemType eWeaponType, AActor* Item)
{
	UE_LOG(LogClass, Warning, TEXT("EventGetItem - Start"));
	// WeaponInterface Event Switch
	switch (eWeaponType)
	{
	case EItemType::TestWeapon:
	{
		UE_LOG(LogClass, Warning, TEXT("EventGetItem::EItemType - TestWeapon"));

		// Check Item is Valid
		if (IsValid(Item) == false)
		{
			UE_LOG(LogClass, Warning, TEXT("EventGetItem::IsValid::Item == false"));

			return;
		}

		// Check Item ( Weapon ) has Owner Character
		ABaseWeapon* BaseWeaponObj = Cast<ABaseWeapon>(Item);
		if (BaseWeaponObj->GetOwnerCharacter() != nullptr)
		{
			UE_LOG(LogClass, Warning, TEXT("EventGetItem::BaseWeaponObj->GetOwnerCharacter() != nullptr"));
			return;
		}

		// Check EquipWeapon null
		if (EquipWeapon != nullptr)
		{
			UE_LOG(LogClass, Warning, TEXT("EventGetItem::EquipWeapon != nullptr"));

			return;
		}


		Res_AttachToWeaponSocket(Item);

		break;
	}
	
	}

	UE_LOG(LogClass, Warning, TEXT("EventGetItem - End"));
}

float AFHProjectCharacter::GetCameraTargetArmLength()
{
	return CameraBoom->TargetArmLength;
}

AActor* AFHProjectCharacter::FindWeapon()
{
	UE_LOG(LogClass, Warning, TEXT("FindWeapon - Start"));
	TArray<AActor*> ActorsArray;
	GetCapsuleComponent()->GetOverlappingActors(ActorsArray, ABaseWeapon::StaticClass());

	double MostShortDistance = 99999999.0f;
	AActor* Weapon = nullptr;

	for (AActor* TargetWeapon : ActorsArray)
	{
		double DistanceToCharacter = FVector::Dist(TargetWeapon->GetActorLocation(), GetActorLocation());

		if (MostShortDistance < DistanceToCharacter)
		{
			UE_LOG(LogClass, Warning, TEXT("FindWeapon::MostShortDistance < DistanceToCharacter"));
			continue;
		}

		MostShortDistance = DistanceToCharacter;
		Weapon = TargetWeapon;
	}

	UE_LOG(LogClass, Warning, TEXT("FindWeapon - End"));

	return Weapon;
}

FRotator AFHProjectCharacter::GetPlayerRotation()
{
	//Get Player Character index 0
	ACharacter* PlayerCharacter0 = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	//If index 0 Player = Self, Get Controller Roatation
	if (PlayerCharacter0 == this)
	{
		return GetControlRotation();
	}

	return PlayerRotation;
}


//////////////////////////////////////////////////////////////////////////
// Input

void AFHProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::Look);

		//Roll
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::RollInput);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::SprintInput);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AFHProjectCharacter::StopSprintInput);

		//Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::CrouchInput);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AFHProjectCharacter::StopCrouchInput);

		//Get Item
		EnhancedInputComponent->BindAction(GetItemAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::GetItemInput);

		//Drop Item
		EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::DropItemInput);

		//Attack - RightClick
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::RightClickInput);
		EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Completed, this, &AFHProjectCharacter::StopRightClickInput);

		//Attack - LeftClick
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Triggered, this, &AFHProjectCharacter::LeftClickInput);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Completed, this, &AFHProjectCharacter::StopLeftClickInput);


		//----------[ NumberKey Action ]----------
		EnhancedInputComponent->BindAction(NumberKey1Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey1Input);

		EnhancedInputComponent->BindAction(NumberKey2Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey2Input);

		EnhancedInputComponent->BindAction(NumberKey3Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey3Input);

		EnhancedInputComponent->BindAction(NumberKey4Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey4Input);

		EnhancedInputComponent->BindAction(NumberKey5Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey5Input);

		EnhancedInputComponent->BindAction(NumberKey6Action, ETriggerEvent::Triggered, this, &AFHProjectCharacter::NumberKey6Input);

	}

}

void AFHProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFHProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFHProjectCharacter::RollInput(const FInputActionValue& Value)
{
	//Roll Action Input
	UE_LOG(LogClass, Warning, TEXT("RollInput"));

	//Server
	Req_DoRollMove();
}

void AFHProjectCharacter::SprintInput(const FInputActionValue& Value)
{
	//Sprint Action Input
	//If you want change Sprint Speed, Fix Value SetMaxWalkSpeed(here);
	UE_LOG(LogClass, Warning, TEXT("SprintInput"));

	//Server
	Req_SetMaxWalkSpeed(750.0f);
}

void AFHProjectCharacter::StopSprintInput(const FInputActionValue& Value)
{
	//StopSprint Action Input
	//If you want change Default Speed, Fix Value SetMaxWalkSpeed(here);
	//And Check AFHProjectCharacter(), GetCharacterMovement()->MaxWalkSpeed = here;
	UE_LOG(LogClass, Warning, TEXT("StopSprintInput"));

	//Server
	Req_SetMaxWalkSpeed(500.0f);
}

void AFHProjectCharacter::CrouchInput(const FInputActionValue& Value)
{
	//Crouch Action Input
	UE_LOG(LogClass, Warning, TEXT("CrouchInput"));
	//bIsMontagePlaying = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

	// Any Montage Is Playing = return
	if (bIsMontagePlaying() == true)
	{
		return;
	}

	// If Character Is Falling = return 
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}

	// Do Crouch Move
	Crouch();
	
}

void AFHProjectCharacter::StopCrouchInput(const FInputActionValue& Value)
{
	//StopCrouch Action Input
	UE_LOG(LogClass, Warning, TEXT("StopCrouchInput"));
	//bIsMontagePlaying = GetMesh()->GetAnimInstance()->IsAnyMontagePlaying();

	// Any Montage Is Playing = return
	if (bIsMontagePlaying() == true)
	{
		return;
	}

	// If Character Is Falling = return 
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}
	
	// Do UnCrouch Move
	UnCrouch();
	
}

void AFHProjectCharacter::GetItemInput(const FInputActionValue& Value)
{
	//Get Item Action Input
	UE_LOG(LogClass, Warning, TEXT("GetItemInput"));

	//Server
	//Req_GetItem();
}

void AFHProjectCharacter::DropItemInput(const FInputActionValue& Value)
{
	//Drop Item Action Input
	UE_LOG(LogClass, Warning, TEXT("DropItemInput"));

	//Server
	Req_DropItem();
}

void AFHProjectCharacter::RightClickInput(const FInputActionValue& Value)
{
	//Right Click Input
	UE_LOG(LogClass, Warning, TEXT("RightClickInput"));

	//Check Character has EquipWeapon
	if (EquipWeapon == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("RightClickInput::EquipWeapon == nullptr"));

		return;
	}

	//Server
	//IsPressed is true
	Req_RightClickAttack(true);
}

void AFHProjectCharacter::StopRightClickInput(const FInputActionValue& Value)
{
	//Stop Right Click Input
	UE_LOG(LogClass, Warning, TEXT("StopRightClickInput"));

	//Check Character has EquipWeapon
	if (EquipWeapon == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("StopRightClickInput::EquipWeapon == nullptr"));

		return;
	}

	//Server
	//IsPressed is false
	Req_RightClickAttack(false);
}

void AFHProjectCharacter::LeftClickInput(const FInputActionValue& Value)
{
	//Left Click Input
	UE_LOG(LogClass, Warning, TEXT("LeftClickInput"));

	//Check Character has EquipWeapon
	if (EquipWeapon == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("LeftClickInput::EquipWeapon == nullptr"));

		return;
	}

	//Server
	//IsPressed is true
	Req_LeftClickAttack(true);
}

void AFHProjectCharacter::StopLeftClickInput(const FInputActionValue& Value)
{
	//Stop Left Click Input
	UE_LOG(LogClass, Warning, TEXT("StopLeftClickInput"));

	//Check Character has EquipWeapon
	if (EquipWeapon == nullptr)
	{
		UE_LOG(LogClass, Warning, TEXT("StopLeftClickInput::EquipWeapon == nullptr"));

		return;
	}

	//UE_LOG(LogClass, Warning, TEXT("LeftClickCount :: %d"), LeftClickCount);

	//Server
	//IsPressed is false
	Req_LeftClickAttack(false);
}

void AFHProjectCharacter::NumberKey1Input(const FInputActionValue& Value)
{
	Req_Test(1);
}

void AFHProjectCharacter::NumberKey2Input(const FInputActionValue& Value)
{

	Req_Test(2);
}

void AFHProjectCharacter::NumberKey3Input(const FInputActionValue& Value)
{

	Req_Test(3);
}

void AFHProjectCharacter::NumberKey4Input(const FInputActionValue& Value)
{

	Req_Test(4);
}

void AFHProjectCharacter::NumberKey5Input(const FInputActionValue& Value)
{

	Req_Test(5);

}

void AFHProjectCharacter::NumberKey6Input(const FInputActionValue& Value)
{

	Req_Test(6);

}


