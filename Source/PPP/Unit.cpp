// Fill out your copyright notice in the Description page of Project Settings.

#define DUR_BASE 0.25f
#define TEXT_OFFSET FVector(0.0f, 0.0f, 150.0f)

#include "Unit.h"

#include "PPPGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "Engine.h"

#define STAT_BAR_OFFSET 150.0f
#define COMMAND_BAR_OFFSET 100.0f

// Sets default values
AUnit::AUnit()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisualMesh->SetupAttachment(RootComponent);

	auto meshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/StarterContent/Shapes/Shape_TriPyramid"));
	
	if (meshAsset.Object != nullptr)
	{
		VisualMesh->SetStaticMesh(meshAsset.Object);
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}

	VisualMesh->OnBeginCursorOver.AddDynamic(this, &AUnit::CustomOnBeginMouseOver);
	OnClicked.AddDynamic(this, &AUnit::CustomOnClicked);

	StatBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatBar"));
	if (StatBarWidget) {
		static ConstructorHelpers::FClassFinder<UUserWidget> widgetObj(TEXT("/Game/Widget/BP_StatBar"));
		if (widgetObj.Succeeded()) {
			StatBarWidget->SetWidgetClass(widgetObj.Class);
			StatBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, STAT_BAR_OFFSET));
			StatBarWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			StatBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
		}
	}
	
	CommandBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CommandBar"));
	if (CommandBarWidget) {
		static ConstructorHelpers::FClassFinder<UUserWidget> widgetObj(TEXT("/Game/Widget/BP_UnitCommandBar"));
		if (widgetObj.Succeeded()) {
			CommandBarWidget->SetWidgetClass(widgetObj.Class);
			CommandBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, COMMAND_BAR_OFFSET));
			CommandBarWidget->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			CommandBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
			CommandBarWidget->SetVisibility(false);
		}
	}
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();

	InfoText = GetWorld()->SpawnActor<ATextRenderActor>(ATextRenderActor::StaticClass(), TEXT_OFFSET, FRotator(90.f, 0.f, 0.f));
	SetInfoText(FText::FromString(FString("")));
}

void AUnit::Destroy()
{
	Super::Destroy();

	DestroyInfoText();
}

void AUnit::SetCommand(bool canMove, bool canAct) {
	if (!CommandBarWidget) {
		return;
	}
	CommandBarWidget->SetVisibility(true);
	UUnitCommandBar* bar = GetUnitCommandBar();
	if (bar) {
		bar->SetAvailable(canMove, canAct);
	}
}

void AUnit::ClearCommand() {
	if (!CommandBarWidget) {
		return;
	}
	CommandBarWidget->SetVisibility(false);
}

void AUnit::DestroyInfoText()
{
	UE_LOG(LogTemp, Log, TEXT("BeginDestroy"));	
	if (InfoText) {
		UE_LOG(LogTemp, Log, TEXT("BeginDestroy  -  in"));		
		InfoText->Destroy();
		InfoText = NULL;
	}	
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (pEventToken && pEventToken->IsActive()) {
		pEventToken->EndAfter -= DeltaTime;
		pEventToken->Elapsed += DeltaTime;		
		if (pEventToken->EndAfter < 0) {
			pEventToken->Finished();
			if (pEventToken->Type == LogicEventTypes::REMOVE_) {
				Destroy();
			}
			else if (pEventToken->Type == LogicEventTypes::USE_ ) {
			}			
			pEventToken = NULL;
			return;
		}

		switch (pEventToken->Type) {
			case LogicEventTypes::REMOVE_:
			{
				AdjustPos(0.0f, 0.0f, -150.0f * DeltaTime / DUR_BASE);
			}
			break;
			case LogicEventTypes::MOVE_:
			{
				int cur = int(floor(pEventToken->Elapsed / DUR_BASE));
				int next = cur + 1;
				if (pEventToken->Path.IsValidIndex(cur) &&
					pEventToken->Path.IsValidIndex(next)) {

					FVector diff = GetCellPos(pEventToken->Path[next]) 
						- GetCellPos(pEventToken->Path[cur]);					
					AdjustPos(
						diff.X * DeltaTime / DUR_BASE,
						diff.Y * DeltaTime / DUR_BASE,
						diff.Z * DeltaTime / DUR_BASE );
				}
			}
			break;
		}
	}
}

FVector AUnit::GetCellPos(FIntPoint p) {
	APPPGameMode* gameMode = Cast<APPPGameMode>(GetWorld()->GetAuthGameMode());
	if (gameMode) {
		return gameMode->GetCellPos(p);
	}
	return FVector();
}

void AUnit::AdjustPos(float x, float y, float z) {
	FVector loc = GetActorLocation();
	loc.X = loc.X + x;
	loc.Y = loc.Y + y;
	loc.Z = loc.Z + z;
	SetActorLocation(loc);
	if (InfoText)
		InfoText->SetActorRelativeLocation(GetActorLocation() + TEXT_OFFSET);
}

void AUnit::SetUnitInfo(int teamID, int maxHP, int hp) {	
	if (InfoText) {
		InfoText->GetTextRender()->SetTextRenderColor(teamID ? FColor::Red : FColor::Blue);	
	}
	HP = hp;
	MaxHP = maxHP;
	UStatBar* bar = GetStatBar();
	if (bar) {
		bar->SetColor(teamID ? FColor::Red : FColor::Blue);
		bar->SetHP(HP, MaxHP);
	}
}

UStatBar* AUnit::GetStatBar() {
	if (StatBarWidget) {
		return Cast<UStatBar>(StatBarWidget->GetUserWidgetObject());
	}
	return NULL;
}

UUnitCommandBar* AUnit::GetUnitCommandBar() {
	if (CommandBarWidget) {
		return Cast<UUnitCommandBar>(CommandBarWidget->GetUserWidgetObject());
	}
	return NULL;
}

void AUnit::SetInfoText(FText text) {	
	InfoText->GetTextRender()->SetText( text);
}

void AUnit::setPos(FIntPoint p)
{
	FVector loc = GetCellPos(p);
	loc.Z = loc.Z + 450.0f;

	SetActorLocation(loc);
	if(InfoText)
		InfoText->SetActorRelativeLocation(GetActorLocation() + TEXT_OFFSET);
}

void AUnit::RemoveFromGrid(LogicEventToken* token) {
	pEventToken = token;
	pEventToken->EndAfter = DUR_BASE;
}

void AUnit::MoveOnGrid(LogicEventToken* token) {
	pEventToken = token;
	pEventToken->EndAfter = DUR_BASE*( pEventToken->Path.Num() - 1 );
	pEventToken->Elapsed = 0;
}

void AUnit::OnAttacked(LogicEventToken* token, int value) {
	pEventToken = token;
	pEventToken->EndAfter = DUR_BASE * 2;
	HP -= value;	
	UStatBar* bar = GetStatBar();
	if (bar) {		
		bar->SetHP(HP, MaxHP);
		bar->SetHPDelta(value);
	}
}

void AUnit::CustomOnBeginMouseOver(UPrimitiveComponent* TouchedComponent)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Mouse Over"));
}

void AUnit::CustomOnClicked(AActor* Target, FKey ButtonPressed)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Magenta, TEXT("OnClicked"));
}
