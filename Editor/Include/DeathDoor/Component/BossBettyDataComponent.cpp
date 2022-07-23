#include "BossBettyDataComponent.h"
#include "GameBehaviorTree.h"
#include "GameStateComponent.h"
#include "Component/AnimationMeshComponent.h"
#include "Component/ColliderBox3D.h"
#include "Component/ColliderSphere.h"
#include "Collision/Collision.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "BossBettyDataComponent.h"
#include "../DataManager.h"
#include "Component/CameraComponent.h"
#include "GameObject/GameObject.h"

CBossBettyDataComponent::CBossBettyDataComponent() :
	m_ThrowFarAttackEnable(false),
	m_FarAttackType(BossBettyFarAttackType::JumpSmash),
    m_ChangeDirLimitAngle(5.f)
{
    m_ComponentType = Component_Type::ObjectComponent;
    SetTypeID<CBossBettyDataComponent>();
}

CBossBettyDataComponent::CBossBettyDataComponent(const CBossBettyDataComponent& com) :
    CMonsterDataComponent(com)
{
}

CBossBettyDataComponent::~CBossBettyDataComponent()
{
}

void CBossBettyDataComponent::Start()
{
    CMonsterDataComponent::Start();

    m_Data = CDataManager::GetInst()->GetObjectData("BossBetty");

    // Move Speed 는 CSV OBJ Data 로 세팅한다.
    m_OriginMoveSpeed = m_Data.MoveSpeed;
    m_CurMoveSpeed = m_Data.MoveSpeed;

    // HitBox 에 콜백을 걸어준다.

    // Current Animation 은 Idle 로 세팅한다.
    CAnimationSequenceInstance* AnimInst = dynamic_cast<CAnimationMeshComponent*>(m_Object->GetRootComponent())->GetAnimationInstance();
    AnimInst->ChangeAnimation("Idle");

    // m_BossBettySpinCollider 를 Object 의 Component List 에 추가한다.
    // - 그리고 Spin 중간에, Collide 시 Spin Collider Animation 으로 바꾸는 Callback도 세팅한다.
    // - 처음에는 비활성화 시킨다.
    m_BossBettySpinCollider = (CColliderBox3D*)(m_Object->FindComponent("BossBettySpinCollide"));

    if (m_BossBettySpinCollider)
    {
        m_BossBettySpinCollider->Enable(false);

        m_BossBettySpinCollider->AddCollisionCallback(Collision_State::Stay,
            this, &CBossBettyDataComponent::OnChangeFromSpinToSpinCollideWhenCollide);

        m_BossBettySpinCollider->AddCollisionCallback(Collision_State::Stay,
            this, &CBossBettyDataComponent::OnPreventGoingOutOfMapSurroundingCollider);
    }

    m_MeleeAttackCollider = dynamic_cast<CColliderBox3D*>((m_Object->FindComponent("BossBettyAttackCollider")));

    if (m_MeleeAttackCollider)
    {
        m_MeleeAttackCollider->SetExtent(3.f, 3.f, 3.f);

        m_MeleeAttackCollider->Enable(false);

        m_MeleeAttackCollider->AddCollisionCallback(Collision_State::Begin,
            (CMonsterDataComponent*)this, &CMonsterDataComponent::OnHitMeleeAttack);
        
    }

    // 근거리 사정 거리 판별 Square Pos 위치 만들기 
    //  0: 왼쪽 하단, 1 : 왼쪽 상단, 2 : 오른쪽 상단, 3 : 오른쪽 하단
    const Vector3& ObjectWorldScale = m_Object->GetRootComponent()->GetWorldScale();
    const Vector3& MeshOriginScale = dynamic_cast<CAnimationMeshComponent*>(m_Object->GetRootComponent())->GetMeshSize();
    // const Vector3& FinalMeshScale = ObjectWorldScale * MeshOriginScale;
    const Vector3& FinalMeshScale = Vector3(5.f, 0.f, 5.f);

    m_PunchLeftSquarePos[0] = Vector3(FinalMeshScale.x * -1 * 0.5f, 0.f, FinalMeshScale.z * 0.5f);
    m_PunchLeftSquarePos[1] = Vector3(FinalMeshScale.x * -1 * 0.5f, 0.f, FinalMeshScale.z);
    m_PunchLeftSquarePos[2] = Vector3(0.f, 0.f, FinalMeshScale.z);
    m_PunchLeftSquarePos[3] = Vector3(0.f, 0.f, FinalMeshScale.z * 0.5f);

    m_PunchRightSquarePos[0] = Vector3(0.f, 0.f, FinalMeshScale.z * 0.5f);
    m_PunchRightSquarePos[1] = Vector3(0.f, 0.f, FinalMeshScale.z);
    m_PunchRightSquarePos[2] = Vector3(FinalMeshScale.x * 0.5f, 0.f, FinalMeshScale.z);
    m_PunchRightSquarePos[3] = Vector3(FinalMeshScale.x * 0.5f, 0.f, FinalMeshScale.z * 0.5f);

    m_SlashLeftSquarePos[0] = Vector3(FinalMeshScale.x * -1 * 1.5f, 0.f, FinalMeshScale.z * -1.f);
    m_SlashLeftSquarePos[1] = Vector3(FinalMeshScale.x * -1 * 1.5f, 0.f, FinalMeshScale.z);
    m_SlashLeftSquarePos[2] = Vector3(FinalMeshScale.x * -1 * 0.5f, 0.f, FinalMeshScale.z);
    m_SlashLeftSquarePos[3] = Vector3(FinalMeshScale.x * -1 * 0.5f, 0.f, FinalMeshScale.z * -1.f);

    m_SlashRightSquarePos[0] = Vector3(FinalMeshScale.x * 0.5f, 0.f, FinalMeshScale.z * -1.f);
    m_SlashRightSquarePos[1] = Vector3(FinalMeshScale.x * 0.5f, 0.f, FinalMeshScale.z);
    m_SlashRightSquarePos[2] = Vector3(FinalMeshScale.x * 1 * 1.5f, 0.f, FinalMeshScale.z);
    m_SlashRightSquarePos[3] = Vector3(FinalMeshScale.x * -1 * 1.5f, 0.f, FinalMeshScale.z * -1.f);

    m_TwoSideFrontSquarePos[0] = Vector3(FinalMeshScale.x * -1 * 1.5f, 0.f, FinalMeshScale.z);
    m_TwoSideFrontSquarePos[1] = Vector3(FinalMeshScale.x * -1 * 1.5f, 0.f, FinalMeshScale.z * 1.5f);
    m_TwoSideFrontSquarePos[2] = Vector3(FinalMeshScale.x * 1 * 1.5f, 0.f, FinalMeshScale.z * 1.5f);
    m_TwoSideFrontSquarePos[3] = Vector3(FinalMeshScale.x * 1 * 1.5f, 0.f, FinalMeshScale.z);

    m_RelativePunchRightPos = (m_PunchRightSquarePos[0] + m_PunchRightSquarePos[2]) / 2.f;
    m_RelativePunchLeftPos = (m_PunchLeftSquarePos[0] + m_PunchLeftSquarePos[2]) / 2.f;

    m_RelativeSlashRightPos = (m_SlashLeftSquarePos[0] + m_SlashLeftSquarePos[2]) / 2.f;
    m_RelativeSlashLeftPos = (m_SlashRightSquarePos[0] + m_SlashRightSquarePos[2]) / 2.f;
}

void CBossBettyDataComponent::OnBossBettyGenerateTwoSideCloseAttackEffect()
{
	// 양쪽에 
	// 1) 충돌체 활성화
	// 2) Particle 제작
}

void CBossBettyDataComponent::OnBossBettyGenerateRightCloseAttackEffect()
{
    // m_RelativePunchRightPos
    m_MeleeAttackCollider->SetRelativePos(m_RelativePunchRightPos);
}

void CBossBettyDataComponent::OnBossBettyGenerateLeftCloseAttackEffect()
{
    // m_RelativePunchLeftPos
    m_MeleeAttackCollider->SetRelativePos(m_RelativePunchLeftPos);
}

void CBossBettyDataComponent::OnBossBettyRoarEffect(float DeltaTime)
{
	// 1. 앞으로, 원형 형태로 뻗어나가는 형태의 Particle 만들어내기 
}

// 투사체를 던진 이후, 투사체가 바닥, 벽 등에 충돌 이후, 터질 때 효과 
void CBossBettyDataComponent::OnExplodeBettyThrowBallCallback()
{
    // 자기 위치를 기준으로 20개의 위치를 세팅한다.
    // 서서히 Delay 를 주면서 내려오게 할 것이다.
}

void CBossBettyDataComponent::OnChangeFromSpinToSpinCollideWhenCollide(const CollisionResult& Result)
{
    CAnimationSequenceInstance* AnimInst = dynamic_cast<CAnimationMeshComponent*>(m_Object->GetRootComponent())->GetAnimationInstance();

    if (AnimInst->GetCurrentAnimation()->GetName() == "Spin")
    {
        if (Result.Dest->GetGameObject()->GetName() != "MapSurroundCollider")
            return;

        // 구 ~ 구 충돌
        // BossBettySpinCollider 충돌체는, 전체 Map Surround Collider 안에 존재한다.
        // 구 안에, 특정 구가 존재할 경우에는, 항상 충돌로 판정한다.
        // 대신, 구끼리 충돌하는 그 순간에는 HitPoint 정보가 0이 아닌 상태로 존재하게 될 것이다.
        if (Result.HitPoint.x == 0.f)
            return;

        // 1. Spin Collider 로 Change
        AnimInst->ChangeAnimation("SpinCollide");

        // 벽 충돌 -> Camera Shake 효과를 준다.
        OnBossBettyNormalShakeCamera();

        // 2. 더이상의 움직임을 멈춘다.
        m_MoveZ = false;

        // BossBetty Spin Collider 를 Enable False 처리 한다.
        m_BossBettySpinCollider->Enable(false);
    }
}

void CBossBettyDataComponent::OnPreventGoingOutOfMapSurroundingCollider(const CollisionResult& Result)
{
    CAnimationSequenceInstance* AnimInst = dynamic_cast<CAnimationMeshComponent*>(m_Object->GetRootComponent())->GetAnimationInstance();
    
    if (Result.Dest->GetGameObject()->GetName() != "MapSurroundCollider")
        return;

    // 밖으로 나가는 순간, Z Move 를 비활성화 한다.
    if (Result.HitPoint.x > 0.f)
    {
        OnDisableMoveZ();
    }
}

void CBossBettyDataComponent::OnBossBettyDisableSpinCollider()
{
    m_BossBettySpinCollider->Enable(false);
}

void CBossBettyDataComponent::OnBossBettyEnableSpinCollider()
{
    m_BossBettySpinCollider->Enable(true);
}

void CBossBettyDataComponent::OnBossBettySetCurrentNodeNullPtr()
{	
    // Current Node 가 BettySpinAttackNode 로 되어 있을 것이다. 
    // 이를 nullptr 로 세팅
    m_State->GetBehaviorTree()->SetCurrentNode(nullptr);
}

void CBossBettyDataComponent::OnBossBettyResetOriginalMoveSpeed()
{
    m_CurMoveSpeed = m_OriginMoveSpeed;
}

void CBossBettyDataComponent::OnBossBettyNormalShakeCamera()
{
    m_Scene->GetCameraManager()->GetCurrentCamera()->Shake(0.2f, 1.2f);
}

void CBossBettyDataComponent::OnBossBettyApplyOutOfMapSurroundingColliderMoveSpeed()
{
   SetCurMoveSpeed(m_OriginMoveSpeed * 0.2f);
}

void CBossBettyDataComponent::OnBossBettyEnableAttackCollider()
{
    CMonsterDataComponent::OnActiveMeleeAttackCollider();

    m_MeleeAttackCollider->Enable(true);
}

void CBossBettyDataComponent::OnBossBettyDisableAttackCollider()
{
    CMonsterDataComponent::OnInActiveMeleeAttackCollider();

    m_MeleeAttackCollider->Enable(false);
}
