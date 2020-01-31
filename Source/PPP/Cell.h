// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cell.generated.h"

UCLASS()
class PPP_API ACell : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualMesh;

	UFUNCTION()
	void CustomOnBeginMouseOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void CustomOnEndMouseOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void CustomOnClicked(AActor* Target, FKey ButtonPressed);

	FIntPoint pos;
	bool MouseOver = false;
	bool Reachable = false;
	bool Actable = false;

public:	
	// Sets default values for this actor's properties
	ACell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void setPos(FIntPoint);
	FIntPoint getPos();

	void SetReachable(bool);
	void SetActable(bool);
};
