// Cigarette.h — a cigarette whose length, char edge, ember glow and ash are all driven by parameters.
// The cigarette lies along local +X: filter end at X = 0, lit end towards +X.
// Replace SMOKESIM_API with your module's API macro (e.g. MYPROJECT_API).
// Add "Niagara" to PublicDependencyModuleNames in your .Build.cs.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cigarette.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UNiagaraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCigAshTooLong, float, AshLengthCm);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCigBurntOut);

UCLASS()
class SMOKESIM_API ACigarette : public AActor
{
	GENERATED_BODY()

public:
	ACigarette();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Light it (call when the lighter flame reaches the tip). */
	UFUNCTION(BlueprintCallable, Category = "Cigarette")
	void Light();

	/** Put it out (stubbing, or dropped after burning the fingers). */
	UFUNCTION(BlueprintCallable, Category = "Cigarette")
	void Extinguish();

	/** 0 = not drawing, 1 = full draw. Drive this from the "hold to inhale" input. */
	UFUNCTION(BlueprintCallable, Category = "Cigarette")
	void SetInhale(float InInhale);

	/** Removes the ash column; returns how long it was (cm) so you can spawn the falling ash piece. */
	UFUNCTION(BlueprintCallable, Category = "Cigarette")
	float DropAsh();

	/** World transform of the ash column centre — spawn the physics ash chunk here when it breaks off. */
	UFUNCTION(BlueprintPure, Category = "Cigarette")
	FTransform GetAshWorldTransform() const;

	// ---------------- components ----------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Filter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Paper;     // full length, burnt part masked in the material
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> EmberCap;  // glowing disc at the burn front
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Ash;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USceneComponent> BurnPoint;      // moves with the burn front
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UPointLightComponent> EmberLight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UNiagaraComponent> SmokeThread;  // assign NS_CigThread in the Blueprint

	// ---------------- materials (assign in a Blueprint subclass) ----------------
	UPROPERTY(EditAnywhere, Category = "Cigarette|Materials") TObjectPtr<UMaterialInterface> PaperMaterial;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Materials") TObjectPtr<UMaterialInterface> EmberMaterial;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Materials") TObjectPtr<UMaterialInterface> AshMaterial;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Materials") TObjectPtr<UMaterialInterface> FilterMaterial;

	// ---------------- dimensions (cm) ----------------
	UPROPERTY(EditAnywhere, Category = "Cigarette|Shape") float FilterLength = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Shape") float PaperLength = 6.0f;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Shape") float Diameter = 0.78f;

	// ---------------- burning ----------------
	/** Fraction of the paper burnt per second while resting (~0.08%/s ≈ 20 min for a whole cigarette untouched). */
	UPROPERTY(EditAnywhere, Category = "Cigarette|Burn") float IdleBurnPerSecond = 0.0008f;
	/** Fraction burnt per second at full draw (a 1.5 s puff burns ~5–6%). */
	UPROPERTY(EditAnywhere, Category = "Cigarette|Burn") float InhaleBurnPerSecond = 0.037f;
	/** Ash length gained per cm of paper burnt. */
	UPROPERTY(EditAnywhere, Category = "Cigarette|Burn") float AshPerBurntCm = 0.9f;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Burn") FVector2D AshDropRangeCm = FVector2D(2.0f, 2.8f);
	/** BurnProgress at which the ember reaches the filter and burns the fingers. */
	UPROPERTY(EditAnywhere, Category = "Cigarette|Burn") float BurntOutAt = 0.95f;

	// ---------------- ember light ----------------
	UPROPERTY(EditAnywhere, Category = "Cigarette|Ember") float EmberCandelas = 0.6f;
	UPROPERTY(EditAnywhere, Category = "Cigarette|Ember") float InhaleGlowMultiplier = 3.5f;

	// ---------------- state ----------------
	UPROPERTY(BlueprintReadOnly, Category = "Cigarette|State") float BurnProgress = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Cigarette|State") float AshLength = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Cigarette|State") float Inhale = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Cigarette|State") bool bLit = false;
	UPROPERTY(BlueprintReadOnly, Category = "Cigarette|State") float AshDropThreshold = 2.4f;

	UPROPERTY(BlueprintAssignable) FCigAshTooLong OnAshTooLong;
	UPROPERTY(BlueprintAssignable) FCigBurntOut OnBurntOut;

protected:
	virtual void BeginPlay() override;

private:
	void Layout();
	void UpdateMaterials(float DeltaSeconds);
	float BurnFrontX() const { return FilterLength + PaperLength * (1.f - BurnProgress); }

	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> PaperMID;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> EmberMID;

	float SmoothedGlow = 0.f;
	float FlickerTime = 0.f;
	bool bAshEventFired = false;
	bool bBurntOutFired = false;
};
