// Copyright Epic Games, Inc. All Rights Reserved.

#include "UltimateSFCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// AUltimateSFCharacter

AUltimateSFCharacter::AUltimateSFCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 10.f;
	BaseLookUpRate = 10.f;
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 8000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagMaxDistance = 150.0f;
	CameraBoom->CameraLagSpeed = 8.0f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SetReplicates(true);
	SetReplicateMovement(true);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	WalkSpeed = 150.f;
	RunSpeed = 300.f;
	SprintSpeed = 600.f;

	DefaultCombatSpeed = 100.f;
	DefaultCombatDashSpeed = 200.f;


}

//////////////////////////////////////////////////////////////////////////
// Input

void AUltimateSFCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AUltimateSFCharacter::SprintStarted);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AUltimateSFCharacter::SprintStopped);
	PlayerInputComponent->BindAction("ToggleRun", IE_Pressed, this, &AUltimateSFCharacter::ToggleRun);

	//PlayerInputComponent->BindAction("Guarding", IE_Pressed, this, &AUltimateSFCharacter::GuardingStarted);
	//PlayerInputComponent->BindAction("Guarding", IE_Released, this, &AUltimateSFCharacter::GuardingStopped);

	PlayerInputComponent->BindAction("Dodging", IE_Pressed, this, &AUltimateSFCharacter::DodgingFire);


	//PlayerInputComponent->BindAction("ToggleCombatMode", IE_Pressed, this, &AUltimateSFCharacter::ToggleCombatMode);




	//For coordinating attacks
	PlayerInputComponent->BindAction("W", IE_Pressed, this, &AUltimateSFCharacter::IsWPressed);
	PlayerInputComponent->BindAction("W", IE_Released, this, &AUltimateSFCharacter::IsWReleased);

	PlayerInputComponent->BindAction("S", IE_Pressed, this, &AUltimateSFCharacter::IsSPressed);
	PlayerInputComponent->BindAction("S", IE_Released, this, &AUltimateSFCharacter::IsSReleased);

	PlayerInputComponent->BindAction("D", IE_Pressed, this, &AUltimateSFCharacter::IsDPressed);
	PlayerInputComponent->BindAction("D", IE_Released, this, &AUltimateSFCharacter::IsDReleased);

	PlayerInputComponent->BindAction("A", IE_Pressed, this, &AUltimateSFCharacter::IsAPressed);
	PlayerInputComponent->BindAction("A", IE_Released, this, &AUltimateSFCharacter::IsAReleased);

	//Attack inputs
	PlayerInputComponent->BindAction("LeftMouseAttack", IE_Pressed, this, &AUltimateSFCharacter::LeftMouseAttack);
	PlayerInputComponent->BindAction("RightMouseAttack", IE_Pressed, this, &AUltimateSFCharacter::RightMouseAttack);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AUltimateSFCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AUltimateSFCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AUltimateSFCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &AUltimateSFCharacter::MouseX);

	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AUltimateSFCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &AUltimateSFCharacter::MouseY);


	//// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AUltimateSFCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AUltimateSFCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUltimateSFCharacter::OnResetVR);
}

void AUltimateSFCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUltimateSFCharacter, WalkSpeed)
		DOREPLIFETIME(AUltimateSFCharacter, RunSpeed)
		DOREPLIFETIME(AUltimateSFCharacter, SprintSpeed)

		DOREPLIFETIME(AUltimateSFCharacter, bIsCombatMode)
		DOREPLIFETIME(AUltimateSFCharacter, bIsSprinting)
		DOREPLIFETIME(AUltimateSFCharacter, bIsToggleRun)


}


void AUltimateSFCharacter::OnResetVR()
{
	// If BrawlToDeath is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in BrawlToDeath.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUltimateSFCharacter::MouseX(float AxisValue)
{
	MouseXVal = AxisValue;
}

void AUltimateSFCharacter::MouseY(float AxisValue)
{
	MouseYVal = AxisValue;
}

void AUltimateSFCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bIsCombatMode == false) {
		Jump();
	}
}

void AUltimateSFCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bIsCombatMode == false) {
		StopJumping();
	}
}


void AUltimateSFCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUltimateSFCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AUltimateSFCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AUltimateSFCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}








void AUltimateSFCharacter::ToggleRun()
{
	if (bIsCombatMode == true)
	{
		bIsToggleRun = false;
		C_SetToggleRun(bIsToggleRun);
	}
	else if (bIsCombatMode == false)
	{
		if (bIsToggleRun == true)
		{
			bIsToggleRun = false;
			C_MaxWalkSpeed(WalkSpeed);
			C_SetToggleRun(bIsToggleRun);

		}
		else if (bIsToggleRun == false)
		{
			bIsToggleRun = true;
			C_SetToggleRun(bIsToggleRun);
			C_MaxWalkSpeed(RunSpeed);
		}
	}
}




void AUltimateSFCharacter::SprintStarted()
{
	//If combat mode is on it only gets on for a sec so the player cannot dashes constantly
	if (bIsCombatMode == true)
	{
		bIsSprinting = true;
		C_MaxWalkSpeed(DefaultCombatDashSpeed);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AUltimateSFCharacter::SprintFalseTimer, 0.5f, false);

	}
	else if (bIsCombatMode == false)
	{
		bIsToggleRun = true;
		bIsSprinting = true;
		C_MaxWalkSpeed(SprintSpeed);
	}
}




void AUltimateSFCharacter::SprintFalseTimer()
{
	bIsSprinting = false;
	C_MaxWalkSpeed(DefaultCombatSpeed);
}







void AUltimateSFCharacter::SprintStopped()
{
	bIsSprinting = false;

	if (bIsCombatMode == true)
	{
		C_MaxWalkSpeed(DefaultCombatSpeed);
	}
	else if (bIsCombatMode == false)
	{
		if (bIsToggleRun == true)
		{
			C_MaxWalkSpeed(RunSpeed);
		}
		else if (bIsToggleRun == false)
		{
			C_MaxWalkSpeed(WalkSpeed);
			bIsToggleRun = false;
			C_SetToggleRun(bIsToggleRun);

		}
	}
}









void AUltimateSFCharacter::C_SetToggleRun_Implementation(bool RunToggle)
{
	bIsToggleRun = RunToggle;
	S_SetToggleRun(RunToggle);
}
void AUltimateSFCharacter::C_MaxWalkSpeed_Implementation(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	S_MaxWalkSpeed(Speed);
}
void AUltimateSFCharacter::C_SetCombatMode_Implementation(bool CombatModeBool)
{
	bIsCombatMode = CombatModeBool;
	S_SetCombatMode(CombatModeBool);
}


void AUltimateSFCharacter::S_SetToggleRun_Implementation(bool RunToggle)
{
	bIsToggleRun = RunToggle;
}
void AUltimateSFCharacter::S_SetCombatMode_Implementation(bool CombatModeBool)
{
	bIsCombatMode = CombatModeBool;
}
void AUltimateSFCharacter::S_MaxWalkSpeed_Implementation(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}








void AUltimateSFCharacter::IsWPressed()
{
	bIsW = true;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsWReleased()
{
	bIsW = false;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsSPressed()
{
	bIsS = true;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsSReleased()
{
	bIsS = false;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsDPressed()
{
	bIsD = true;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsDReleased()
{
	bIsD = false;
	bIsKicking = false;
	bIsPunching = false;

}

void AUltimateSFCharacter::IsAPressed()
{
	bIsA = true;
	bIsKicking = false;
	bIsPunching = false;
}

void AUltimateSFCharacter::IsAReleased()
{
	bIsA = false;
	bIsKicking = false;
	bIsPunching = false;
}


/// <summary>
/// 
/// 
/// *********************************************************CombatMode Functions*********************************************************
/// 
/// 
/// </summary>



/// 
/// LeftMouse Click Or LeftMouse Click + W -> Jab
/// LeftMouse Click + A Or LeftMouse Click + W + A Or LeftMouse Click + S + A  -> LeftHook
/// LeftMouse Click + D Or LeftMouse Click + W + D Or LeftMouse Click + S + D  -> RightHook
/// LeftMouse Click + Mouse Forward -> Straight
/// LeftMouse Click + S -> UpperCut
/// 

void AUltimateSFCharacter::LeftMouseAttack()
{

	bIsUpper = false;

	if (((bIsA == true && bIsW == true) || (bIsA == true && bIsS == true) || bIsA == true) && (bIsCombatMode == true) && (bIsPunching == false) && (bIsDodging == false))
	{
		bIsPunching = true;
		bIsLeftAttack = true;
		DamageDealt = 8.0;
		S_LeftMouseAttack(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, LeftMouseLeftHook);
	}
	else if (((bIsD == true && bIsW == true) || (bIsD == true && bIsS == true) || bIsD == true) && (bIsCombatMode == true) && (bIsPunching == false) && (bIsDodging == false))
	{
		bIsPunching = true;
		bIsLeftAttack = false;
		DamageDealt = 8.0f;
		S_LeftMouseAttack(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt,LeftMouseRightHook);
	}
	else if ((MouseYVal < -0.15) && (bIsCombatMode == true) && (bIsPunching == false) && (bIsDodging == false))
	{
		bIsPunching = true;
		bIsLeftAttack = false;
		DamageDealt = 10.0f;
		S_LeftMouseAttack(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, LeftMouseStraight);

	}
	else if ((MouseYVal > 0.05) && (bIsCombatMode == true) && (bIsPunching == false) && (bIsDodging == false))
	{
		bIsPunching = true;
		bIsLeftAttack = true;
		DamageDealt = 15.0f;
		S_LeftMouseAttack(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt,LeftMouseUpperCut);
	}
	else if ((bIsCombatMode == true) && (bIsPunching == false) && (bIsDodging == false))
	{
		bIsPunching = true;
		bIsLeftAttack = true;
		DamageDealt = 5.0f;
		S_LeftMouseAttack_Jab(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, LeftMouseJab);
	
	}
	if ((bIsDodging == false) && (bHasDodged == false))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
			{
				bIsUpper = false; bIsPunching = false; bIsLeftAttack = false;
			}, 0.5f, false);
	}
}




/** Left Mouse Attack  Server Side Functions */




void AUltimateSFCharacter::S_LeftMouseAttack_Jab_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsPunching = Punching;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	M_LeftMouseAttack_Jab(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, Anim);
}

void AUltimateSFCharacter::S_LeftMouseAttack_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsPunching = Punching;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	M_LeftMouseAttack(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, Anim);
}


/**  Left Mouse Attack Net Multicast functions **/


void AUltimateSFCharacter::M_LeftMouseAttack_Jab_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsPunching = Punching;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	if (Anim)
	{
		PlayAnimMontage(Anim, 1.5f, NAME_None);
	}
	
}

void AUltimateSFCharacter::M_LeftMouseAttack_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsPunching = Punching;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	if (Anim)
	{
		PlayAnimMontage(Anim, 1.3f, NAME_None);
	}
}









/// 
/// RightMouse Click Or LeftMouse Click + W -> LowKick
/// RightMouse Click + A Or LeftMouse Click + W + A Or LeftMouse Click + S + A  -> LeftMiddleKick
/// RightMouse Click + D Or LeftMouse Click + W + D Or LeftMouse Click + S + D  -> RightMiddleKick
/// RightMouse Click + Mouse Forward -> HighKick
/// 


void AUltimateSFCharacter::RightMouseAttack()
{
	bIsUpper = false;

	if (((bIsA == true && bIsW == true) || bIsA == true) && (bIsCombatMode == true) && (bIsKicking == false) && (bIsDodging == false))
	{
		bIsKicking = true;
		bIsLeftAttack = true;
		DamageDealt = 10.0f;
		S_RightMouseAttack(bIsUpper, bIsKicking, bIsLeftAttack, DamageDealt, RightMouseLeftMiddleKick);
	}
	else if (((bIsD == true && bIsW == true) || bIsD == true) && (bIsCombatMode == true) && (bIsKicking == false) && (bIsDodging == false))
	{
		bIsKicking = true;
		bIsLeftAttack = false;
		DamageDealt = 10.0f;
		S_RightMouseAttack(bIsUpper, bIsKicking, bIsLeftAttack, DamageDealt, RightMouseRightMiddleKick);
	}
	else if ((MouseYVal < -0.05) && (bIsCombatMode == true) && (bIsKicking == false) && (bIsDodging == false))
	{

		bIsKicking = true;
		bIsLeftAttack = false;
		DamageDealt = 20.0f;
		S_RightMouseAttack(bIsUpper, bIsKicking, bIsLeftAttack, DamageDealt, RightMouseHighKick);
	}
	else if (((bIsCombatMode == true) && (bIsKicking == false)) || (((bIsCombatMode == true) && (bIsKicking == false)) && bIsW == true) && (bIsDodging == false))
	{
		bIsKicking = true;
		bIsLeftAttack = false;
		DamageDealt = 5.0f;
		S_RightMouseAttack_LowKick(bIsUpper, bIsKicking, bIsLeftAttack, DamageDealt, RightMouseLowKick);
	}

	if (bIsDodging == false && bHasDodged == false)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			[&]()
			{
				bIsUpper = false; bIsKicking = false; bIsLeftAttack = false;
			},
			1.1f,
				false);
	}
}



/** Right Mouse Attack Server functions **/

void AUltimateSFCharacter::S_RightMouseAttack_LowKick_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsKicking = Kicking;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	M_LeftMouseAttack_Jab(bIsUpper, bIsPunching, bIsLeftAttack, DamageDealt, Anim);
}

void AUltimateSFCharacter::S_RightMouseAttack_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsKicking = Kicking;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	M_LeftMouseAttack(bIsUpper, bIsKicking, bIsLeftAttack, DamageDealt, Anim);
}




/** Right Mouse Attack Net Multicast functions **/


void AUltimateSFCharacter::M_RightMouseAttack_LowKick_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsKicking = Kicking;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	if (Anim)
	{
		PlayAnimMontage(Anim, 1.5f, NAME_None);
	}

}

void AUltimateSFCharacter::M_RightMouseAttack_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim)
{
	bIsUpper = Upper;

	bIsKicking = Kicking;
	bIsLeftAttack = LeftAttack;
	DamageDealt = Dam;
	if (Anim)
	{
		PlayAnimMontage(Anim, 1.3f, NAME_None);
	}
}





void AUltimateSFCharacter::DodgingFire()
{
	if ((bIsCombatMode == true && bIsKicking == false && bIsPunching == false)
		&& ((bIsD == true) || (bIsD == true) && (bIsW == true) || (bIsD == true) && (bIsS == true) || bIsW == true))
	{
		bIsUpper = false;
		bIsDodging = true;
		bHasDodged = true;
		DamageMultiplier = 2.f;
		S_DodgingFire(bIsUpper, bIsDodging, bHasDodged, DamageMultiplier, DodgingRight);
	}
	else if (bIsCombatMode == true && bIsKicking == false && bIsPunching == false)
	{
		bIsUpper = false;
		bIsDodging = true;
		bHasDodged = true;
		DamageMultiplier = 2.f;
		S_DodgingFire(bIsUpper, bIsDodging, bHasDodged, DamageMultiplier, DodgingLeft);
	}

	//Turns bIsDodging off as soon as the animation is over 
	//and then Turns off bHasDodged off and sets DamageMultiplier back to 1
	//a second after the animation is over for the character's next attack to increase its damage

	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[&]()
		{
			bIsDodging = false; GetWorld()->GetTimerManager().SetTimer(TimerHandle,
				[&]()
				{
					bHasDodged = false;  DamageMultiplier = 1.f;
				},
				1.f,
					false);
		},
		1.5f,
			false);



}


void AUltimateSFCharacter::S_DodgingFire_Implementation(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim)
{
	M_DodgingFire(Upper, isDodging, hasDodged, DamageMult, Anim);
}

void AUltimateSFCharacter::M_DodgingFire_Implementation(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim)
{
	bIsUpper = Upper;
	bIsDodging = isDodging;
	bHasDodged = hasDodged;
	DamageMultiplier = DamageMult;
	if (Anim)
	{
		PlayAnimMontage(Anim, 1.0f, NAME_None);
	}
}









// ----------------Substituted by blueprint due to technical issues--------------------------------

//void AUltimateSFCharacter::GuardingStarted()
//{
//	if (bIsKicking == false && bIsCombatMode == true)
//	{
//		DamageReducingValue = 3.f;
//		bIsUpper = true;
//		bIsGuarding = true;
//	}
//}
//
//
//void AUltimateSFCharacter::GuardingStopped()
//{
//	DamageReducingValue = 1.f;
//	bIsGuarding = false;
//	bIsUpper = false;
//}


//void AUltimateSFCharacter::PunchTrace()
//{
//	TArray<TEnumAsByte<EObjectTypeQuery>> PawnArray;
//	PawnArray.Add(EObjectTypeQuery::ObjectTypeQuery1(FCollisionObjectQueryParams::FCollisionObjectQueryParams(ECC_Pawn)));
//	TArray<AActor*> param;
//	FHitResult HitResult;
//	FLinearColor TraceColor = FLinearColor::Red;
//	FLinearColor TraceHitColor = FLinearColor::Green;
//
//	
//	if (bIsPunching == true)
//	{
//		if (bIsLeftAttack == true)
//		{
//			UKismetSystemLibrary::CapsuleTraceSingleForObjects(GetWorld(), this->GetMesh()->GetSocketLocation("hand_l"),
//				this->GetMesh()->GetSocketLocation("hand_l"), 22.0f, 22.0f, PawnArray, false, param, EDrawDebugTrace::None, HitResult , true, TraceColor, TraceHitColor, 5.0f);
//	
//
//		}
//
//	}
//}

//void AUltimateSFCharacter::PivotAnimationsController()
//{
//	bIsUpper = false;
//	if (bIsCombatMode == true && bIsKicking == false && bIsPunching == false)
//	{
//		if (MouseXVal > 1.0f)
//		{
//			bIsUpper = false;
//			if (RightPivot)
//			{
//				PlayAnimMontage(RightPivot, 1.4f, NAME_None);
//			}
//		}
//		else if (MouseXVal < -1.0f)
//		{
//			bIsUpper = false;
//			if (LeftPivot)
//			{
//				PlayAnimMontage(LeftPivot, 1.4f, NAME_None);
//			}
//		}
//	}
//
//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
//		{
//			bIsUpper = true;
//		}, 1.f, false);
//}


//void AUltimateSFCharacter::ToggleCombatMode()
//{
//	if (bIsCombatMode == true)
//	{
//		bIsCombatMode = false;
//		C_SetCombatMode(bIsCombatMode);
//
//		C_MaxWalkSpeed(WalkSpeed);
//	}
//	else if (bIsCombatMode == false)
//	{
//		bIsCombatMode = true;
//		C_SetCombatMode(bIsCombatMode);
//		C_MaxWalkSpeed(DefaultCombatSpeed);
//
//	}
//}

// ----------------Substituted by blueprint due to technical issues--------------------------------