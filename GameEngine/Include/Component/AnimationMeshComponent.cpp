
#include "AnimationMeshComponent.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneResource.h"
#include "../Render/RenderManager.h"
#include "../Resource/Shader/Standard2DConstantBuffer.h"
#include "../Animation/AnimationSequenceInstance.h"
#include "../GameObject/GameObject.h"

CAnimationMeshComponent::CAnimationMeshComponent()
{
	SetTypeID<CAnimationMeshComponent>();
	m_Render = true;
	m_Animation = nullptr;
	m_Transform->SetTransformState(Transform_State::Ground);
	
	m_InstanceID = 0;
}

CAnimationMeshComponent::CAnimationMeshComponent(const CAnimationMeshComponent& com) :
	CSceneComponent(com)
{
	m_Mesh = com.m_Mesh;

	m_vecMaterialSlot.clear();

	for (size_t i = 0; i < com.m_vecMaterialSlot.size(); ++i)
	{
		m_vecMaterialSlot.push_back(com.m_vecMaterialSlot[i]->Clone());
	}

	m_Skeleton = com.m_Skeleton->Clone();

	if (com.m_Animation)
	{
		m_Animation = com.m_Animation->Clone();
		m_Animation->SetSkeleton(m_Skeleton);
		m_Animation->SetInstancingBoneBuffer(m_Mesh->GetBoneBuffer());
	}
}

CAnimationMeshComponent::~CAnimationMeshComponent()
{
	auto	iter = m_InstancingCheckList.begin();
	auto	iterEnd = m_InstancingCheckList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->Mesh == m_Mesh)
		{
			auto	iter1 = (*iter)->InstancingList.begin();
			auto	iter1End = (*iter)->InstancingList.end();

			for (; iter1 != iter1End; ++iter1)
			{
				if (*iter1 == this)
				{
					(*iter)->InstancingList.erase(iter1);

					if ((*iter)->InstancingList.size() < 10)
					{
						auto	iter2 = (*iter)->InstancingList.begin();
						auto	iter2End = (*iter)->InstancingList.end();

						for (; iter2 != iter2End; ++iter2)
						{
							((CAnimationMeshComponent*)(*iter2))->SetInstanceID(0);
							(*iter2)->SetInstancing(false);
						}
					}
					else
					{
						auto	iter2 = (*iter)->InstancingList.begin();
						auto	iter2End = (*iter)->InstancingList.end();

						int ID = 0;

						for (; iter2 != iter2End; ++iter2, ++ID)
						{
							((CAnimationMeshComponent*)(*iter2))->SetInstanceID(ID);
							(*iter2)->SetInstancing(true);
						}
					}
					break;
				}
			}

			if ((*iter)->InstancingList.empty())
			{
				SAFE_DELETE((*iter));
				m_InstancingCheckList.erase(iter);
			}

			break;
		}
	}

	SAFE_DELETE(m_Animation);
}

void CAnimationMeshComponent::SetMesh(const std::string& Name)
{
	m_Mesh = (CAnimationMesh*)m_Scene->GetResource()->FindMesh(Name);

	m_Skeleton = m_Mesh->GetSkeleton()->Clone();

	if (m_Animation)
	{
		m_Animation->SetSkeleton(m_Skeleton);
		m_Animation->SetInstancingBoneBuffer(m_Mesh->GetBoneBuffer());
	}

	m_vecMaterialSlot.clear();

	const std::vector<CSharedPtr<CMaterial>>* pMaterialSlots =
		m_Mesh->GetMaterialSlots();

	std::vector<CSharedPtr<CMaterial>>::const_iterator	iter = pMaterialSlots->begin();
	std::vector<CSharedPtr<CMaterial>>::const_iterator	iterEnd = pMaterialSlots->end();

	for (; iter != iterEnd; ++iter)
	{
		m_vecMaterialSlot.push_back((*iter)->Clone());

		(*iter)->SetScene(m_Scene);
	}

	SetMeshSize(m_Mesh->GetMax() - m_Mesh->GetMin());

	m_SphereInfo.Center = (m_Mesh->GetMax() + m_Mesh->GetMin()) / 2.f;

	auto	iter1 = m_InstancingCheckList.begin();
	auto	iter1End = m_InstancingCheckList.end();

	bool	Add = false;

	for (; iter1 != iter1End; ++iter1)
	{
		if ((*iter1)->Mesh == m_Mesh)
		{
			bool	InstancingEnable = (*iter1)->InstancingList.back()->GetInstancing();

			(*iter1)->InstancingList.push_back(this);
			Add = true;

			// 인스턴싱 개수를 판단한다.
			if (InstancingEnable)
			{
				SetInstancing(InstancingEnable);
				m_InstanceID = (int)(*iter1)->InstancingList.size() - 1;
			}

			else
			{
				if ((*iter1)->InstancingList.size() == 10)
				{
					auto	iter2 = (*iter1)->InstancingList.begin();
					auto	iter2End = (*iter1)->InstancingList.end();

					int ID = 0;
					for (; iter2 != iter2End; ++iter2, ++ID)
					{
						((CAnimationMeshComponent*)(*iter2))->SetInstanceID(ID);
						(*iter2)->SetInstancing(true);
					}

					m_Instancing = true;
				}
			}

			break;
		}
	}

	if (!Add)
	{
		InstancingCheckCount* CheckCount = new InstancingCheckCount;

		m_InstancingCheckList.push_back(CheckCount);

		CheckCount->InstancingList.push_back(this);
		CheckCount->Mesh = m_Mesh;
		CheckCount->LayerName = "Default";
	}
}

void CAnimationMeshComponent::SetMesh(CAnimationMesh* Mesh)
{
	m_Mesh = Mesh;

	m_Skeleton = m_Mesh->GetSkeleton()->Clone();

	if (m_Animation)
	{
		m_Animation->SetSkeleton(m_Skeleton);
		m_Animation->SetInstancingBoneBuffer(m_Mesh->GetBoneBuffer());
	}

	m_vecMaterialSlot.clear();

	const std::vector<CSharedPtr<CMaterial>>* pMaterialSlots =
		m_Mesh->GetMaterialSlots();

	std::vector<CSharedPtr<CMaterial>>::const_iterator	iter = pMaterialSlots->begin();
	std::vector<CSharedPtr<CMaterial>>::const_iterator	iterEnd = pMaterialSlots->end();

	for (; iter != iterEnd; ++iter)
	{
		m_vecMaterialSlot.push_back((*iter)->Clone());

		(*iter)->SetScene(m_Scene);
	}

	SetMeshSize(m_Mesh->GetMax() - m_Mesh->GetMin());

	m_SphereInfo.Center = (m_Mesh->GetMax() + m_Mesh->GetMin()) / 2.f;

	auto	iter1 = m_InstancingCheckList.begin();
	auto	iter1End = m_InstancingCheckList.end();

	bool	Add = false;

	for (; iter1 != iter1End; ++iter1)
	{
		if ((*iter1)->Mesh == m_Mesh)
		{
			bool	InstancingEnable = (*iter1)->InstancingList.back()->GetInstancing();

			(*iter1)->InstancingList.push_back(this);
			Add = true;

			// 인스턴싱 개수를 판단한다.
			if (InstancingEnable)
			{
				SetInstancing(InstancingEnable);
				m_InstanceID = (int)(*iter1)->InstancingList.size() - 1;
			}

			else
			{
				if ((*iter1)->InstancingList.size() == 10)
				{
					auto	iter2 = (*iter1)->InstancingList.begin();
					auto	iter2End = (*iter1)->InstancingList.end();

					int ID = 0;
					for (; iter2 != iter2End; ++iter2, ++ID)
					{
						((CAnimationMeshComponent*)(*iter2))->SetInstanceID(ID);
						(*iter2)->SetInstancing(true);
					}

					m_Instancing = true;
				}
			}

			break;
		}
	}

	if (!Add)
	{
		InstancingCheckCount* CheckCount = new InstancingCheckCount;

		m_InstancingCheckList.push_back(CheckCount);

		CheckCount->InstancingList.push_back(this);
		CheckCount->Mesh = m_Mesh;
		CheckCount->LayerName = "Default";
	}
}

void CAnimationMeshComponent::SetMaterial(CMaterial* Material, int Index)
{
	m_vecMaterialSlot[Index] = Material->Clone();

	m_vecMaterialSlot[Index]->SetScene(m_Scene);
}

void CAnimationMeshComponent::AddMaterial(CMaterial* Material)
{
	m_vecMaterialSlot.push_back(Material->Clone());

	m_vecMaterialSlot[m_vecMaterialSlot.size() - 1]->SetScene(m_Scene);
}

bool CAnimationMeshComponent::SetCustomShader(const std::string& Name)
{
	m_CustomShader = m_Scene->GetResource()->FindShader(Name);

	if (!m_CustomShader)
	{
		assert(false);
		return false;
	}

	// 현재 반투명상태인 Material 체크
	size_t Size = m_vecMaterialSlot.size();

	std::vector<bool> vecTransparent;
	vecTransparent.resize(Size);

	for (size_t i = 0; i < Size; ++i)
	{
		if (m_vecMaterialSlot[i]->IsTransparent())
		{
			vecTransparent[i] = true;
		}
	}

	// 반투명 상태가 아닌 Material들에 Custom Shader 적용
	for (size_t i = 0; i < Size; ++i)
	{
		if (false == vecTransparent[i])
		{
			m_vecMaterialSlot[i]->SetShader((CGraphicShader*)(m_CustomShader.Get()));
		}
	}

	return true;
}

bool CAnimationMeshComponent::SetCustomTransparencyShader(const std::string& Name)
{
	m_CustomTransparentShader = m_Scene->GetResource()->FindShader(Name);

	if (!m_CustomTransparentShader)
	{
		assert(false);
		return false;
	}

	// 현재 반투명상태인 Material 체크
	size_t Size = m_vecMaterialSlot.size();

	std::vector<bool> vecTransparent;
	vecTransparent.resize(Size);

	for (size_t i = 0; i < Size; ++i)
	{
		if (m_vecMaterialSlot[i]->IsTransparent())
		{
			vecTransparent[i] = true;
		}
	}

	// 반투명 상태인 Material들에 Custom Shader 적용
	for (size_t i = 0; i < Size; ++i)
	{
		if (vecTransparent[i])
		{
			m_vecMaterialSlot[i]->SetShader((CGraphicShader*)(m_CustomTransparentShader.Get()));
		}
	}

	return true;
}

void CAnimationMeshComponent::SetBaseColor(const Vector4& Color, int Index)
{
	m_vecMaterialSlot[Index]->SetBaseColor(Color);
}

void CAnimationMeshComponent::SetBaseColor(float r, float g, float b, float a, int Index)
{
	m_vecMaterialSlot[Index]->SetBaseColor(r, g, b, a);
}

void CAnimationMeshComponent::SetAmbientColor(const Vector4& Color, int Index)
{
	m_vecMaterialSlot[Index]->SetAmbientColor(Color);
}

void CAnimationMeshComponent::SetAmbientColor(float r, float g, float b, float a, int Index)
{
	m_vecMaterialSlot[Index]->SetAmbientColor(r, g, b, a);
}

void CAnimationMeshComponent::SetSpecularColor(const Vector4& Color, int Index)
{
	m_vecMaterialSlot[Index]->SetSpecularColor(Color);
}

void CAnimationMeshComponent::SetSpecularColor(float r, float g, float b, float a, int Index)
{
	m_vecMaterialSlot[Index]->SetSpecularColor(r, g, b, a);
}

void CAnimationMeshComponent::SetEmissiveColor(const Vector4& Color, int Index)
{
	m_vecMaterialSlot[Index]->SetEmissiveColor(Color);
}

void CAnimationMeshComponent::SetEmissiveColor(float r, float g, float b, float a, int Index)
{
	m_vecMaterialSlot[Index]->SetEmissiveColor(r, g, b, a);
}

void CAnimationMeshComponent::SetSpecularPower(float Power, int Index)
{
	m_vecMaterialSlot[Index]->SetSpecularPower(Power);
}

void CAnimationMeshComponent::SetRenderState(CRenderState* State, int Index)
{
	m_vecMaterialSlot[Index]->SetRenderState(State);
}

void CAnimationMeshComponent::SetRenderState(const std::string& Name, int Index)
{
	m_vecMaterialSlot[Index]->SetRenderState(Name);
}

void CAnimationMeshComponent::SetTransparency(bool Enable, int Index)
{
	if (Enable)
	{
		// 한 Material이라도 반투명이라면 반투명 Layer에서 렌더해야 한다.
		m_LayerName = "Transparency";

		// 반투명시 적용되어야 할 커스텀 쉐이더가 있는 경우
		if (m_CustomTransparentShader)
		{
			m_vecMaterialSlot[Index]->SetShader((CGraphicShader*)m_CustomTransparentShader.Get());
		}
		// 없는 경우 Default Transparent Shader로 렌더
		else
		{
			m_vecMaterialSlot[Index]->SetShader("Transparent3DShader");
		}

		m_vecMaterialSlot[Index]->SetTransparency(Enable);
	}
	else
	{
		m_vecMaterialSlot[Index]->SetTransparency(Enable);

		// 커스텀 쉐이더가 있는 경우
		if (m_CustomShader)
		{
			m_vecMaterialSlot[Index]->SetShader((CGraphicShader*)m_CustomShader.Get());
		}
		// 없는 경우 Default 3D Shader로 렌더
		else
		{
			m_vecMaterialSlot[Index]->SetShader("Standard3DShader");
		}

		// 하나의 Material이라도 반투명이라면, 레이어를 Transparency 레이어로 유지한다.
		size_t Size = m_vecMaterialSlot.size();
		for (size_t i = 0; i < Size; ++i)
		{
			if (m_vecMaterialSlot[i]->IsTransparent())
			{
				return;
			}
		}

		// 모두 다 불투명 상태라면, Default Layer로 바꾼다.
		m_LayerName = "Default";
	}
}

void CAnimationMeshComponent::SetTransparencyAllMaterial(bool Enable)
{
	size_t Size = m_vecMaterialSlot.size();

	if (Enable)
	{
		m_LayerName = "Transparency";

		for (size_t i = 0; i < Size; ++i)
		{
			if (m_CustomTransparentShader)
			{
				m_vecMaterialSlot[i]->SetShader((CGraphicShader*)m_CustomTransparentShader.Get());
			}
			else
			{
				m_vecMaterialSlot[i]->SetShader("Standard3DShader");
			}
		}
	}
	else
	{
		m_LayerName = "Default";

		for (size_t i = 0; i < Size; ++i)
		{
			if (m_CustomShader)
			{
				m_vecMaterialSlot[i]->SetShader((CGraphicShader*)m_CustomShader.Get());
			}
			else
			{
				m_vecMaterialSlot[i]->SetShader("Transparent3DShader");
			}
		}
	}
}

void CAnimationMeshComponent::SetOpacity(float Opacity, int Index)
{
	m_vecMaterialSlot[Index]->SetOpacity(Opacity);
}

void CAnimationMeshComponent::AddOpacity(float Opacity, int Index)
{
	m_vecMaterialSlot[Index]->AddOpacity(Opacity);
}

void CAnimationMeshComponent::SetMaterialShader(const std::string& Name)
{
	CGraphicShader* Shader = dynamic_cast<CGraphicShader*>(CResourceManager::GetInst()->FindShader(Name));

	if (!Shader)
		return;

	int Size = (int)m_vecMaterialSlot.size();
	for (int i = 0; i < Size; ++i)
	{
		m_vecMaterialSlot[i]->SetShader(Shader);
	}
}

void CAnimationMeshComponent::AddTexture(int MaterialIndex, int Register, int ShaderType, const std::string& Name, CTexture* Texture)
{
	m_vecMaterialSlot[MaterialIndex]->AddTexture(Register, ShaderType, Name, Texture);
}

void CAnimationMeshComponent::AddTexture(int MaterialIndex, int Register, int ShaderType, const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	m_vecMaterialSlot[MaterialIndex]->AddTexture(Register, ShaderType, Name, FileName, PathName);
}

void CAnimationMeshComponent::AddTextureFullPath(int MaterialIndex, int Register, int ShaderType, const std::string& Name, const TCHAR* FullPath)
{
	m_vecMaterialSlot[MaterialIndex]->AddTextureFullPath(Register, ShaderType, Name, FullPath);
}

void CAnimationMeshComponent::AddTexture(int MaterialIndex, int Register, int ShaderType, const std::string& Name,
	const std::vector<TCHAR*>& vecFileName, const std::string& PathName)
{
	m_vecMaterialSlot[MaterialIndex]->AddTexture(Register, ShaderType, Name, vecFileName, PathName);
}

void CAnimationMeshComponent::SetTexture(int MaterialIndex, int Index, int Register, int ShaderType, const std::string& Name, CTexture* Texture)
{
	m_vecMaterialSlot[MaterialIndex]->SetTexture(Index, Register, ShaderType, Name, Texture);
}

void CAnimationMeshComponent::SetTexture(int MaterialIndex, int Index, int Register, int ShaderType, const std::string& Name, const TCHAR* FileName,
	const std::string& PathName)
{
	m_vecMaterialSlot[MaterialIndex]->SetTexture(Index, Register, ShaderType, Name, FileName, PathName);
}

void CAnimationMeshComponent::SetTextureFullPath(int MaterialIndex, int Index, int Register, int ShaderType, const std::string& Name,
	const TCHAR* FullPath)
{
	m_vecMaterialSlot[MaterialIndex]->SetTextureFullPath(Index, Register, ShaderType, Name, FullPath);
}

void CAnimationMeshComponent::SetTexture(int MaterialIndex, int Index, int Register, int ShaderType, const std::string& Name,
	const std::vector<TCHAR*>& vecFileName, const std::string& PathName)
{
	m_vecMaterialSlot[MaterialIndex]->SetTexture(Index, Register, ShaderType, Name, vecFileName, PathName);
}

void CAnimationMeshComponent::Start()
{
	CSceneComponent::Start();

	if (m_Animation)
		m_Animation->Start();
}

bool CAnimationMeshComponent::Init()
{
	//m_Mesh = (CSpriteMesh*)m_Scene->GetResource()->FindMesh("SpriteMesh");
	//SetMaterial(m_Scene->GetResource()->FindMaterial("Color"));
	AddMaterial(m_Scene->GetResource()->FindMaterial("Color"));

	return true;
}

void CAnimationMeshComponent::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);

	if (m_Animation)
		m_Animation->Update(DeltaTime);
}

void CAnimationMeshComponent::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);
}

void CAnimationMeshComponent::PrevRender()
{
	CSceneComponent::PrevRender();
}

void CAnimationMeshComponent::Render()
{
	CSceneComponent::Render();

	if (!m_Mesh)
		return;

	if (m_Animation)
		m_Animation->SetShader();

	size_t	Size = m_vecMaterialSlot.size();

	for (size_t i = 0; i < Size; ++i)
	{
		m_vecMaterialSlot[i]->EnableDecal(m_ReceiveDecal);

		m_vecMaterialSlot[i]->Render();

		m_Mesh->Render((int)i);

		m_vecMaterialSlot[i]->Reset();
	}

	if (m_Animation)
		m_Animation->ResetShader();
}

void CAnimationMeshComponent::RenderShadowMap()
{
	CSceneComponent::RenderShadowMap();

	if (!m_Mesh)
	{
		return;
	}

	if (m_Animation)
	{
		m_Animation->SetShader();
	}

	size_t Size = m_vecMaterialSlot.size();
	for (size_t i = 0; i < Size; ++i)
	{
		m_Mesh->Render((int)i);
	}

	if (m_Animation)
	{
		m_Animation->ResetShader();
	}
}

void CAnimationMeshComponent::PostRender()
{
	CSceneComponent::PostRender();
}

CAnimationMeshComponent* CAnimationMeshComponent::Clone()
{
	return new CAnimationMeshComponent(*this);
}

void CAnimationMeshComponent::Save(FILE* File)
{
	std::string	MeshName = m_Mesh->GetName();

	int	Length = (int)MeshName.length();

	fwrite(&Length, sizeof(int), 1, File);
	fwrite(MeshName.c_str(), sizeof(char), Length, File);

	int	MaterialSlotCount = (int)m_vecMaterialSlot.size();

	fwrite(&MaterialSlotCount, sizeof(int), 1, File);

	for (int i = 0; i < MaterialSlotCount; ++i)
	{
		m_vecMaterialSlot[i]->Save(File);
	}

	CSceneComponent::Save(File);
}

void CAnimationMeshComponent::Load(FILE* File)
{
	char	MeshName[256] = {};

	int	Length = 0;

	fread(&Length, sizeof(int), 1, File);
	fread(MeshName, sizeof(char), Length, File);

	SetMesh(MeshName);

	int	MaterialSlotCount = 0;

	fread(&MaterialSlotCount, sizeof(int), 1, File);

	m_vecMaterialSlot.clear();

	m_vecMaterialSlot.resize(MaterialSlotCount);

	for (int i = 0; i < MaterialSlotCount; ++i)
	{
		m_vecMaterialSlot[i] = new CMaterial;

		m_vecMaterialSlot[i]->Load(File);
	}

	CSceneComponent::Load(File);
}
void CAnimationMeshComponent::RenderAnimationEditor()
{
	CSceneComponent::RenderAnimationEditor();

	if (!m_Mesh)
		return;

	if (m_Animation)
		m_Animation->SetShader();

	size_t	Size = m_vecMaterialSlot.size();

	for (size_t i = 0; i < Size; ++i)
	{
		m_vecMaterialSlot[i]->EnableDecal(m_ReceiveDecal);

		m_vecMaterialSlot[i]->Render();

		m_Mesh->Render((int)i);

		m_vecMaterialSlot[i]->Reset();
	}

	if (m_Animation)
		m_Animation->ResetShader();
}
void CAnimationMeshComponent::DeleteAnimationInstance()
{
	SAFE_DELETE(m_Animation);
}
void CAnimationMeshComponent::AddChild(CSceneComponent* Child,
	const std::string& SocketName)
{
	CSceneComponent::AddChild(Child, SocketName);

	if (m_Skeleton && SocketName != "")
	{
		m_Socket = m_Skeleton->GetSocket(SocketName);

		Child->GetTransform()->SetSocket(m_Socket);
	}
}

void CAnimationMeshComponent::AddChild(CGameObject* Child,
	const std::string& SocketName)
{
	CSceneComponent::AddChild(Child, SocketName);

	if (m_Skeleton && SocketName != "")
	{
		m_Socket = m_Skeleton->GetSocket(SocketName);

		CSceneComponent* ChildComponent = Child->GetRootComponent();

		ChildComponent->GetTransform()->SetSocket(m_Socket);
	}
}
