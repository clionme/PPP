// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextRenderActor.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "Widget/StatBar.h"
#include "Widget/UnitCommandBar.h"
#include "Unit.generated.h"

UCLASS()
class PPP_API AUnit : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualMesh;

	UFUNCTION()
	void CustomOnBeginMouseOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void CustomOnClicked(AActor* Target, FKey ButtonPressed);

public:	
	// Sets default values for this actor's properties
	AUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UWidgetComponent* StatBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UWidgetComponent* CommandBarWidget;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Destroy();
	void DestroyInfoText();

	void AdjustPos(float x, float y, float z);
	void SetUnitInfo(int, int, int);
	void SetInfoText(FText);

	void SetCommand(bool, bool);
	void ClearCommand();

	UStatBar* GetStatBar();
	UUnitCommandBar* GetUnitCommandBar();

	void setPos(FIntPoint);

	void RemoveFromGrid( LogicEventToken* );
	void MoveOnGrid(LogicEventToken*);	
	void OnAttacked(LogicEventToken*, int);

	ATextRenderActor* InfoText;	

	int ID = -1;
	int HP;
	int MaxHP;
	LogicEventToken* pEventToken;
};
