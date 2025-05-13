// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkedCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineSessionInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetworkedCharacter

ANetworkedCharacter::ANetworkedCharacter() :
	OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	OnFindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(123, 10.f, FColor::Red, 
				FString::Printf(TEXT("Subsystem name: %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
		}
	}
}

void ANetworkedCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

void ANetworkedCharacter::CreateNewSession()
{
	if (OnlineSessionInterface.IsValid())
	{
		auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession != nullptr)
		{
			OnlineSessionInterface->DestroySession(NAME_GameSession);
		}

		OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
		TSharedPtr<FOnlineSessionSettings> OnlineSessionSettings = MakeShareable(new FOnlineSessionSettings());
		OnlineSessionSettings->bIsLANMatch = false;
		OnlineSessionSettings->bAllowJoinInProgress = true;
		OnlineSessionSettings->NumPublicConnections = 4;
		OnlineSessionSettings->bShouldAdvertise = true;
		OnlineSessionSettings->bUsesPresence = true;
		OnlineSessionSettings->bAllowJoinViaPresence = true;
		OnlineSessionSettings->bUseLobbiesIfAvailable = true;
		OnlineSessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		OnlineSessionInterface->CreateSession(*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), NAME_GameSession, *OnlineSessionSettings);
	}
}

void ANetworkedCharacter::JoinOnlineSession()
{
	
	if (OnlineSessionInterface)
	{
		OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionCompleteDelegate);
		SessionSearch = MakeShareable(new FOnlineSessionSearch);
		SessionSearch->MaxSearchResults = 1000;
		SessionSearch->bIsLanQuery = false;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		OnlineSessionInterface->FindSessions(*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
	}
}

void ANetworkedCharacter::OnCreateSessionComplete(FName SessionName, bool isSuccess)
{
	if (isSuccess && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString(TEXT("Session Created from character class")));
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Blue, SessionName.ToString());
		GetWorld()->ServerTravel(FString("/Game/ThirdPerson/Maps/PlayerLobby?listen"));
	}
	else
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString(TEXT("Session Creation Failed from character class")));

}

void ANetworkedCharacter::OnFindSessionComplete(bool isSuccess)
{
	if (OnlineSessionInterface.IsValid())
	{
		for (auto Result : SessionSearch->SearchResults)
		{
			auto ID = Result.GetSessionIdStr();
			auto UserID = Result.Session.OwningUserName;
			FString MatchType;
			Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

			GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red,
				FString::Printf(TEXT("SessionID: %s UserName: %s"), *ID, *UserID));

			if (MatchType == FString("FreeForAll"))
			{
				GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, TEXT("Joining free for all session"));
				OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

				OnlineSessionInterface->JoinSession(*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), NAME_GameSession, Result);
			}
		}
		
	}
}

void ANetworkedCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSessionInterface.IsValid())
	{
		FString Address;
		OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString::Printf(TEXT("Connection String: %s"), *Address));

		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetworkedCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkedCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkedCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetworkedCharacter::Move(const FInputActionValue& Value)
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

void ANetworkedCharacter::Look(const FInputActionValue& Value)
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

void ANetworkedCharacter::CallOpenLevel(const FString& LevelName)
{
	UGameplayStatics::OpenLevel(this, *LevelName);
}

void ANetworkedCharacter::CallClientTravel(const FString& Address)
{
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void ANetworkedCharacter::OpenLobby()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel("/Game/ThirdPerson/Maps/PlayerLobby?listen");
	}
}
