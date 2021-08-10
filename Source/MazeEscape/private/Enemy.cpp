// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "EnemyController.h"
#include "MazePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 초기화
	InitalizedData();

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereEndOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	SetEnemyAIController();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::DestoryEnemy()
{
	Destroy();
}

void AEnemy::InitalizedData()
{
	Health = 100.f;
	MaxHealth = 100.f;
	BaseDamage = 50.f;
	HealthBarDisplayTime = 4.f;
	HitReactTimeMin = 0.5f;
	HitReactTimeMax = 0.75f;
	bCanHitReact = true;
	bInAttackRange = false;
	HitNumberDestroyTime = 1.5f;
	bStunned = false;
	StunChance = 0.5f;
	AttackRMB = TEXT("AttackRMB");
	AttackLFast = TEXT("AttackLFast");
	AttackRFast = TEXT("AttackRFast");
	AttackCFast = TEXT("AttackCFast");
	AttackL = TEXT("AttackL");
	AttackR = TEXT("AttackR");
	LeftWeaponSocket = TEXT("FX_Trail_L_01");
	RightWeaponSocket = TEXT("FX_Trail_R_01");
	bCanAttack2 = true;
	AttackWaitTime2 = 1.5f;
	bDying = false;
	DeathTime = 3.f;
	IsDropItem = false;
	IsDead = false;
	isBoss = false;
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Player, AController* InstigatorController)
{
	// Enemy 사망시 실행 X
	if(IsDead) return;
	// 사운드
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	// 파티클
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles,
			HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(EnemyController)
	{
		// 타겟 어그로
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	}
	
	Health -= DamageAmount;

	if(Health <= 0.f)
	{
		IsDead = true;
		Health = 0.f;
		Die();
	}
	if(bDying) return DamageAmount;
	// 적 체력바 보이기(쏘고나서 이후 HealthBarDisplayTime 동안만) 
	ShowHealthBar();
	// 확률적으로 애니메이션 재생 / 애니메이션 재생으로 인해 움직이지 못함(스턴)
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if(Stunned <= StunChance)
	{
		// 히트/사망 애니메이션 몽타주
		PlayHitMontage(FName("HitReactFront"), 1.0f);
		SetStunned(true);
	}
	
	return DamageAmount;
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	if(bDying) return;
	bDying = true;
	// 체력바 숨기기
	HideHealthBar();
	// 사망 몽타주 실행
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	if(EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Death"), true);
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), nullptr);
		EnemyController->StopMovement();
	}
	// 아이템 드롭
	DropItem();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if(bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		// 빠르게 총을 쏠때 히트 애니메이션 재생 간격을 주기 위해 
		bCanHitReact = false;
		const float HitReactTime{FMath::FRandRange(HitReactTimeMin, HitReactTimeMax)};
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
	}
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber); // 입력 매개변수를 사용해 바인딩 가능
	GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for(auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{HitPair.Key};
		const FVector Location{HitPair.Value};
		FVector ResultLocation = FVector(Location.X, Location.Y, Location.Z + 30.f);
		FVector2D ScreenPosition;

		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), ResultLocation, ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}	
}

float AEnemy::RandomizationDamage(float Damage)
{
	return FMath::RandRange(Damage-10.f, Damage+30.f);
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	// 공격간격
	bCanAttack2 = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime2);
	if(EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

FName AEnemy::GetSectionName()
{
	return isBoss ? GetBossAttackSectionName() : GetAttackSectionName();
}


FName AEnemy::GetAttackSectionName()
{
	FName SectionName;
	const int32 Section{FMath::RandRange(1, 4)};
	switch(Section)
	{
	case 1:
		SectionName = AttackLFast;
		break;
	case 2:
		SectionName = AttackRFast;
		break;
	case 3:
		SectionName = AttackL;
		break;
	case 4:
		SectionName = AttackR;
		break;
	}
	return SectionName;
}

FName AEnemy::GetBossAttackSectionName()
{
	FName SectionName;
	const int32 Section{FMath::RandRange(1, 6)};
	switch(Section)
	{
	case 1:
		SectionName = AttackLFast;
		break;
	case 2:
		SectionName = AttackRFast;
		break;
	case 3:
		SectionName = AttackCFast;
		break;
	case 4:
		SectionName = AttackRMB;
		break;
	case 5:
		SectionName = AttackR;
		break;
	case 6:
		SectionName = AttackRMB;
		break;
	}
	return SectionName;
}


void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestoryEnemy, DeathTime);
}

void AEnemy::SetEnemyAIController()
{
	EnemyController = Cast<AEnemyController>(GetController());
	// 위치변환 (로컬 -> 월드 공간)
	const FVector WorldPatorlPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatorlPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
	
	// Blackboard에 있는 변수
	if(EnemyController)
	{
		// ValueAs '' 해당 타입의 변수값 SET 
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true); // 공격가능여부
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatorlPoint);		
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatorlPoint2);		
		EnemyController->RunBehaviorTree(BehaviorTree); // 루트노드에서 순차 실행
	}
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == nullptr) return;

	auto Player = Cast<AMazePlayer>(OtherActor);
	
	if(Player && EnemyController)
	{
		if(EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Player);			
		}
	}
	
}

void AEnemy::AgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor == nullptr) return;

	auto Player = Cast<AMazePlayer>(OtherActor);
	if(Player && EnemyController)
	{
		if(EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);			
		}
	}
	
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if(EnemyController)
	{
		if(EnemyController->GetBlackboardComponent())
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
		}
	}
}

void AEnemy::StunPlayer(AMazePlayer* Player)
{
	// 웅크린 상태에서 스턴시 애니메이션이 이상함 > 임시조치
	if(Player && !Player->GetCrouching())
	{
		const float Stun{FMath::FRandRange(0.f, 1.f)};
		if(Stun <= Player->GetStunChance())
		{
			Player->SetPlayerStunned(true);
			Player->Stun();
		}
	}
}

void AEnemy::DoDamage(AMazePlayer* Player)
{
	if(Player->GetHealth() == 0) return;
	
	UGameplayStatics::ApplyDamage(Player, RandomizationDamage(BaseDamage), EnemyController, this, UDamageType::StaticClass());
	// 스턴시에는 몽타주에서 사운드 실행
	if(!Player->GetPlayerStunned() && Player->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Player->GetMeleeImpactSound(), GetActorLocation());
	}
}

void AEnemy::SpawnBlood(AMazePlayer* Player, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{GetMesh()->GetSocketByName(SocketName)};
	if(TipSocket)
	{
		const FTransform SocketTransfrom{TipSocket->GetSocketTransform(GetMesh())};
		if(Player->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Player->GetBloodParticles(), Player->GetTransform());
			// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Player->GetBloodParticles(), SocketTransfrom);
		}
	}
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == nullptr) return;
	auto Player = Cast<AMazePlayer>(OtherActor);
	if(Player)
	{
		bInAttackRange = true;
		if(EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}		
	}
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor == nullptr) return;
	auto Player = Cast<AMazePlayer>(OtherActor);
	if(Player)
	{
		bInAttackRange = false;
		if(EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Player = Cast<AMazePlayer>(OtherActor);
	if(Player)
	{
		StunPlayer(Player); // 확률 스턴
		DoDamage(Player); // 데미지 & 사운드 재생
		SpawnBlood(Player, LeftWeaponSocket); // 파티클 효과
	}
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto Player = Cast<AMazePlayer>(OtherActor);
	if(Player)
	{
		StunPlayer(Player); // 확률 스턴
		DoDamage(Player); // 데미지 & 사운드 재생
		SpawnBlood(Player, RightWeaponSocket); // 파티클 효과
	}
}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ResetCanAttack()
{
	bCanAttack2 = true;
	if(EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::DropItem()
{
	if(IsDropItem && WeaponClass)
	{
		FActorSpawnParameters Param;
		Param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector ActorLocation = GetActorLocation();
		FVector ItemLocation = FVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + 70); 
		
		GetWorld()->SpawnActor<AWeapon>(WeaponClass,
			ItemLocation,
			GetActorRotation(), Param);
		// 아이템 드롭 소리
		if(DropSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DropSound, ActorLocation);
		}
	}
}

FRotator AEnemy::GetLookAtRotationYaw(FVector ActorLocation)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ActorLocation);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	// Target 방향으로 스무스하게 바라봄
	return FMath::RInterpTo(GetActorRotation(), LookAtRotationYaw, GetWorld()->GetDeltaSeconds(), 200.f);
}

