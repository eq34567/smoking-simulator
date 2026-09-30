// Cigarette.cpp

#include "Cigarette.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Engine basic shapes are 100 units: cylinder is 100 tall along Z, 100 across, pivot at its centre.
	void PlaceAlongX(UStaticMeshComponent* C, float StartX, float Length, float Dia)
	{
		const float L = FMath::Max(Length, 0.01f);
		C->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));            // local Z -> along X
		C->SetRelativeLocation(FVector(StartX + L * 0.5f, 0.f, 0.f));
		C->SetRelativeScale3D(FVector(Dia / 100.f, Dia / 100.f, L / 100.f));
	}
}

ACigarette::ACigarette()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	auto MakePart = [this](const TCHAR* Name, UStaticMesh* Mesh)
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		C->SetupAttachment(Root);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(true);
		if (Mesh) C->SetStaticMesh(Mesh);
		return C;
	};
	UStaticMesh* Cyl = CylinderMesh.Succeeded() ? CylinderMesh.Object : nullptr;
	UStaticMesh* Sph = SphereMesh.Succeeded() ? SphereMesh.Object : nullptr;

	Filter   = MakePart(TEXT("Filter"), Cyl);
	Paper    = MakePart(TEXT("Paper"), Cyl);
	Ash      = MakePart(TEXT("Ash"), Cyl);
	EmberCap = MakePart(TEXT("EmberCap"), Sph);

	BurnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BurnPoint"));
	BurnPoint->SetupAttachment(Root);

	EmberLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("EmberLight"));
	EmberLight->SetupAttachment(BurnPoint);
	EmberLight->SetRelativeLocation(FVector(1.5f, 0.f, 0.f));      // a little past the tip so the fingers don't blow out
	EmberLight->SetIntensityUnits(ELightUnits::Candelas);
	EmberLight->SetIntensity(0.f);
	EmberLight->SetLightColor(FLinearColor(1.f, 0.36f, 0.08f));
	EmberLight->SetAttenuationRadius(35.f);
	EmberLight->SetSourceRadius(0.3f);
	EmberLight->SetCastShadows(true);                               // tiny finger shadows sell it; turn off if too costly

	SmokeThread = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeThread"));
	SmokeThread->SetupAttachment(BurnPoint);
	SmokeThread->SetAutoActivate(false);
}

void ACigarette::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Layout();
}

void ACigarette::BeginPlay()
{
	Super::BeginPlay();

	if (FilterMaterial) Filter->SetMaterial(0, FilterMaterial);
	if (AshMaterial) Ash->SetMaterial(0, AshMaterial);
	if (PaperMaterial) PaperMID = Paper->CreateDynamicMaterialInstance(0, PaperMaterial);
	if (EmberMaterial) EmberMID = EmberCap->CreateDynamicMaterialInstance(0, EmberMaterial);

	AshDropThreshold = FMath::FRandRange(AshDropRangeCm.X, AshDropRangeCm.Y);
	Layout();
	UpdateMaterials(0.f);
}

void ACigarette::Light()
{
	if (bLit || BurnProgress >= BurntOutAt) return;
	bLit = true;
	if (SmokeThread->GetAsset()) SmokeThread->Activate(true);
}

void ACigarette::Extinguish()
{
	bLit = false;
	Inhale = 0.f;
	SmokeThread->Deactivate();   // lets the existing thread drift away naturally
}

void ACigarette::SetInhale(float InInhale)
{
	Inhale = FMath::Clamp(InInhale, 0.f, 1.f);
}

float ACigarette::DropAsh()
{
	const float Removed = AshLength;
	AshLength = 0.f;
	bAshEventFired = false;
	AshDropThreshold = FMath::FRandRange(AshDropRangeCm.X, AshDropRangeCm.Y);
	Layout();
	return Removed;
}

FTransform ACigarette::GetAshWorldTransform() const
{
	return Ash->GetComponentTransform();
}

void ACigarette::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bLit && BurnProgress < 1.f)
	{
		const float Rate = FMath::Lerp(IdleBurnPerSecond, InhaleBurnPerSecond, Inhale);
		const float Burnt = FMath::Min(Rate * DeltaSeconds, 1.f - BurnProgress);
		BurnProgress += Burnt;
		AshLength += Burnt * PaperLength * AshPerBurntCm;

		if (AshLength >= AshDropThreshold && !bAshEventFired)
		{
			bAshEventFired = true;
			OnAshTooLong.Broadcast(AshLength);        // the gameplay side decides: flick, or it falls on you
		}
		if (BurnProgress >= BurntOutAt && !bBurntOutFired)
		{
			bBurntOutFired = true;
			OnBurntOut.Broadcast();                   // burn the fingers
		}
		Layout();
	}

	// The side-stream thread only rises while you're NOT drawing — during a draw the smoke goes in, not up.
	if (SmokeThread->GetAsset())
	{
		const bool bShouldSmoke = bLit && Inhale < 0.15f;
		if (bShouldSmoke && !SmokeThread->IsActive()) SmokeThread->Activate();
		else if (!bShouldSmoke && SmokeThread->IsActive()) SmokeThread->Deactivate();
		SmokeThread->SetVariableFloat(FName("Inhale"), Inhale);
	}

	UpdateMaterials(DeltaSeconds);
}

void ACigarette::Layout()
{
	const float Front = BurnFrontX();

	PlaceAlongX(Filter, 0.f, FilterLength, Diameter * 1.01f);
	PlaceAlongX(Paper, FilterLength, PaperLength, Diameter);         // burnt part is masked away by the material

	// glowing ember disc sitting in the burn front
	EmberCap->SetRelativeRotation(FRotator::ZeroRotator);
	EmberCap->SetRelativeLocation(FVector(Front, 0.f, 0.f));
	EmberCap->SetRelativeScale3D(FVector(0.25f, Diameter * 0.97f, Diameter * 0.97f) / 100.f);

	// ash column grows out of the front; a slight taper and droop are left to the ash material (WPO)
	const bool bHasAsh = AshLength > 0.02f;
	Ash->SetVisibility(bHasAsh);
	if (bHasAsh) PlaceAlongX(Ash, Front, AshLength, Diameter * 0.94f);

	BurnPoint->SetRelativeLocation(FVector(Front, 0.f, 0.f));
}

void ACigarette::UpdateMaterials(float DeltaSeconds)
{
	FlickerTime += DeltaSeconds;

	// Glow target: resting ember ~1, full draw ~InhaleGlowMultiplier; eased so it breathes rather than snaps.
	const float Target = bLit ? FMath::Lerp(1.f, InhaleGlowMultiplier, Inhale) : 0.f;
	SmoothedGlow = FMath::FInterpTo(SmoothedGlow, Target, DeltaSeconds, bLit ? 6.f : 2.f);
	const float Flicker = 1.f + 0.06f * FMath::Sin(FlickerTime * 9.f) + 0.04f * FMath::Sin(FlickerTime * 23.f + 1.3f);
	const float Glow = SmoothedGlow * Flicker;

	EmberLight->SetIntensity(EmberCandelas * Glow);

	const FVector Axis = GetActorForwardVector();                                   // filter -> tip
	const FVector FrontWS = GetActorTransform().TransformPosition(FVector(BurnFrontX(), 0.f, 0.f));

	if (PaperMID)
	{
		PaperMID->SetVectorParameterValue(TEXT("BurnFront"), FLinearColor(FrontWS));
		PaperMID->SetVectorParameterValue(TEXT("CigAxis"), FLinearColor(Axis));
		PaperMID->SetScalarParameterValue(TEXT("Glow"), Glow);
		PaperMID->SetScalarParameterValue(TEXT("Lit"), bLit ? 1.f : 0.f);
	}
	if (EmberMID)
	{
		EmberMID->SetScalarParameterValue(TEXT("Glow"), Glow);
	}
}
