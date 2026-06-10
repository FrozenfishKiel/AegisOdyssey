#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOSkillProjectile_Fireball.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAOSkillGameplayAbility;
class UAOSkillProjectileExecutionDefinition;

/**
 * Runtime projectile for Fireball.
 *
 * This actor owns flight, overlap hit handling, impact target collection,
 * impact VFX playback, and self cleanup.
 */
UCLASS()
class AEGISODYSSEY_API AAOSkillProjectile_Fireball : public AActor
{
	GENERATED_BODY()

public:
	AAOSkillProjectile_Fireball(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeFromSkillAbility(UAOSkillGameplayAbility* InOwningSkillAbility);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	void HandleTargetHit(AActor* TargetActor, const FHitResult* HitResult);
	void CollectImpactTargets(AActor* PrimaryTarget, TArray<AActor*>& OutTargets) const;
	void UpdateTravelDistanceLimit();
	bool ShouldIgnoreHitActor(const AActor* TargetActor) const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisualMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> FlightEffectComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent = nullptr;

	UPROPERTY()
	TWeakObjectPtr<UAOSkillGameplayAbility> OwningSkillAbility;

	UPROPERTY()
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY()
	float MaxTravelDistance = 0.0f;

	UPROPERTY()
	bool bHasResolvedHit = false;
};
