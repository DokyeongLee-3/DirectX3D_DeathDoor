
#include "PlayerDataComponent.h"
#include "Input.h"
#include "GameObject/GameObject.h"
#include "Scene/Scene.h"
#include "Component/AnimationMeshComponent.h"
#include "../DataManager.h"
#include "GameStateComponent.h"
#include "Component/ColliderComponent.h"
#include "Component/ColliderBox3D.h"
#include "../Component/PlayerNormalAttackCheckCollider.h"
#include "../Component/PlayerBombComponent.h"
#include "Scene/Navigation3DManager.h"
#include "Component/StaticMeshComponent.h"
#include "Component/AnimationMeshComponent.h"
#include "Component/ParticleComponent.h"

CPlayerDataComponent::CPlayerDataComponent() :
	m_OnSlash(false),
	m_AnimComp(nullptr),
	m_Body(nullptr),
	m_FrameCount(0),
	m_MouseLButtonDown(false),
	m_Unbeatable(false),
	m_UnbeatableAccTime(0.f),
	m_UnbeatableTime(2.7f),
	m_LadderUpEnable(false),
	m_LadderDownEnable(false),
	m_IsClimbingLadder(false),
	m_AdjLadder(nullptr),
	m_Slash(nullptr),
	m_Sword(nullptr),
	m_CurrentDustIndex(0)
{
	SetTypeID<CPlayerDataComponent>();
	m_ComponentType = Component_Type::ObjectComponent;

	m_PlayerData.Abilty_Type = Player_Ability::Arrow;
}

CPlayerDataComponent::CPlayerDataComponent(const CPlayerDataComponent& com)	:
	CObjectDataComponent(com)
{
}

CPlayerDataComponent::~CPlayerDataComponent()
{
}

void CPlayerDataComponent::Start()
{
	m_Scene->GetSceneMode()->SetPlayerObject(m_Object);

	CInput::GetInst()->CreateKey("WeaponArrow", '1');
	//CInput::GetInst()->CreateKey("WeaponFire", '2');
	CInput::GetInst()->CreateKey("WeaponChain", '3');
	CInput::GetInst()->CreateKey("WeaponBomb", '4');
	CInput::GetInst()->CreateKey("None", '5');

	CInput::GetInst()->CreateKey("MoveForward", 'W');
	CInput::GetInst()->CreateKey("MoveBack", 'S');
	CInput::GetInst()->CreateKey("MoveLeft", 'A');
	CInput::GetInst()->CreateKey("MoveRight", 'D');
	CInput::GetInst()->CreateKey("Roll", VK_SPACE);

	CInput::GetInst()->SetKeyCallback("WeaponArrow", KeyState_Down, this, &CPlayerDataComponent::SetPlayerAbilityArrow);
	CInput::GetInst()->SetKeyCallback("WeaponFire", KeyState_Down, this, &CPlayerDataComponent::SetPlayerAbilityFire);
	CInput::GetInst()->SetKeyCallback("WeaponChain", KeyState_Down, this, &CPlayerDataComponent::SetPlayerAbilityChain);
	CInput::GetInst()->SetKeyCallback("WeaponBomb", KeyState_Down, this, &CPlayerDataComponent::SetPlayerAbilityBomb);
	CInput::GetInst()->SetKeyCallback("None", KeyState_Down, this, &CPlayerDataComponent::SetSkillNone);

	m_AnimComp = m_Object->FindComponentFromType<CAnimationMeshComponent>();

	// Player Melee Attack Check Collider
	m_AttackCheckCollider = m_Object->FindComponentFromType<CPlayerNormalAttackCheckCollider>();
	m_AttackCheckCollider->Enable(false);

	// Player Animation Notify는 여기 추가
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerSlashL", "PlayerSlashL", 2, this, &CPlayerDataComponent::SetTrueOnSlash);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerSlashL", "PlayerSlashL", 8, this, &CPlayerDataComponent::SetFalseOnSlash);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerSlashR", "PlayerSlashR", 2, this, &CPlayerDataComponent::SetTrueOnSlash);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerSlashR", "PlayerSlashR", 8, this, &CPlayerDataComponent::SetFalseOnSlash);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerHitBack", "PlayerHitBack", 0, this, &CPlayerDataComponent::OnHitBack);

	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerBomb", "PlayerBomb", 3, this, &CPlayerDataComponent::OnBombLift);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerBomb", "PlayerBombCountRest", 4, this, &CPlayerDataComponent::OnBombCountReset);

	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerRun", "MoveDustOn", 10, this, &CPlayerDataComponent::OnResetDustParticle);
	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerRun", "MoveDustOn", 30, this, &CPlayerDataComponent::OnResetDustParticle);

	m_AnimComp->GetAnimationInstance()->AddNotify<CPlayerDataComponent>("PlayerRoll", "PlayerRoll", 0, this, &CPlayerDataComponent::OnRoll);
	
	m_AnimComp->GetAnimationInstance()->SetEndFunction<CPlayerDataComponent>("PlayerHitBack", this, &CPlayerDataComponent::OnHitBackEnd);
	m_AnimComp->GetAnimationInstance()->SetEndFunction<CPlayerDataComponent>("PlayerHitRecover", this, &CPlayerDataComponent::OnHitRecoverEnd);
	m_AnimComp->GetAnimationInstance()->SetEndFunction<CPlayerDataComponent>("PlayerRoll", this, &CPlayerDataComponent::OnRollEnd);

	m_Data = CDataManager::GetInst()->GetObjectData("Player");
	m_Body = (CColliderComponent*)m_Object->FindComponent("Body");

	m_Slash = (CStaticMeshComponent*)m_Object->FindComponent("Slash");
	if(m_Slash)
		m_Slash->Enable(false);
	m_SlashDir = m_Object->GetWorldAxis(AXIS_Z);
	m_SlashDir *= 1.f;

	m_Sword = (CAnimationMeshComponent*)m_Object->FindComponent("SwordAnim");

	m_Object->FindAllSceneComponentFromTypeName<CParticleComponent>("Dust", m_vecMoveDust);
}

bool CPlayerDataComponent::Init()
{

	return true;
}

void CPlayerDataComponent::Update(float DeltaTime)
{
	m_FrameCount = 0;

	CGameStateComponent* Comp = m_Object->FindObjectComponentFromType<CGameStateComponent>();

	if (Comp->IsTreeUpdate())
	{
		//++m_FrameCount;
		bool LButtonDown = CInput::GetInst()->GetMouseLButtonClick();
		bool LButtonUp = CInput::GetInst()->GetMouseLButtonUp();

		// 
		if (LButtonDown && !m_MouseLButtonDown && !LButtonUp)
		{
			m_MouseLButtonDown = true;

			m_KeyStateQueue.push(VK_LBUTTON);

		}

		if (LButtonUp)
			m_MouseLButtonDown = false;

		if (m_Unbeatable)
		{
			if (m_UnbeatableAccTime > m_UnbeatableTime)
			{
				CColliderBox3D* BodyCollider = (CColliderBox3D*)m_Object->FindComponent("Body");

				if (BodyCollider)
					BodyCollider->SetRigidCollisionIgnore(false);

				m_Unbeatable = false;
				m_UnbeatableAccTime = 0.f;
				return;
			}

			m_IsHit = false;

			m_UnbeatableAccTime += DeltaTime;
		}
	}


}

void CPlayerDataComponent::PostUpdate(float DeltaTime)
{
}

void CPlayerDataComponent::PrevRender()
{
}

void CPlayerDataComponent::Render()
{
}

void CPlayerDataComponent::PostRender()
{
}

CPlayerDataComponent* CPlayerDataComponent::Clone()
{
	return new CPlayerDataComponent(*this);
}

bool CPlayerDataComponent::Save(FILE* File)
{
	CObjectDataComponent::Save(File);

	fwrite(&m_PlayerData, sizeof(PlayerData), 1, File);

	return true;
}

bool CPlayerDataComponent::Load(FILE* File)
{
	CObjectDataComponent::Load(File);

	fread(&m_PlayerData, sizeof(PlayerData), 1, File);

	return true;
}

bool CPlayerDataComponent::SaveOnly(FILE* File)
{
	CObjectDataComponent::Save(File);

	fwrite(&m_PlayerData, sizeof(PlayerData), 1, File);

	return true;
}

bool CPlayerDataComponent::LoadOnly(FILE* File)
{
	CObjectDataComponent::Load(File);

	fread(&m_PlayerData, sizeof(PlayerData), 1, File);

	return true;
}

void CPlayerDataComponent::SetAdjLadder(CGameObject* Ladder)
{
	m_AdjLadder = Ladder;
}

inline void CPlayerDataComponent::SetTrueOnSlash()
{
	m_OnSlash = true;

	m_AttackCheckCollider->Enable(true);
	m_Slash->Enable(true);

	Vector3 AxisZ = m_Object->GetWorldAxis(AXIS_Z);

	float Angle = 0.f;
	float DotProduct = m_SlashDir.Dot(AxisZ);
	Vector3 CrossVec = m_SlashDir.Cross(AxisZ);

	if (DotProduct >= -0.99999999999f && DotProduct <= 0.99999999999f)
	{
		Angle = RadianToDegree(acosf(DotProduct));

		if (CrossVec.y < 0)
			Angle *= -1.f;
	}

	else
	{
		if (DotProduct == -1.f)
			Angle = 180.f;
	}

	m_Slash->AddRelativeRotationY(Angle);

	Matrix mat;
	mat.Rotation(Vector3(0.f, Angle, 0.f));

	m_SlashDir = m_SlashDir.TransformCoord(mat);
	
}

inline void CPlayerDataComponent::SetFalseOnSlash()
{
	m_OnSlash = false;

	m_AttackCheckCollider->Enable(false);
	m_Slash->Enable(false);
}

CAnimationMeshComponent* CPlayerDataComponent::GetSword() const
{
	return m_Sword;
}

CGameObject* CPlayerDataComponent::GetAdjLadder() const
{
	return m_AdjLadder;
}

CAnimationMeshComponent* CPlayerDataComponent::GetAnimationMeshComponent() const
{
	return m_AnimComp;
}

void CPlayerDataComponent::OnHitBack()
{
	m_Body->Enable(false);
	m_NoInterrupt = true;
}

void CPlayerDataComponent::OnHitBackEnd()
{
	m_AnimComp->GetAnimationInstance()->ChangeAnimation("PlayerHitRecover");
}

void CPlayerDataComponent::OnHitRecoverEnd()
{
	m_Body->Enable(true);

	m_NoInterrupt = false;
	m_IsHit = false;
}

void CPlayerDataComponent::OnRoll()
{
	//m_Body->Enable(false);
	m_Unbeatable = true;

	m_Body->SetRigidCollisionIgnore(true);
}

void CPlayerDataComponent::OnRollEnd()
{
	//m_Body->Enable(true);
	//m_Unbeatable = false;

	m_Body->SetRigidCollisionIgnore(false);
}

void CPlayerDataComponent::OnBombLift()
{
	CPlayerBombComponent* BombComp = m_Object->FindObjectComponentFromType<CPlayerBombComponent>();

	if (BombComp)
	{
		BombComp->SetLiftSpeed(0.1f);
		BombComp->LiftBomb();
	}
}

void CPlayerDataComponent::OnBombCountReset()
{
	CPlayerBombComponent* BombComp = m_Object->FindObjectComponentFromType<CPlayerBombComponent>();

	if (BombComp)
	{
		BombComp->ResetCount();
	}
}


void CPlayerDataComponent::ForceUpdateAttackDirection()
{
	CNavigation3DManager* Manager = m_Object->GetScene()->GetNavigation3DManager();

	Vector3 TargetPos;
	Vector3 ZAxis = m_Object->GetWorldAxis(AXIS::AXIS_Z);

	Vector3 CurrentForwardVector = Vector3(-ZAxis.x, 0.f, -ZAxis.z);
	Vector3 DestForwardVector;
	Vector3 PickingPoint;
	Vector3 CrossVec;
	bool Over180;

	if (Manager->CheckNavMeshPickingPoint(PickingPoint))
	{
		Vector3 CurrentPos = m_Object->GetWorldPos();
		DestForwardVector = Vector3(PickingPoint.x, 0.f, PickingPoint.z) - Vector3(CurrentPos.x, 0.f, CurrentPos.z);
		DestForwardVector.Normalize();

		CrossVec = Vector3(CurrentForwardVector.x, CurrentForwardVector.y, CurrentForwardVector.z).Cross(Vector3(DestForwardVector.x, DestForwardVector.y, -DestForwardVector.z));

		// 반시계 방향으로 180도가 넘는다
		if (CrossVec.y < 0)
			Over180 = true;
		else
			Over180 = false;
	}

	else
	{
		PickingPoint = m_Object->GetWorldPos();
		DestForwardVector = CurrentForwardVector;
		return;
	}

	//////

	Vector3 CurrentFowardYZero = Vector3(CurrentForwardVector.x, 0.f, CurrentForwardVector.z);
	Vector3 DestForwardYZero = Vector3(DestForwardVector.x, 0.f, DestForwardVector.z);

	CrossVec = Vector3(CurrentFowardYZero.x, 0.f, CurrentFowardYZero.z).Cross(Vector3(DestForwardYZero.x, 0.f, DestForwardYZero.z));

	// 반시계 방향으로 180도가 넘는다
	if (CrossVec.y < 0)
		Over180 = true;
	else
		Over180 = false;

	float DotProduct = CurrentFowardYZero.Dot(DestForwardYZero);

	if (DotProduct > 0.99f || DotProduct < -0.99f)
	{
		return;
	}

	float Rad = acosf(DotProduct);
	float Degree = RadianToDegree(Rad);

	DestForwardVector.Normalize();

	// CurretForwardVector기준 DestForwardVector로 시계 방향으로 180도가 넘는다면
	if (Over180)
	{
		m_Object->AddWorldRotationY(-Degree);
	}

	else
	{
		m_Object->AddWorldRotationY(Degree);
	}

	m_AttackDir = DestForwardVector;
}

void CPlayerDataComponent::OnResetDustParticle()
{
	//Vector3 ObjectPos = m_Object->GetWorldPos();
	//Vector3 ZDir = m_Object->GetWorldAxis(AXIS_Z);

	//m_vecMoveDust[m_CurrentDustIndex]->SetWorldPos(ObjectPos.x + ZDir.x, ObjectPos.y + 1.f, ObjectPos.z + ZDir.z);

	//if (m_vecMoveDust[m_CurrentDustIndex])
	//	m_vecMoveDust[m_CurrentDustIndex]->RecreateOnlyOnceCreatedParticleWithOutLifeTimeSetting();

	//++m_CurrentDustIndex;

	//if (m_CurrentDustIndex >= m_vecMoveDust.size())
	//	m_CurrentDustIndex = 0;
}
