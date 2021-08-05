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
	virtual void BulletHit_Implementation(FHitResult HitResult) override;
	
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float Health;
	// 최대체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float MaxHealth;
	// 머리 스켈레톤 이름 (헤드샷을 위함)	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	FString HeadBone;
	// 체력바 보이는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HealthBarDisplayTime;
	// 체력바 타이머
	FTimerHandle HealthBarTimer;
	// 히트 / 사망 애니메이션이 포함된 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	UAnimMontage* HitMontage;
	// 몽타주 간격 시간
	FTimerHandle HitReactTimer;
	// 몽타주 재생 여부
	bool bCanHitReact;
	// 히트 애니메이션 재생 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitReactTimeMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitReactTimeMax;
	// 위젯 & 벡터 Array
	UPROPERTY(VisibleAnywhere, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	TMap<UUserWidget*, FVector> HitNumbers;
	// 히트 위젯이 화면에서 제거되는 시간
	UPROPERTY(EditAnywhere, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float HitNumberDestroyTime;
	// HitNumbers에 저장된 데미지값 Enemy 위치에 보여주기
	void UpdateHitNumbers();

	/**************************************************************************************************/
	/** 인공지능 **/

	// Enemy의 행동 AI
	UPROPERTY(EditAnywhere, Category="Enemy|Behavior Tree", meta=(AllowPrivateAccess=true))
	class UBehaviorTree* BehaviorTree;
	

// Getter & Setter
public:
	FORCEINLINE FString GetHeadBone() const {return HeadBone;}
	
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const {return BehaviorTree;}
	
};
