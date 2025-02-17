// Fill out your copyright notice in the Description page of Project Settings.

#include "DivineComedy.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMath.h"
#include "DivineCameraManager.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>



#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text);

bool ADivineCameraManager::JumpFrame () {
  return WasJumpingLastFrame == 0 && GetViewTargetPawn ()->GetMovementComponent ()->IsFalling ();

}

void ADivineCameraManager::ProcessViewRotation (float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) {
  //Vector pointing player direction when jump starts. Parallel to the XY plane.
  if ( JumpFrame () ) {
    JumpOrientation = GetViewTargetPawn ()->GetActorForwardVector ();
    JumpOrientation.Z = 0;
    JumpOrientation.Normalize ();
  }

  //if ( GetViewTargetPawn ()->GetMovementComponent ()->IsFalling () ) {
  //  print (TEXT ("falling"));
  //} else {
  //  print (TEXT ("standing"));
  //}

  //Determine the camera mode
  float ViewLatitudeAngle = FMath::RadiansToDegrees (FGenericPlatformMath::Acos (FVector::DotProduct (OutViewRotation.Vector (), FVector (0, 0, -1))));
  //print (FString::Printf (TEXT ("down angle: %f"), ViewLatitudeAngle));
  if ( DownFallCameraMode == false )
  {
    if (
      GetViewTargetPawn ()->GetMovementComponent ()->IsFalling () &&
      ViewLatitudeAngle < CameraModeTriggerAngleLow
      )
    {
      DownFallCameraMode = true;
    }
  }
  else if (
    GetViewTargetPawn ()->GetMovementComponent ()->IsFalling () == false ||
    ViewLatitudeAngle > CameraModeTriggerAngleHigh
    )
  {
    DownFallCameraMode = false;
  }


  //Calculate the pitch rotation axis.
  FVector rotAxis;
  if ( DownFallCameraMode )
  {
    rotAxis = JumpOrientation;
  }
  else
  {
    rotAxis = FVector (0, 0, 1);
  }


  //Calculate the rotation.
  FRotator NewOutViewRotation;
  FQuat OutQuat (OutViewRotation);
  FQuat YawQuat (rotAxis, OutDeltaRot.Yaw*Sensitivity/10000);
  FQuat PitchQuat (FVector (0, 1, 0), -OutDeltaRot.Pitch*Sensitivity/10000);
  FRotator result = FRotator (YawQuat*OutQuat*PitchQuat);
  //TODO: If a change from last frame is non-infinitesimal, don't do it.
  NewOutViewRotation = FRotator (result);

  //when in standing mode
  if ( DownFallCameraMode == false )
  {
    if ( NewOutViewRotation.Roll > RollToleranceMargin ) {
      NewOutViewRotation.Roll -= DeltaTime * RollRecoverySpeedParameter*FMath::Abs(NewOutViewRotation.Roll);
    } else if ( NewOutViewRotation.Roll < -RollToleranceMargin ) {
      NewOutViewRotation.Roll += DeltaTime * RollRecoverySpeedParameter*FMath::Abs (NewOutViewRotation.Roll);
    }
  }

  print (FString::Printf (TEXT("Roll: %f"), NewOutViewRotation.Roll));
  
  if ( DownFallCameraMode == false)
  {
    LimitViewPitch (NewOutViewRotation, -85, 85);
  }

   OutViewRotation = NewOutViewRotation;


  OutDeltaRot = FRotator::ZeroRotator;
  WasJumpingLastFrame = GetViewTargetPawn ()->GetMovementComponent ()->IsFalling ();
}


