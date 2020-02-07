// Fill out your copyright notice in the Description page of Project Settings.

#include "Cell.h"

#include "PPPGameMode.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"
#include "Engine/World.h"


// Sets default values
ACell::ACell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetupAttachment(RootComponent);

	auto meshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));

	VisualMesh->OnBeginCursorOver.AddDynamic(this, &ACell::CustomOnBeginMouseOver);
	VisualMesh->OnEndCursorOver.AddDynamic(this, &ACell::CustomOnEndMouseOver);
	OnClicked.AddDynamic(this, &ACell::CustomOnClicked);

	if (meshAsset.Object != nullptr)
	{		
		VisualMesh->SetStaticMesh(meshAsset.Object);
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		VisualMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 4.0f));
	}
}

// Called when the game starts or when spawned
void ACell::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Reachable || Actable) {
		FVector loc = GetActorLocation() + FVector(0, 0, 410);
		FVector size = FVector(45, 45, 2);

		FColor color;
		if (MouseOver) {
			color = FColor::Green;
		}
		else {
			if (Reachable) {
				color = FColor::Blue;
			}
			else if (Actable) {
				color = FColor::Red;
			}
		}

		DrawDebugBox(GetWorld(), loc, size, color, false, -1, 0, 5);
	}
}

void ACell::setCellHeight(int height)
{
	FVector loc = GetActorLocation();
	loc.Z = (float)height * 110.0f / 2.0f;	
	SetActorLocation(loc);
}

void ACell::setPos(FIntPoint p)
{
	pos = p;

	FVector loc = GetActorLocation();
	loc.X = (float)p.X * 110.0f;
	loc.Y = (float)p.Y * 110.0f;
	SetActorLocation(loc);
}

FIntPoint ACell::getPos() { 
	return pos; 
};

void ACell::SetReachable(bool value) {
	Reachable = value;
}

void ACell::SetActable(bool value) {
	Actable = value;
}

void ACell::CustomOnBeginMouseOver(UPrimitiveComponent* TouchedComponent) {
	MouseOver = true;
}

void ACell::CustomOnEndMouseOver(UPrimitiveComponent* TouchedComponent) {
	MouseOver = false;
}

void ACell::CustomOnClicked(AActor* Target, FKey ButtonPressed) {	
	if (Reachable) {
		APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
		if (gameMode) {
			gameMode->OnUnitCommandMove(pos);
		}
	}
	else if (Actable) {
		APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
		if (gameMode) {
			gameMode->OnUnitCommandAct(pos);
		}
	}
}
