#include "DDInstanceSceneMode.h"
#include "Scene/Scene.h"
#include "Component/ColliderBox3D.h"
#include "Component/PaperBurnComponent.h"
#include "../Component/GameStateComponent.h"
#include "ObjectPool.h"

CDDInstanceSceneMode::CDDInstanceSceneMode()
{
	SetTypeID<CDDInstanceSceneMode>();
}

CDDInstanceSceneMode::~CDDInstanceSceneMode()
{
	auto iter = m_SpawnPhaseList.begin();
	auto iterEnd = m_SpawnPhaseList.end();

	for (; iter != iterEnd; ++iter)
	{
		SAFE_DELETE(*iter);
	}

	m_SpawnPhaseList.clear();

	while (!m_PaperBurnEndSpawnQueue.empty())
	{
		DDSpawnObjectSet Set = m_PaperBurnEndSpawnQueue.front();
		m_DoorPaperburnQueue.pop();

		SAFE_DELETE(Set.Info);
	}
}

void CDDInstanceSceneMode::Start()
{
	CDDSceneMode::Start();

	CGameObject* EnterTriggerObj = m_Scene->FindObject(m_EnterTriggerObjectName);

	if (EnterTriggerObj)
	{
		m_EnterTrigger = EnterTriggerObj->FindComponentFromType<CColliderBox3D>();

		if (m_EnterTrigger)
		{
			m_EnterTrigger->AddCollisionCallback(Collision_State::Begin, this, &CDDInstanceSceneMode::OnCollideEnterTrigger);
		}

		EnterTriggerObj->Enable(true);
	}

	m_BlockerObj = m_Scene->FindObject(m_BlockerObjectName);

	if (m_BlockerObj)
	{
		m_BlockerObj->Enable(false);
		m_BlockerObj->FindComponentFromType<CColliderBox3D>()->ForceUpdateCBuffer();
		m_BlockerOriginY = m_BlockerObj->GetWorldPos().y;
	}

	if (m_ExitPointObj)
	{
		m_ExitPointObj->Enable(false);
	}

	// ��� ��ȯ�� ������ ī��Ʈ ���� ī���Ϳ� �߰�
	auto iter = m_SpawnPhaseList.begin();
	auto iterEnd = m_SpawnPhaseList.end();

	for (; iter != iterEnd; ++iter)
	{
		m_MonsterCount += int((*iter)->SpawnList.size());
	}
}

void CDDInstanceSceneMode::Update(float DeltaTime)
{
	CDDSceneMode::Update(DeltaTime);

	if (m_BlockerObj && m_BlockerUpMoving)
	{
		Vector3 CurPos = m_BlockerObj->GetWorldPos();

		if (CurPos.y - m_BlockerOriginY >= 7.f)
		{
			m_BlockerUpMoving = false;
			m_SpawnEventRunning = true;
		}
		else
		{
			// Blocker UP Moving
			m_BlockerObj->AddWorldPos(0.f, 7.f * DeltaTime, 0.f);
		}
	}

	if (m_BlockerObj && m_BlockerDownMoving)
	{
		// Blocker Down Moving
		Vector3 CurPos = m_BlockerObj->GetWorldPos();

		if (CurPos.y <= m_BlockerOriginY)
		{
			m_BlockerDownMoving = false;
			m_BlockerObj->Enable(false);
		}
		else
		{
			// Blocker UP Moving
			m_BlockerObj->AddWorldPos(0.f, -7.f * DeltaTime, 0.f);
		}
	}

	// Spawn Event ������ ���
	if (m_SpawnEventRunning)
	{
		// ������ ����� �ִ� ���
		if (m_CurPhase)
		{
			m_PhaseTimer += DeltaTime;

			// ����� �ð� ���ݵ��� ��ٸ�
			if (m_PhaseTimer <= m_CurPhase->Interval)
			{
				return;
			}

			// ���� ���� ������ �ִ� ���
			if (m_CurSpawnInfo)
			{
				m_SpawnTimer += DeltaTime;

				// ���� ���ݸ�ŭ ��ٸ�
				if (m_SpawnTimer <= m_CurSpawnInfo->Interval)
				{
					return;
				}

				// ���� ��ȯ��
				// TODO : ���� ��ȯ ��ƼŬ �߰�
				CGameObject* SpawnDoor = CObjectPool::GetInst()->GetMapObject("SpawnDoor", m_Scene);

				if (SpawnDoor)
				{
					SpawnDoor->Start();
					CPaperBurnComponent* DoorPaperBurn = SpawnDoor->FindObjectComponentFromType<CPaperBurnComponent>();
					DoorPaperBurn->SetFinishCallback(this, &CDDInstanceSceneMode::OnSpawnDoorPaperBurnEnd);
					DoorPaperBurn->SetInverse(true);
					DoorPaperBurn->SetEndEvent(PaperBurnEndEvent::None);
					DoorPaperBurn->StartPaperBurn();

					SpawnDoor->SetWorldPos(m_CurSpawnInfo->SpawnPosition);
					SpawnDoor->SetWorldRotation(m_CurSpawnInfo->SpawnRotation);

					// PaperBurn�� ���� ���� �����ϱ� ����, ť�� �־�ְ� PaperBurn�� ������ �޸� �����Ѵ�.
					DDSpawnObjectSet ObjSet;
					ObjSet.DoorPaperBurn = DoorPaperBurn;
					ObjSet.Info = m_CurSpawnInfo;
					m_PaperBurnEndSpawnQueue.push(ObjSet);
				}
				else
				{
					// SpawnDoor ������Ʈ�� ã�ƿ��� ���� ���� �߸��� �����̹Ƿ� ����
					SAFE_DELETE(m_CurSpawnInfo);
					assert(false);
					return;
				}

				m_CurSpawnInfo = nullptr;
				m_SpawnTimer = 0.f;
				return;
			}
			// ���� ���� ������ ���� ���
			else
			{
				// ���� ����� ��ȯ�� ���͸� �� ��ȯ�� ���
				// OnDieMonster() ���� ���� �������� ���͵��� ��� �� ó���� ��� �ٽ� SpawnEvent �߻���Ŵ
				if (m_CurPhase->SpawnList.empty())
				{
					SAFE_DELETE(m_CurPhase);
					m_PhaseTimer = 0.f;
					m_SpawnEventRunning = false;
					return;
				}
				// ���ο� ��ȯ ������ ����
				else
				{
					m_CurSpawnInfo = m_CurPhase->SpawnList.front();
					m_CurPhase->SpawnList.pop_front();
					m_SpawnTimer = 0.f;
					return;
				}
			}
		}
		else
		{
			// ���� ����� ������ ���� ����Ʈ���� ������
			if (!m_SpawnPhaseList.empty())
			{
				m_CurPhase = m_SpawnPhaseList.front();
				m_SpawnPhaseList.pop_front();

				m_CurPhaseMonsterCount = (int)m_CurPhase->SpawnList.size();
			}

			// �� �̻� �����ִ� ����� ���� ���, �̺�Ʈ ����
			if (!m_CurPhase)
			{
				m_SpawnEventRunning = false;
				return;
			}

			m_PhaseTimer = 0.f;
		}
	}
}

bool CDDInstanceSceneMode::Save(FILE* File)
{
	CDDSceneMode::Save(File);

	int Length = m_EnterTriggerObjectName.length();
	fwrite(&Length, sizeof(int), 1, File);
	fwrite(m_EnterTriggerObjectName.c_str(), sizeof(char), Length, File);

	Length = m_BlockerObjectName.length();
	fwrite(&Length, sizeof(int), 1, File);
	fwrite(m_BlockerObjectName.c_str(), sizeof(char), Length, File);

	int PhaseCount = (int)m_SpawnPhaseList.size();
	PhaseCount = (int)m_SpawnPhaseList.size();
	fwrite(&PhaseCount, sizeof(int), 1, File);

	auto iterPhase = m_SpawnPhaseList.begin();
	auto iterPhaseEnd = m_SpawnPhaseList.end();

	int SpawnCount = 0;
	for (; iterPhase != iterPhaseEnd; ++iterPhase)
	{
		fwrite(&(*iterPhase)->Interval, sizeof(float), 1, File);

		SpawnCount = (int)(*iterPhase)->SpawnList.size();
		fwrite(&SpawnCount, sizeof(int), 1, File);

		auto iterSpawnInfo = (*iterPhase)->SpawnList.begin();
		auto iterSpawnInfoEnd = (*iterPhase)->SpawnList.end();

		for (; iterSpawnInfo != iterSpawnInfoEnd; ++iterSpawnInfo)
		{
			Length = (*iterSpawnInfo)->MonsterName.length();
			fwrite(&Length, sizeof(int), 1, File);
			fwrite((*iterSpawnInfo)->MonsterName.c_str(), sizeof(char), Length, File);
			fwrite(&(*iterSpawnInfo)->SpawnPosition, sizeof(Vector3), 1, File);
			fwrite(&(*iterSpawnInfo)->SpawnRotation, sizeof(Vector3), 1, File);
			fwrite(&(*iterSpawnInfo)->Interval, sizeof(float), 1, File);
		}
	}

	return true;
}

bool CDDInstanceSceneMode::Load(FILE* File)
{
	CDDSceneMode::Load(File);

	int Length = 0;
	char Buf[128] = {};
	fread(&Length, sizeof(int), 1, File);
	fread(Buf, sizeof(char), Length, File);
	m_EnterTriggerObjectName = Buf;

	ZeroMemory(Buf, Length);
	fread(&Length, sizeof(int), 1, File);
	fread(Buf, sizeof(char), Length, File);
	m_BlockerObjectName = Buf;

	int PhaseCount = 0;
	fread(&PhaseCount, sizeof(int), 1, File);

	char NameBuf[128] = {};
	for (int i = 0; i < PhaseCount; ++i)
	{
		DDInstanceSpawnPhaseInfo* Phase = new DDInstanceSpawnPhaseInfo;
		m_SpawnPhaseList.push_back(Phase);

		fread(&Phase->Interval, sizeof(float), 1, File);

		int SpawnCount = 0;
		fread(&SpawnCount, sizeof(int), 1, File);

		for (int j = 0; j < SpawnCount; ++j)
		{
			DDInstanceSpawnInfo* SpawnInfo = new DDInstanceSpawnInfo;
			ZeroMemory(NameBuf, Length);
			fread(&Length, sizeof(int), 1, File);
			fread(NameBuf, sizeof(char), Length, File);
			SpawnInfo->MonsterName = NameBuf;
			fread(&SpawnInfo->SpawnPosition, sizeof(Vector3), 1, File);
			fread(&SpawnInfo->SpawnRotation, sizeof(Vector3), 1, File);
			fread(&SpawnInfo->Interval, sizeof(float), 1, File);
			Phase->SpawnList.push_back(SpawnInfo);
		}
	}

	return true;
}

void CDDInstanceSceneMode::OnDieMonster()
{
	CDDSceneMode::OnDieMonster();

	--m_CurPhaseMonsterCount;

	// ���� �������� ���Ͱ� ���� ó���� ��� ���� ���� �̺�Ʈ �ٽ� ������
	if (m_CurPhaseMonsterCount == 0 && m_MonsterCount != 0)
	{
		m_SpawnEventRunning = true;
	}
}

void CDDInstanceSceneMode::OnClearDungeon()
{
	CDDSceneMode::OnClearDungeon();

	m_BlockerDownMoving = true;
	m_BlockerUpMoving = false;
}

void CDDInstanceSceneMode::AddSpawnPhase()
{
	DDInstanceSpawnPhaseInfo* PhaseInfo = new DDInstanceSpawnPhaseInfo;

	m_SpawnPhaseList.push_back(PhaseInfo);
}

bool CDDInstanceSceneMode::AddSpawnInfo(int PhaseIndex)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* Info = new DDInstanceSpawnInfo;
	Phase->SpawnList.push_back(Info);

	++m_MonsterCount;

	return true;
}

bool CDDInstanceSceneMode::AddSpawnInfo(int PhaseIndex, const std::string& MonsterName,
	const Vector3& SpawnPoint, float Interval)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* Info = new DDInstanceSpawnInfo;

	Info->MonsterName = MonsterName;
	Info->SpawnPosition = SpawnPoint;
	Info->Interval = Interval;

	Phase->SpawnList.push_back(Info);

	++m_MonsterCount;

	return true;
}

bool CDDInstanceSceneMode::DeleteSpawnPhaseInfo(int PhaseIndex)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	auto iter = m_SpawnPhaseList.begin();
	auto iterEnd = m_SpawnPhaseList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter) == Phase)
		{
			m_MonsterCount -= (int)(*iter)->SpawnList.size();
			m_SpawnPhaseList.erase(iter);
			return true;
		}
	}

	return false;
}

bool CDDInstanceSceneMode::SetSpawnPhaseInterval(int PhaseIndex, float Interval)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	Phase->Interval = Interval;

	return true;
}

bool CDDInstanceSceneMode::SetSpawnInfo(int PhaseIndex, int SpawnIndex, const std::string& MonsterName, const Vector3& SpawnPoint, float Interval)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	SpawnInfo->MonsterName = MonsterName;
	SpawnInfo->SpawnPosition = SpawnPoint;
	SpawnInfo->Interval = Interval;

	return true;
}

bool CDDInstanceSceneMode::DeleteSpawnInfo(int PhaseIndex, int SpawnIndex)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	if (!SpawnInfo)
	{
		return false;
	}

	auto iter = Phase->SpawnList.begin();
	auto iterEnd = Phase->SpawnList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter) == SpawnInfo)
		{
			Phase->SpawnList.erase(iter);
			--m_MonsterCount;
			return true;
		}
	}

	return false;
}

bool CDDInstanceSceneMode::SetSpawnMonsterName(int PhaseIndex, int SpawnIndex, const std::string& MonsterName)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	if (!SpawnInfo)
	{
		return false;
	}

	SpawnInfo->MonsterName = MonsterName;
}

bool CDDInstanceSceneMode::SetSpawnPoint(int PhaseIndex, int SpawnIndex, const Vector3& SpawnPoint)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	if (!SpawnInfo)
	{
		return false;
	}

	SpawnInfo->SpawnPosition = SpawnPoint;
}

bool CDDInstanceSceneMode::SetSpawnRotation(int PhaseIndex, int SpawnIndex, const Vector3& SpawnRotation)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	if (!SpawnInfo)
	{
		return false;
	}

	SpawnInfo->SpawnRotation = SpawnRotation;
}

bool CDDInstanceSceneMode::SetSpawnInterval(int PhaseIndex, int SpawnIndex, float Interval)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return false;
	}

	DDInstanceSpawnInfo* SpawnInfo = GetSpawnInfo(Phase, SpawnIndex);

	if (!SpawnInfo)
	{
		return false;
	}

	SpawnInfo->Interval = Interval;
}

void CDDInstanceSceneMode::SetEnterCollider(CColliderBox3D* Collider)
{
	m_EnterTrigger = Collider;
}

size_t CDDInstanceSceneMode::GetSpawnListSize(int PhaseIdx)
{
	DDInstanceSpawnPhaseInfo* PhaseInfo = GetPhaseInfo(PhaseIdx);

	if (!PhaseInfo)
	{
		return -1;
	}

	return PhaseInfo->SpawnList.size();
}

bool CDDInstanceSceneMode::IsValidPhaseIndex(int Index)
{
	if (Index <= -1 || m_SpawnPhaseList.size() - 1 < Index)
	{
		return false;
	}

	return true;
}

bool CDDInstanceSceneMode::IsValidSpawnIndex(int PhaseIndex, int SpawnIndex)
{
	if (!IsValidPhaseIndex(PhaseIndex))
	{
		return false;
	}

	DDInstanceSpawnPhaseInfo* Info = GetPhaseInfo(PhaseIndex);

	if (SpawnIndex <= -1 || Info->SpawnList.size() <= SpawnIndex)
	{
		return false;
	}

	return true;
}

bool CDDInstanceSceneMode::IsValidSpawnIndex(DDInstanceSpawnPhaseInfo* Phase, int SpawnIndex)
{
	if (SpawnIndex <= -1 || Phase->SpawnList.size() <= SpawnIndex)
	{
		return false;
	}

	return true;
}

DDInstanceSpawnPhaseInfo* CDDInstanceSceneMode::GetPhaseInfo(int Index)
{
	if (!IsValidPhaseIndex(Index))
	{
		return nullptr;
	}

	auto iter = m_SpawnPhaseList.begin();
	auto iterEnd = m_SpawnPhaseList.end();

	int Count = 0;
	for (; iter != iterEnd; ++iter)
	{
		if (Count == Index)
		{
			return (*iter);
		}
		++Count;
	}

	return nullptr;
}

DDInstanceSpawnInfo* CDDInstanceSceneMode::GetSpawnInfo(int PhaseIndex, int SpawnIndex)
{
	DDInstanceSpawnPhaseInfo* Phase = GetPhaseInfo(PhaseIndex);

	if (!Phase)
	{
		return nullptr;
	}

	return GetSpawnInfo(Phase, SpawnIndex);
}

DDInstanceSpawnInfo* CDDInstanceSceneMode::GetSpawnInfo(DDInstanceSpawnPhaseInfo* Phase, int SpawnIndex)
{
	if (!IsValidSpawnIndex(Phase, SpawnIndex))
	{
		return nullptr;
	}

	auto iter = Phase->SpawnList.begin();
	auto iterEnd = Phase->SpawnList.end();

	int Count = 0;
	for (; iter != iterEnd; ++iter)
	{
		if (Count == SpawnIndex)
		{
			return (*iter);
		}
		++Count;
	}

	return nullptr;
}

void CDDInstanceSceneMode::OnCollideEnterTrigger(const CollisionResult& Result)
{
	CGameObject* Player = m_Scene->GetPlayerObject();

	if (Player && Result.Dest->GetGameObject() == Player)
	{
		m_BlockerUpMoving = true;
		m_SpawnEventRunning = true;

		if (m_BlockerObj)
		{
			m_BlockerObj->Enable(true);
		}

		m_EnterTrigger->GetGameObject()->Enable(false);
	}
}

void CDDInstanceSceneMode::OnSpawnDoorPaperBurnEnd()
{
	DDSpawnObjectSet Set = m_PaperBurnEndSpawnQueue.front();
	m_PaperBurnEndSpawnQueue.pop();

	// ���͸� ��ȯ�Ѵ�.
	CGameObject* Monster = CObjectPool::GetInst()->GetMonster(Set.Info->MonsterName, m_Scene);
	Monster->SetWorldPos(Set.Info->SpawnPosition);
	Monster->SetWorldRotation(Set.Info->SpawnRotation);

	CGameStateComponent* State = Monster->FindObjectComponentFromType<CGameStateComponent>();
	State->SetTreeUpdate(true);

	// �����۹� ����� �� �� �ı��ϱ� ���� ť�� �ִ´�.
	Set.DoorPaperBurn->SetInverse(false);
	Set.DoorPaperBurn->SetFinishCallback(this, &CDDInstanceSceneMode::OnSpawnDoorDestroy);
	Set.DoorPaperBurn->StartPaperBurn();
	m_DoorPaperburnQueue.push(Set.DoorPaperBurn);

	// �޸� ����
	SAFE_DELETE(Set.Info);
}

void CDDInstanceSceneMode::OnSpawnDoorDestroy()
{
	CPaperBurnComponent* PaperBurn = m_DoorPaperburnQueue.front();
	m_DoorPaperburnQueue.pop();

	PaperBurn->GetGameObject()->Destroy();
}
