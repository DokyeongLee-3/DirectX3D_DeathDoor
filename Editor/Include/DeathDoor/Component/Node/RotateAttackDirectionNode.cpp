
#include "RotateAttackDirectionNode.h"
#include "Component/AnimationMeshComponent.h"
#include "Animation/AnimationSequenceInstance.h"
#include "Component/BehaviorTree.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Scene/Navigation3DManager.h"
#include "../PlayerDataComponent.h"
#include "../GameStateComponent.h"

CRotateAttackDirectionNode::CRotateAttackDirectionNode() :
	m_Over180(false)
{
	m_Result = NodeResult::Node_True;
	SetTypeID(typeid(CRotateAttackDirectionNode).hash_code());
}

CRotateAttackDirectionNode::CRotateAttackDirectionNode(const CRotateAttackDirectionNode& Node) :
	CActionNode(Node)
{
}

CRotateAttackDirectionNode::~CRotateAttackDirectionNode()
{
}

NodeResult CRotateAttackDirectionNode::OnStart(float DeltaTime)
{
	//m_Object->SetNoInterrupt(false);
	//m_CallStart = true;

	CNavigation3DManager* Manager = m_Object->GetScene()->GetNavigation3DManager();

	Vector3 TargetPos;
	Vector3 ZAxis = m_Object->GetWorldAxis(AXIS::AXIS_Z);
	ZAxis *= -1.f;
	m_CurrentForwardVector = ZAxis;

	if (Manager->CheckNavMeshPickingPoint(m_PickingPoint))
	{
		Vector3 CurrentPos = m_Object->GetWorldPos();
		m_DestForwardVector = Vector3(m_PickingPoint.x, 0.f, m_PickingPoint.z) - Vector3(CurrentPos.x, 0.f, CurrentPos.z);
		m_DestForwardVector.Normalize();

		Vector3 CrossVec = m_CurrentForwardVector.Cross(m_DestForwardVector);

		// 시계 방향으로 180도가 넘는다
		if (CrossVec.y < 0)
			m_Over180 = true;
		else
			m_Over180 = false;
	}

	else
	{
		m_PickingPoint = m_Object->GetWorldPos();
		m_DestForwardVector = m_CurrentForwardVector;
		m_Result = NodeResult::Node_False;
		return NodeResult::Node_True;
	}

	Vector3 CurrentFowardYZero = Vector3(m_CurrentForwardVector.x, 0.f, m_CurrentForwardVector.z);
	Vector3 DestForwardYZero = Vector3(m_DestForwardVector.x, 0.f, m_DestForwardVector.z);

	float DotProduct = CurrentFowardYZero.Dot(DestForwardYZero);

	if (DotProduct > 0.9999f || DotProduct < -0.9999f)
	{
		m_IsEnd = true;
		m_Result = NodeResult::Node_True;
		return NodeResult::Node_True;
	}

	float Rad = acosf(DotProduct);
	float Degree = RadianToDegree(Rad);

	m_DestForwardVector.Normalize();

	CPlayerDataComponent* PlayerDataComp = dynamic_cast<CPlayerDataComponent*>(dynamic_cast<CGameStateComponent*>(m_Owner->GetOwner())->GetData());
	PlayerDataComp->SetAttackDir(m_DestForwardVector);

	// CurretForwardVector기준 DestForwardVector로 시계 방향으로 180도가 넘는다면 반시계 방향 회전을 해준다
	if (m_Over180)
	{
		m_Object->AddWorldRotationY(-Degree);

		m_IsEnd = true;
		m_Result = NodeResult::Node_True;
		return NodeResult::Node_True;
	}

	else
	{
		m_Object->AddWorldRotationY(Degree);

		m_IsEnd = true;
		m_Result = NodeResult::Node_True;
		return NodeResult::Node_True;
	}

	return NodeResult::Node_True;
}

NodeResult CRotateAttackDirectionNode::OnUpdate(float DeltaTime)
{
	//return NodeResult::Node_True;
	return m_Result;
}

NodeResult CRotateAttackDirectionNode::OnEnd(float DeltaTime)
{
	m_Owner->SetCurrentNode(nullptr);
	m_IsEnd = false;

	return NodeResult::Node_True;
}

NodeResult CRotateAttackDirectionNode::Invoke(float DeltaTime)
{
	return CActionNode::Invoke(DeltaTime);
}
