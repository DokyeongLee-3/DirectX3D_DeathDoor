#pragma once

#include "../Component/SceneComponent.h"
#include "../Component/ObjectComponent.h"
#include "../Component/NavAgent.h"
#include "../Scene/Navigation3DManager.h"

class CGameObject :
	public CRef
{
	friend class CScene;

protected:
	CGameObject();
	CGameObject(const CGameObject& obj);
	virtual ~CGameObject();

protected:
	class CScene* m_Scene;
	bool		m_NoInterrupt;	// 다른 State로 전환 불가능한 상태인지(ex. 특정 스킬 사용중에 다른 스킬을 쓸 수 없다)
	Object_Type m_ObjectType;
	bool		m_IsEnemy;

public:
	bool IsEnemy()	const
	{
		return m_IsEnemy;
	}

	void SetEnemy(bool Enemy)
	{
		m_IsEnemy = Enemy;
	}

	Object_Type GetObjectType()	const
	{
		return m_ObjectType;
	}

	void SetObjectType(Object_Type Type)
	{
		m_ObjectType = Type;
	}

	class CScene* GetScene()    const
	{
		return m_Scene;
	}

	bool IsNoInterrupt()	const
	{
		return m_NoInterrupt;
	}

	void SetNoInterrupt(bool Enable)
	{
		m_NoInterrupt = Enable;
	}

public:
	void SetScene(class CScene* Scene);
	virtual void Destroy();

protected:
	CSharedPtr<CSceneComponent> m_RootComponent;
	std::list<CSceneComponent*>	m_SceneComponentList;
	std::vector<CSharedPtr<CObjectComponent>>   m_vecObjectComponent;

	CGameObject* m_Parent;
	std::vector<CSharedPtr<CGameObject>>   m_vecChildObject;
	float		m_LifeSpan;
	class CNavAgent* m_NavAgent;

	CGameObject* m_AttackTarget;
	ChampionInfo m_ChampionInfo;

public:
	const ChampionInfo& GetChampionInfo()	const
	{
		return m_ChampionInfo;
	}

	void SetAttackTarget(CGameObject* Target)
	{
		m_AttackTarget = Target;
	}

	CGameObject* GetAttackTarget()	const
	{
		return m_AttackTarget;
	}

public:
	void AddChildObject(CGameObject* Obj);
	void DeleteObj();
	bool DeleteChildObj(const std::string& Name);
	// 실제로 지우진 않고, 나는 부모를 nullptr로 만들고, 부모에게 내 자신을 자식 목록에서 지우게 함
	void ClearParent();

public:
	bool CheckSceneComponent(CSceneComponent* Com)
	{
		auto iter = m_SceneComponentList.begin();
		auto iterEnd = m_SceneComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter) == Com)
				return true;
		}

		return false;
	}

	const std::list<CSceneComponent*>& GetSceneComponents()	const
	{
		return m_SceneComponentList;
	}

	void SetRootComponent(CSceneComponent* Component)
	{
		m_RootComponent = Component;
	}

	CSceneComponent* GetRootComponent()	const
	{
		return m_RootComponent;
	}

	void AddSceneComponent(CSceneComponent* Component)
	{
		m_SceneComponentList.push_back(Component);
	}

	void DeleteSceneComponent(CSceneComponent* Component)
	{
		auto iter = m_SceneComponentList.begin();
		auto iterEnd = m_SceneComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter) == Component)
			{
				m_SceneComponentList.erase(iter);
				return;
			}
		}
	}

	void DeleteSceneComponent(const std::string& Name)
	{
		auto iter = m_SceneComponentList.begin();
		auto iterEnd = m_SceneComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->GetName() == Name)
			{
				m_SceneComponentList.erase(iter);
				return;
			}
		}
	}

	bool DeleteObjectComponent(const std::string& Name);

	class CComponent* FindComponent(const std::string& Name);
	template <typename T>
	T* FindComponentFromType()
	{
		auto	iter = m_SceneComponentList.begin();
		auto	iterEnd = m_SceneComponentList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->CheckType<T>())
				return (T*)*iter;
		}

		auto	iter1 = m_vecObjectComponent.begin();
		auto	iter1End = m_vecObjectComponent.end();

		for (; iter1 != iter1End; ++iter1)
		{
			if ((*iter1)->CheckType<T>())
				return (T*)(*iter1).Get();
		}

		return nullptr;
	}

	template <typename T>
	T* FindObjectComponentFromType()
	{
		auto	iter1 = m_vecObjectComponent.begin();
		auto	iter1End = m_vecObjectComponent.end();

		for (; iter1 != iter1End; ++iter1)
		{
			if ((*iter1)->CheckType<T>())
				return (T*)(*iter1).Get();
		}

		return nullptr;
	}

	void GetAllSceneComponentsName(std::vector<FindComponentName>& vecNames);
	void GetAllSceneComponentsPointer(std::vector<CSceneComponent*>& OutVecSceneComp);
	void GetAllObjectComponentsPointer(std::vector<CObjectComponent*>& OutVecObjComp);
	size_t GetSceneComponentCount()	const
	{
		return m_SceneComponentList.size();
	}

	void SetLifeSpan(float LifeSpan)
	{
		m_LifeSpan = LifeSpan;
	}

public:
	virtual void Start();
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	void AddCollision();
	virtual void PrevRender();
	virtual void Render();
	virtual void PostRender();
	virtual CGameObject* Clone();
	virtual bool Save(FILE* File);
	virtual bool Load(FILE* File);
	virtual bool Save(const char* FullPath);
	virtual bool Load(const char* FullPath);
	virtual bool Save(const char* FileName, const std::string& PathName);
	virtual bool Load(const char* FileName, const std::string& PathName);
	virtual bool SaveOnly(CComponent* Component, const char* FullPath);
	virtual bool SaveOnly(const std::string& ComponentName, const char* FullPath);
	virtual bool LoadOnly(const char* FullPath, CComponent*& OutCom);

public:
	// NavAgent가 있을 경우에 동작한다.
	void Move(const Vector3& EndPos);

	// m_NavAgent의 m_PathList에 TargetPos를 추가해준다
	void AddPath(const Vector3& TargetPos);
	void ClearPath();
	
public:
	void SetNavAgent(CNavAgent* Agent)
	{
		m_NavAgent = Agent;
	}

	CNavAgent* GetNavAgent()	const
	{
		return m_NavAgent;
	}

	void SetNavManagerLandScape(class CLandScape* LandScape);
	bool IsNavAgentPathListEmpty()	const;

public:
	template <typename T>
	T* CreateComponent(const std::string& Name)
	{
		T* Component = new T;

		Component->SetName(Name);
		Component->SetScene(m_Scene);
		Component->SetGameObject(this);

		if (!Component->Init())
		{
			SAFE_RELEASE(Component);
			return nullptr;
		}

		if (Component->GetComponentType() == Component_Type::ObjectComponent)
		{
			m_vecObjectComponent.push_back((class CObjectComponent*)Component);

			if (Component->GetTypeID() == typeid(CNavAgent).hash_code())
			{
				m_NavAgent = (CNavAgent*)Component;
				m_NavAgent->SetUpdateComponent(m_RootComponent);
			}
		}

		else
		{
			if (Component->GetTypeID() == typeid(CLandScape).hash_code())
			{
				SetNavManagerLandScape((CLandScape*)Component);
			}

			m_SceneComponentList.push_back((class CSceneComponent*)Component);

			if (!m_RootComponent)
				m_RootComponent = (class CSceneComponent*)Component;
		}

		return Component;
	}

	// Root가 이미 있으면 Root의 자식 Component로 자동으로 들어가게 하는 함수
	template <typename T>
	T* CreateComponentAddChild(const std::string& Name)
	{
		T* Component = new T;

		Component->SetName(Name);
		Component->SetScene(m_Scene);
		Component->SetGameObject(this);

		if (!Component->Init())
		{
			SAFE_RELEASE(Component);
			return nullptr;
		}

		if (Component->GetComponentType() == Component_Type::ObjectComponent)
		{
			m_vecObjectComponent.push_back((class CObjectComponent*)Component);

			if (Component->GetTypeID() == typeid(CNavAgent).hash_code())
			{
				m_NavAgent = (CNavAgent*)Component;
				m_NavAgent->SetUpdateComponent(m_RootComponent);
			}
		}

		else
		{
			m_SceneComponentList.push_back((class CSceneComponent*)Component);

			if (Component->GetTypeID() == typeid(CLandScape).hash_code())
			{
				SetNavManagerLandScape((CLandScape*)Component);
			}

			if (!m_RootComponent)
				m_RootComponent = (class CSceneComponent*)Component;

			else
				m_RootComponent->AddChild((class CSceneComponent*)Component);
		}

		Component->Start();

		return Component;

	}

	template <typename T>
	T* LoadComponent()
	{
		T* Component = new T;

		Component->SetScene(m_Scene);
		Component->SetGameObject(this);


		if (Component->GetComponentType() == Component_Type::ObjectComponent)
			m_vecObjectComponent.push_back((class CObjectComponent*)Component);

		else
		{
			m_SceneComponentList.push_back((class CSceneComponent*)Component);

			if (!m_RootComponent)
				m_RootComponent = Component;
		}

		Component->Start();

		return Component;
	}

	template <typename T>
	T* LoadObjectComponent()
	{
		T* Component = new T;

		Component->SetScene(m_Scene);
		Component->SetGameObject(this);


		m_vecObjectComponent.push_back((class CObjectComponent*)Component);

		return Component;
	}

public:	// =============== 저주받은 Transform 영역 ===============
	void SetUpdateByMat(bool UpdateByMat);
	void DecomposeWorld();
	void SetTransformByWorldMatrix(const Matrix& matTRS);

	// 자신의 축 기준으로 이동
	void AddWorldPosByLocalAxis(AXIS Axis, float Amount);
	void AddWorldPosByLocalAxis(const Vector3& Pos);

	void SetInheritScale(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritScale(Inherit);
	}

	void SetInheritRotX(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritRotX(Inherit);
	}

	void SetInheritRotY(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritRotY(Inherit);
	}

	void SetInheritRotZ(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritRotZ(Inherit);
	}

	void SetInheritParentRotationPosX(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritParentRotationPosX(Inherit);
	}

	void SetInheritParentRotationPosY(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritParentRotationPosY(Inherit);
	}

	void SetInheritParentRotationPosZ(bool Inherit)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetInheritParentRotationPosZ(Inherit);
	}

	void InheritScale(bool Current)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->InheritScale(Current);
	}

	void InheritRotation(bool Current)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->InheritRotation(Current);
	}

	void InheritParentRotationPos(bool Current)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->InheritParentRotationPos(Current);
	}

	void InheritWorldScale(bool Current)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->InheritWorldScale(Current);
	}

	void InheritWorldRotation(bool Current)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->InheritWorldRotation(Current);
	}

public:
	Vector3 GetRelativeScale()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetRelativeScale();
	}

	Vector3 GetRelativeRot()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetRelativeRot();
	}

	Vector3 GetRelativePos()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetRelativePos();
	}

	Vector3 GetRelativeAxis(AXIS Axis)
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetRelativeAxis(Axis);
	}

public:
	void SetRelativeScale(const Vector3& Scale)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeScale(Scale);
	}

	void SetRelativeScale(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeScale(x, y, z);
	}

	void SetRelativeRotation(const Vector3& Rot)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeRotation(Rot);
	}

	void SetRelativeRotation(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeRotation(x, y, z);
	}

	void SetRelativeRotationX(float x)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeRotationX(x);
	}

	void SetRelativeRotationY(float y)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeRotationY(y);
	}

	void SetRelativeRotationZ(float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativeRotationZ(z);
	}

	void SetRelativePos(const Vector3& Pos)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativePos(Pos);
	}

	void SetRelativePos(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetRelativePos(x, y, z);
	}

	void AddRelativeScale(const Vector3& Scale)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeScale(Scale);
	}

	void AddRelativeScale(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeScale(x, y, z);
	}

	void AddRelativeRotation(const Vector3& Rot)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeRotation(Rot);
	}

	void AddRelativeRotation(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeRotation(x, y, z);
	}

	void AddRelativeRotationX(float x)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeRotationX(x);
	}

	void AddRelativeRotationY(float y)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeRotationY(y);
	}

	void AddRelativeRotationZ(float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativeRotationZ(z);
	}

	void AddRelativePos(const Vector3& Pos)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativePos(Pos);
	}

	void AddRelativePos(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddRelativePos(x, y, z);
	}

public:
	Vector3 GetWorldScale()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetWorldScale();
	}

	Vector3 GetWorldRot()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetWorldRot();
	}

	Vector3 GetWorldPos()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetWorldPos();
	}

	Vector3 GetPivot()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetPivot();
	}

	Vector3 GetMeshSize()	const
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetMeshSize();
	}

	const Matrix& GetWorldMatrix()	const
	{
		if (!m_RootComponent)
			return Matrix();

		return m_RootComponent->GetWorldMatrix();
	}

	Vector3 GetWorldAxis(AXIS Axis)
	{
		if (!m_RootComponent)
			return Vector3();

		return m_RootComponent->GetWorldAxis(Axis);
	}

public:
	void SetPivot(const Vector3& Pivot)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetPivot(Pivot);
	}

	void SetPivot(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetPivot(x, y, z);
	}

	void SetMeshSize(const Vector3& Size)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetMeshSize(Size);
	}

	void SetMeshSize(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetMeshSize(x, y, z);
	}

public:
	void SetWorldScale(const Vector3& Scale)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldScale(Scale);
	}

	void SetWorldScale(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldScale(x, y, z);
	}

	void SetWorldRotation(const Vector3& Rot)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldRotation(Rot);
	}

	void SetWorldRotation(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldRotation(x, y, z);
	}

	void SetWorldRotationX(float x)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldRotationX(x);
	}

	void SetWorldRotationY(float y)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldRotationY(y);
	}

	void SetWorldRotationZ(float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldRotationZ(z);
	}

	void SetWorldPos(const Vector3& Pos)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldPos(Pos);
	}

	void SetWorldPos(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->SetWorldPos(x, y, z);
	}

	void AddWorldScale(const Vector3& Scale)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldScale(Scale);
	}

	void AddWorldScale(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldScale(x, y, z);
	}

	void AddWorldRotation(const Vector3& Rot)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldRotation(Rot);
	}

	void AddWorldRotation(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldRotation(x, y, z);
	}

	void AddWorldRotationX(float x)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldRotationX(x);
	}

	void AddWorldRotationY(float y)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldRotationY(y);
	}

	void AddWorldRotationZ(float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldRotationZ(z);
	}

	void AddWorldPos(const Vector3& Pos)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldPos(Pos);
	}

	void AddWorldPos(float x, float y, float z)
	{
		if (!m_RootComponent)
			return;

		m_RootComponent->AddWorldPos(x, y, z);
	}
};



