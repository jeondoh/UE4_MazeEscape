// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BulletHitInterface.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class MAZEESCAPE_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void DoDamage(class AMazePlayer* Player);

	void SpawnBlood(class AMazePlayer* Player, FName SocketName);
	
	void Die();
	
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	UFUNCTION()
	void ResetHitReactTimer();

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// BulletHitInterface에서 Override
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Player, AController* InstigatorController) override;
	
	// 데미지 피해량
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	void ShowHealthBar_Implementation();
	
	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);

	UFUNCTION()
	void DestoryEnemy();

private:
	// 변수 초기화
	void InitalizedData();

	// 총에 맞았을때 파티클
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Particles", meta=(AllowPrivateAccess=true))
	class UParticleSystem* ImpactParticles;
	// 총에 맞았을때 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Sounds", meta=(AllowPrivateAccess=true))
	class USoundCue* ImpactSound;
	// 현재체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float Health;
	// 최대체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float MaxHealth;
	// 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float BaseDamage;
	// 좌측 무기 소켓명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	FName LeftWeaponSocket;
	// 좌측 무기 소켓명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	FName RightWeaponSocket;
	// 머리 스켈레톤 이름 (헤드샷을 위함)	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	FString HeadBone;
	// 체력바 보이는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HealthBarDisplayTime;
	// 체력바 타이머
	FTimerHandle HealthBarTimer;
	// 죽음여부
	UPROPERTY(VisibleAnywhere, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	bool bDying;
	// 죽은이후 일정 시간 이후 destory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float DeathTime;
	FTimerHandle DeathTimer;
	// 위젯 & 벡터 Array
	UPROPERTY(VisibleAnywhere, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	TMap<UUserWidget*, FVector> HitNumbers;
	// 히트 위젯이 화면에서 제거되는 시간
	UPROPERTY(EditAnywhere, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitNumberDestroyTime;
	// 몬스터 사망여부
	bool IsDead;
	// HitNumbers에 저장된 데미지값 Enemy 위치에 보여주기
	void UpdateHitNumbers();
	// 피해량 랜덤
	float RandomizationDamage(float Damage);

	/**************************************************************************************************/
	/** 애니메이션 **/

	// 히트 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Animate", meta=(AllowPrivateAccess=true))
	UAnimMontage* HitMontage;
	// 히트 몽타주 간격 시간
	FTimerHandle HitReactTimer;
	// 히트 몽타주 재생 여부
	bool bCanHitReact;
	// 히트 애니메이션 재생 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitReactTimeMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitReactTimeMax;
	// 사망 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Animate", meta=(AllowPrivateAccess=true))
	UAnimMontage* DeathMontage;

	// 공격 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Animate", meta=(AllowPrivateAccess=true))
	UAnimMontage* AttackMontage;
	// 공격 몽타주 이름
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	/**************************************************************************************************/
	/** 인공지능 **/

	// Enemy의 행동 AI
	UPROPERTY(EditAnywhere, Category="Enemy|BehaviorTree", meta=(AllowPrivateAccess=true))
	class UBehaviorTree* BehaviorTree;
	// 적이 이동할 지점
	// MakeEditWidget = 해당 엑터의 위치를 중심으로 한 로컬 위치
	UPROPERTY(EditAnywhere, Category="Enemy|BehaviorTree", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	FVector PatrolPoint;
	UPROPERTY(EditAnywhere, Category="Enemy|BehaviorTree", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	FVector PatrolPoint2;
	UPROPERTY()
	class AEnemyController* EnemyController;
	// Enemy 클래스 AI 설정
	void SetEnemyAIController();
	// 어그로 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Component", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	class USphereComponent* AgroSphere;
	// 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Component", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	class USphereComponent* CombatRangeSphere;
	// 우측 무기 Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enemy|Component", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	class UBoxComponent* LeftWeaponCollision;
	// 좌측 무기 Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enemy|Component", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	class UBoxComponent* RightWeaponCollision;
	// 히트 애니메이션 재생 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	bool bStunned;
	// 기절할 수치(확률) 0 : 스턴 X  1 : 스턴 O
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	float StunChance;
	// CombatRangeSphere에 따라 공격 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	bool bInAttackRange;
	// 공격가능 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	bool bCanAttack2;
	// 공격 간격 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	float AttackWaitTime2;
	// 공격 간격 타이머
	FTimerHandle AttackWaitTimer;
	// AgroSphere에 오버랩 되었을때
	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// AgroSphere 오버랩 끝났을 경우
	UFUNCTION()
	void AgroSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);
	// 캐릭터 기절
	void StunPlayer(AMazePlayer* Player);

	void ResetCanAttack();

	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
			
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 무기 충돌 활성화/비활성화
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	/**************************************************************************************************/
	/** 아이템 드롭 **/

	// 무기 클래스
	UPROPERTY(EditAnywhere, Category = "Enemy|PlayerWeapon", meta=(AllowPrivateAccess=true, MakeEditWidget=true)) 
	TSubclassOf<class AWeapon> WeaponClass;

	// 아이템 드롭 여부
	UPROPERTY(EditAnywhere, Category = "Enemy|PlayerWeapon", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	bool IsDropItem;

	// 아이템 드롭 사운드
	UPROPERTY(EditAnywhere, Category = "Enemy|PlayerWeapon", meta=(AllowPrivateAccess=true, MakeEditWidget=true))
	class USoundCue* DropSound;
	
	UFUNCTION()
	void DropItem();
	
	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE FString GetHeadBone() const {return HeadBone;}
	
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const {return BehaviorTree;}

	FORCEINLINE bool GetIsDead() const {return IsDead;}
	
};
