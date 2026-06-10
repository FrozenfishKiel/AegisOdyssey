#include "AegisOdyssey/SkillSystem/Execution/Runtime/AOSkillProjectile_Fireball.h"

#include "AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.h"
#include "AegisOdyssey/SkillSystem/Execution/Definitions/AOSkillExecutionDefinition_Projectile.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillProjectile_Fireball)

AAOSkillProjectile_Fireball::AAOSkillProjectile_Fireball(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(24.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	VisualMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMeshComponent"));
	VisualMeshComponent->SetupAttachment(CollisionComponent);
	VisualMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Flight presentation lives on the projectile itself so BP children can swap Niagara assets directly.
	FlightEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlightEffectComponent"));
	FlightEffectComponent->SetupAttachment(CollisionComponent);
	FlightEffectComponent->SetAutoActivate(true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 1600.0f;
	ProjectileMovementComponent->MaxSpeed = 1600.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AAOSkillProjectile_Fireball::InitializeFromSkillAbility(UAOSkillGameplayAbility* InOwningSkillAbility)
{
	OwningSkillAbility = InOwningSkillAbility;
	SpawnLocation = GetActorLocation();
	MaxTravelDistance = 0.0f;

	// Runtime projectile parameters still come from the projectile execution definition.
	if (OwningSkillAbility.IsValid())
	{
		if (const UAOSkillProjectileExecutionDefinition* ExecutionDefinition = Cast<UAOSkillProjectileExecutionDefinition>(OwningSkillAbility->GetSkillExecutionDefinitionFromCurrentSourceObject()))
		{
			const float AbilityLevel = OwningSkillAbility->GetAbilityLevel();
			CollisionComponent->SetSphereRadius(FMath::Max(1.0f, ExecutionDefinition->CollisionRadius.GetValueAtLevel(AbilityLevel)));
			ProjectileMovementComponent->InitialSpeed = FMath::Max(0.0f, ExecutionDefinition->ProjectileSpeed.GetValueAtLevel(AbilityLevel));
			ProjectileMovementComponent->MaxSpeed = ProjectileMovementComponent->InitialSpeed;
			MaxTravelDistance = FMath::Max(0.0f, ExecutionDefinition->MaxTravelDistance.GetValueAtLevel(AbilityLevel));
		}
	}

	// Ignore the caster so the fireball does not immediately overlap its owner on spawn.
	if (OwningSkillAbility.IsValid())
	{
		if (AActor* AvatarActor = OwningSkillAbility->GetAvatarActorFromActorInfo())
		{
			CollisionComponent->IgnoreActorWhenMoving(AvatarActor, true);
		}
	}
}

void AAOSkillProjectile_Fireball::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
}

void AAOSkillProjectile_Fireball::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTravelDistanceLimit();
}

void AAOSkillProjectile_Fireball::OnSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleTargetHit(OtherActor, &SweepResult);
}

void AAOSkillProjectile_Fireball::HandleTargetHit(AActor* TargetActor, const FHitResult* HitResult)
{
	if (bHasResolvedHit || !TargetActor || !OwningSkillAbility.IsValid())
	{
		return;
	}

	AActor* SourceActor = OwningSkillAbility->GetAvatarActorFromActorInfo();
	if (!SourceActor || ShouldIgnoreHitActor(TargetActor))
	{
		return;
	}

	// 投射体命中和伤害结算只由服务端裁定。
	// 客户端收到的重叠只用于本地物理查询，不允许自己先炸。
	if (!HasAuthority())
	{
		return;
	}

	bHasResolvedHit = true;

	TArray<AActor*> HitTargets;
	CollectImpactTargets(TargetActor, HitTargets);

	if (const UAOSkillProjectileExecutionDefinition* ExecutionDefinition =
		Cast<UAOSkillProjectileExecutionDefinition>(OwningSkillAbility->GetSkillExecutionDefinitionFromCurrentSourceObject()))
	{
		const FVector ImpactNormal = GetActorForwardVector();
		OwningSkillAbility->ExecuteSkillCue(
			ExecutionDefinition->CueConfig.ExecuteCueTag,
			OwningSkillAbility->BuildSkillCueParameters(GetActorLocation(), ImpactNormal, this));
	}

	OwningSkillAbility->RouteSkillEffectApplicationFromRuntimeActor(HitTargets, HitResult, this);
	Destroy();
}

void AAOSkillProjectile_Fireball::UpdateTravelDistanceLimit()
{
	if (bHasResolvedHit || MaxTravelDistance <= 0.0f)
	{
		return;
	}

	const float TravelDistanceSquared = FVector::DistSquared(SpawnLocation, GetActorLocation());
	if (TravelDistanceSquared >= FMath::Square(MaxTravelDistance))
	{
		Destroy();
	}
}

void AAOSkillProjectile_Fireball::CollectImpactTargets(AActor* PrimaryTarget, TArray<AActor*>& OutTargets) const
{
	OutTargets.Reset();

	if (!PrimaryTarget || !OwningSkillAbility.IsValid())
	{
		return;
	}

	const UAOSkillProjectileExecutionDefinition* ExecutionDefinition =
		Cast<UAOSkillProjectileExecutionDefinition>(OwningSkillAbility->GetSkillExecutionDefinitionFromCurrentSourceObject());
	if (!ExecutionDefinition)
	{
		OutTargets.Add(PrimaryTarget);
		return;
	}

	const float AbilityLevel = OwningSkillAbility->GetAbilityLevel();
	const float ExplosionRadius = FMath::Max(0.0f, ExecutionDefinition->ExplosionRadius.GetValueAtLevel(AbilityLevel));
	if (ExplosionRadius <= 0.0f)
	{
		OutTargets.Add(PrimaryTarget);
		return;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(const_cast<AAOSkillProjectile_Fireball*>(this));
	if (AActor* SourceActor = OwningSkillAbility->GetAvatarActorFromActorInfo())
	{
		IgnoreActors.Add(SourceActor);
	}

	TArray<AActor*> OverlappedActors;
	const FVector ExplosionCenter = GetActorLocation();
	if (!UKismetSystemLibrary::SphereOverlapActors(
		this,
		ExplosionCenter,
		ExplosionRadius,
		ObjectTypes,
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors))
	{
		OutTargets.Add(PrimaryTarget);
		return;
	}

	for (AActor* TargetActor : OverlappedActors)
	{
		if (TargetActor)
		{
			OutTargets.AddUnique(TargetActor);
		}
	}

	OutTargets.AddUnique(PrimaryTarget);

	if (OwningSkillAbility->CanRuntimeActorDrawSkillDebug())
	{
		const FAOSkillDebugConfig& DebugConfig = ExecutionDefinition->DebugConfig;
		DrawDebugSphere(
			GetWorld(),
			ExplosionCenter,
			ExplosionRadius,
			20,
			DebugConfig.SecondaryDebugColor.ToFColor(true),
			false,
			DebugConfig.DebugDrawDuration);
	}
}

bool AAOSkillProjectile_Fireball::ShouldIgnoreHitActor(const AActor* TargetActor) const
{
	if (!TargetActor || !OwningSkillAbility.IsValid())
	{
		return true;
	}

	const AActor* SourceActor = OwningSkillAbility->GetAvatarActorFromActorInfo();
	if (!SourceActor)
	{
		return true;
	}

	if (TargetActor == SourceActor)
	{
		return true;
	}

	// 挂在施法者身上的武器、表现件、子 Actor 也都不应该把火球提前引爆。
	const AActor* CurrentOwner = TargetActor;
	while (CurrentOwner != nullptr)
	{
		if (CurrentOwner == SourceActor)
		{
			return true;
		}

		CurrentOwner = CurrentOwner->GetOwner();
	}

	if (const APawn* SourcePawn = Cast<APawn>(SourceActor))
	{
		if (TargetActor == SourcePawn->GetController())
		{
			return true;
		}
	}

	return false;
}
