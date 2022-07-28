#pragma once
#include "MonsterDataComponent.h"
class CTinyCrowDataComponent :
    public CMonsterDataComponent
{
	friend class CGameObject;

private:
	CTinyCrowDataComponent();
	CTinyCrowDataComponent(const CTinyCrowDataComponent& com);
	virtual ~CTinyCrowDataComponent();

private:
	float m_AccTime;

public:
	virtual void Start() override;
	virtual void Update(float DeltaTime);

	void OnCollisionPlayer(const CollisionResult& Result);
};

