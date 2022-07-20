#pragma once
#include "MonsterDataComponent.h"

enum class BossBettyFarAttackType
{
    Spin,
    JumpSmash
};

enum class BossBettyHPState
{
    None,
    Below60,
    Below30
};

class CBossBettyDataComponent :
    public CMonsterDataComponent
{
    friend class CGameObject;

private:
    CBossBettyDataComponent();
    CBossBettyDataComponent(const CBossBettyDataComponent& com);
    virtual ~CBossBettyDataComponent();

private :
    // 근거리 연속 공격 횟수
    int m_CloseSequentialAttackNum;

    // 원거리 공격 횟수 -> 3번 마다, Spin , Jump 를 번갈아가면서 실행할 것이다.
    int m_FarAttackAttackNum;

    // 6번 원거리 공격 Gauge 가 다 차면, Throw 를 할 것이다.
    bool m_ThrowFarAttackEnable;

    // 원거리 공격 Type
    BossBettyFarAttackType m_FarAttackType;

    // 60%, 30% 가 될 때마다, Angry Attack 중에서 첫번째 Attack 설정하기 
    BossBettyHPState m_BettyHPState;

    // 근거리 공격 범위 정보 (사각형)
    Vector3 m_PunchLeftSquarePos[4];
    Vector3 m_PunchRightSquarePos[4];
    Vector3 m_SlashLeftSquarePos[4];
    Vector3 m_SlashRightSquarePos[4];
    Vector3 m_TwoSideFrontSquarePos[4];

    // 공격 Point 지정 
    Vector3 m_RelativePunchRightPos;
    Vector3 m_RelativePunchLeftPos;
    Vector3 m_RelativeSlashRightPos;
    Vector3 m_RelativeSlashLeftPos;

    // Spin 하는 중에 벽과 충돌했는지 여부를 추가적으로 계속 조사하기 위해서
    // ColliderBox3D 를 하나 더 둘 것이다.
    class CColliderComponent* m_BossBettySpinCollider;

    // Boss Betty Throw Ball 투사체 Object
    class CGameObject* m_BossBettyThrowObject;
    // Boss Betty Punch Down Particle
    // Boss Betty Slash Particle
    // Boss Betty Roar Particle

public:
    virtual void Start() override;
public :
    // (아래 콜백 함수들은, 여러 Action Node 들에서 공통으로 사용하는 효과)
    // 땅을 내리칠때, 양쪽에 Attack 효과를 내기
    void OnBossBettyGenerateTwoSideCloseAttackEffect();
    // 땅 내리칠때 오른쪽에 Attack 효과를 내기 
    void OnBossBettyGenerateRightCloseAttackEffect();
    // 땅 내리칠 때 왼쪽에 Attack 효과를 내기 
    void OnBossBettyGenerateLeftCloseAttackEffect();
    // 울부짖기 효과
    void OnBossBettyRoarEffect();
    // 눈덩이 투사체 충돌시 발생시킬 Callback 함수
    void OnExplodeBettyThrowBallCallback();
    // Spin 중간에 벽에 부딪히면, Spin Collide Animation 으로 바꿔주는 함수 
    void OnChangeFromSpinToSpinCollideWhenCollide(const CollisionResult&);
    // Spin Collider 비활성화
    void OnBossBettyDisableSpinCollider();
    // Spin Collider 활성화
    void OnBossBettyEnableSpinCollider();

    // Setter 함수 ---
public:

    void SetBossBettyThrowObject(class CGameObject* Object)
    {
        m_BossBettyThrowObject = Object;
    }
    void ResetBettyHPState()
    {
        m_BettyHPState = BossBettyHPState::None;
    }
    void SetCloseAngryAttackEnable(bool Enable)
    {
        m_ThrowFarAttackEnable = Enable;
    }

    void IncCloseSequentialAttackNum()
    {
        ++m_CloseSequentialAttackNum;
    }
    void IncFarAttackNum()
    {
        ++m_FarAttackAttackNum;

        // 3번 마자, Far Attack Type 을 다르게 해줄 것이다.
        if (m_FarAttackAttackNum % 3 == 0)
        {
            if (m_FarAttackType == BossBettyFarAttackType::Spin)
                m_FarAttackType = BossBettyFarAttackType::JumpSmash;
            else
                m_FarAttackType = BossBettyFarAttackType::Spin;
        }

        if (m_FarAttackAttackNum == 6)
        {
            m_ThrowFarAttackEnable = true;
            m_FarAttackAttackNum = 0;
        }
    }


    // Getter 함수 ---
public:
    class CColliderComponent* GetBossBettySpinCollider() const
    {
        return m_BossBettySpinCollider;
    }
    class CGameObject* GetBossBettyThrowObject() const
    {
        return m_BossBettyThrowObject;
    }
    // 근거리 공격 위치 
    const Vector3& GetRelativePunchRightPos()
    {
        return m_RelativePunchRightPos;
    }
    const Vector3& GetRelativePunchLeftPos()
    {
        return m_RelativePunchLeftPos;
    }
    const Vector3& GetRelativeSlashRightPos()
    {
        return m_RelativeSlashRightPos;
    }
    const Vector3& GetRelativeSlashLeftPos()
    {
        return m_RelativeSlashLeftPos;
    }
    // 근거리 사정 범위 사각형
    const Vector3* GetPunchLeftSquarePoses()
    {
        return m_PunchLeftSquarePos;
    }
    const Vector3* GetPunchRightSquarePoses()
    {
        return m_PunchRightSquarePos;
    }
    const Vector3* GetSlashLeftSquarePoses()
    {
        return m_SlashLeftSquarePos;
    }
    const Vector3* GetSlashRightSquarePoses()
    {
        return m_SlashRightSquarePos;
    }
    const Vector3* GetTwoSideFrontSquarePoses()
    {
        return m_TwoSideFrontSquarePos;
    }
    bool IsThrowFarAttackEnable() const
    {
        return m_ThrowFarAttackEnable;
    }
    int GetCloseSequentialAttackNum() const
    {
        return m_CloseSequentialAttackNum;
    }
    BossBettyHPState GetBettyHPState() const
    {
        return m_BettyHPState;
    }
    BossBettyFarAttackType GetBettyFarAttackType() const
    {
        return m_FarAttackType;
    }
};

