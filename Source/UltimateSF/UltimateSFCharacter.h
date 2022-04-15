// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Runtime/Engine/Classes/Components/TimelineComponent.h>
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "UltimateSFCharacter.generated.h"

UCLASS(config=Game)
class AUltimateSFCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AUltimateSFCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
		float TurnRateGamepad;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/*WalkSpeed*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		float WalkSpeed;

	/*RunSpeed*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		float RunSpeed;

	/*SprintSpeed*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		float SprintSpeed;

	/*CombatSpeed*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		float DefaultCombatSpeed;

	/*SprintSpeed*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		float DefaultCombatDashSpeed;

	/*Is Sprinting Bool*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		bool bIsSprinting = false;

	/*Is Combat Bool*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		bool bIsCombatMode = false;

	/*Is Punching Bool*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		bool bIsPunching = false;

	/*Is Toggle Run Bool*/
	UPROPERTY(replicated, EditAnywhere, BlueprintReadWrite, Category = Default)
		bool bIsToggleRun = false;

	/*Enum Movement*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		TEnumAsByte<enum EMovementMode> MovementMode;

	/*Determines if character's using a left arm or left leg to attack*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsLeftAttack = false;

	/*Damage dealt value*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		float DamageDealt;

	/*Damage Recieved value*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		float DamageRecieved;

	//Variable for reducing damage
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
	float DamageReducingValue = 1.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
	//Variable for damage increasing after dodging
	float DamageMultiplier = 1.f;

	/*Is Kicking Bool*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsKicking = false;

	/*Is Upper body bool that determines if the animation is in the upperbody animation slot*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsUpper = true;

	/* is W bool checks if w key is being pressed*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsW = false;

	/* is A bool checks if w key is being pressed*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsA = false;


	/* is S bool checks if w key is being pressed*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsS = false;

	/* is D bool checks if w key is being pressed*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsD = false;

	/* Checks if the character is guarding*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsGuarding = false;

	/* Checks if the character is dodging or has dodged*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bHasDodged = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Combat)
		bool bIsDodging = false;



	/* AnimMontage*/

	// For Left Mouse Attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftMouseJab;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftMouseLeftHook;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftMouseRightHook;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftMouseUpperCut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftMouseStraight;


	//For Right Mouse Attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* RightMouseLowKick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* RightMouseLeftMiddleKick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* RightMouseRightMiddleKick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* RightMouseHighKick;


	//Right/Left pivot animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* RightPivot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* LeftPivot;


	//Guarding animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* Guarding;

	//Dodging animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* DodgingRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CombatAnimations)
		UAnimMontage* DodgingLeft;








	//Variables for MouseX, MouseY functions
	float MouseXVal;

	float MouseYVal;

	bool TraceComplex = false;



protected:

	///* Functions which are going to be implemented in a delay function*/
	void SprintFalseTimer();


	/*  Functions that take MouseX and MouseY axis values*/
	void MouseX(float AxisValue);
	void MouseY(float AxisValue);

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);



	/* Handler for toggle run function*/
	void ToggleRun();


	/** Handler for sprinting */
	void SprintStarted();
	void SprintStopped();


	//Server Side Functions
	UFUNCTION(Server, Reliable)
		void S_MaxWalkSpeed(float Speed);
	void S_MaxWalkSpeed_Implementation(float Speed);

	UFUNCTION(Server, Reliable)
		void S_SetCombatMode(bool CombatModeBool);
	void S_SetCombatMode_Implementation(bool CombatModeBool);

	UFUNCTION(Server, Reliable)
		void S_SetToggleRun(bool RunToggle);
	void S_SetToggleRun_Implementation(bool RunToggle);



	//Multicast Side Functions
	UFUNCTION(Client, Reliable)
		void C_MaxWalkSpeed(float Speed);
	void C_MaxWalkSpeed_Implementation(float Speed);

	UFUNCTION(Client, Reliable)
		void C_SetCombatMode(bool CombatModeBool);
	void C_SetCombatMode_Implementation(bool CombatModeBool);

	UFUNCTION(Client, Reliable)
		void C_SetToggleRun(bool RunToggle);
	void C_SetToggleRun_Implementation(bool RunToggle);





	/*  Handler for dodging animation and damage multiplier variable*/
	void DodgingFire();


	// Server Function for DodgingFire
	UFUNCTION(Server, Reliable)
	void S_DodgingFire(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim);
	void S_DodgingFire_Implementation(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim);

	//Multicast Function for DodgingFire
	UFUNCTION(NetMulticast, Reliable)
	void M_DodgingFire(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim);
	void M_DodgingFire_Implementation(bool Upper, bool isDodging, bool hasDodged, float DamageMult, UAnimMontage* Anim);


	/*  Handler for which keyboard is pressed and fires animations*/
	void IsWPressed();
	void IsWReleased();

	void IsSPressed();
	void IsSReleased();

	void IsDPressed();
	void IsDReleased();

	void IsAPressed();
	void IsAReleased();





	/*  Handler for attacks*/
	void LeftMouseAttack();
	void RightMouseAttack();


	//Multicast Functions for Left mouse attacks

	UFUNCTION(NetMulticast, Reliable)
	void M_LeftMouseAttack_Jab(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void M_LeftMouseAttack_Jab_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);

	UFUNCTION(NetMulticast, Reliable)
	void M_LeftMouseAttack(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void M_LeftMouseAttack_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);



	//Server Functions for Left mouse attacks


	UFUNCTION(Server, Reliable)
		void S_LeftMouseAttack_Jab(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void S_LeftMouseAttack_Jab_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);

	UFUNCTION(Server, Reliable)
		void S_LeftMouseAttack(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void S_LeftMouseAttack_Implementation(bool Upper, bool Punching, bool LeftAttack, float Dam, UAnimMontage* Anim);



	//Multicast Functions for Left mouse attacks

	UFUNCTION(NetMulticast, Reliable)
	void M_RightMouseAttack_LowKick(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void M_RightMouseAttack_LowKick_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);

	UFUNCTION(NetMulticast, Reliable)
	void M_RightMouseAttack(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void M_RightMouseAttack_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);



	//Server Functions for Right mouse attacks

	UFUNCTION(Server, Reliable)
		void S_RightMouseAttack_LowKick(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void S_RightMouseAttack_LowKick_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);

	UFUNCTION(Server, Reliable)
		void S_RightMouseAttack(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);
	void S_RightMouseAttack_Implementation(bool Upper, bool Kicking, bool LeftAttack, float Dam, UAnimMontage* Anim);




	///** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	///** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);



	//---------------------------substituted by blueprint due to technical issues----------------------------------------

/*  Handler for turning combat mode on and off*/
//void ToggleCombatMode();


///*  Handler for guarding animation and reduce damage feature*/
//void GuardingStarted();
//void GuardingStopped();


/*  Handler for right/left pivot boxing animations*/
//void PivotAnimationsController();

//---------------------------substituted by blueprint due to technical issues----------------------------------------


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:
	FTimerHandle TimerHandle;
	FTimerManager TimerManager;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};


