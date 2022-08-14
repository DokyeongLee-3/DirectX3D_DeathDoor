
#include "UIManager.h"
#include "Widget/WidgetWindow.h"
#include "Widget/Image.h"
#include "Resource/ResourceManager.h"
#include "PathManager.h"
#include "Scene/SceneManager.h"
#include "../Scene/DDSceneMode.h"
#include "../UI/DDMouseWidgetWindow.h"

#include <sstream>

DEFINITION_SINGLE(CUIManager)

CUIManager::CUIManager()	:
	m_Viewport(nullptr),
	m_AbilityIconOriginSize(Vector2(50.f, 50.f)),
	m_AbilityBoxOriginSize(Vector2(85.f, 85.f)),
	m_Window(nullptr)
{
}

CUIManager::~CUIManager()
{

}

void CUIManager::Init()
{
	CSceneManager::GetInst()->SetIsKeepUIFunction(this, &CUIManager::OnSceneChangeKeepUI);
}

void CUIManager::ActivateAbility(Player_Ability Ability)
{
	switch (Ability)
	{
	case Player_Ability::Arrow:
	{
		CImage* ArrowIcon = m_Window->FindWidget<CImage>("ArrowIcon");
		CImage* ArrowBox = m_Window->FindWidget<CImage>("Arrow_abilty_box");
		ArrowIcon->SetSize(m_AbilityIconOriginSize * 1.1f);
		ArrowBox->SetSize(m_AbilityBoxOriginSize * 1.1f);

		CImage* BombIcon = m_Window->FindWidget<CImage>("BombIcon");
		CImage* HookIcon = m_Window->FindWidget<CImage>("HookIcon");
		CImage* BombBox = m_Window->FindWidget<CImage>("Bomb_abilty_box");
		CImage* HookBox = m_Window->FindWidget<CImage>("Hook_abilty_box");
		BombIcon->SetSize(m_AbilityIconOriginSize);
		HookIcon->SetSize(m_AbilityIconOriginSize);
		BombBox->SetSize(m_AbilityBoxOriginSize);
		HookBox->SetSize(m_AbilityBoxOriginSize);
		break;
	}
	case Player_Ability::Hook:
	{
		CImage* HookIcon = m_Window->FindWidget<CImage>("HookIcon");
		CImage* HookBox = m_Window->FindWidget<CImage>("Hook_abilty_box");
		HookIcon->SetSize(m_AbilityIconOriginSize * 1.1f);
		HookBox->SetSize(m_AbilityBoxOriginSize * 1.1f);

		CImage* BombIcon = m_Window->FindWidget<CImage>("BombIcon");
		CImage* ArrowIcon = m_Window->FindWidget<CImage>("ArrowIcon");
		CImage* BombBox = m_Window->FindWidget<CImage>("Bomb_abilty_box");
		CImage* ArrowBox = m_Window->FindWidget<CImage>("Arrow_abilty_box");
		BombIcon->SetSize(m_AbilityIconOriginSize);
		ArrowIcon->SetSize(m_AbilityIconOriginSize);
		BombBox->SetSize(m_AbilityBoxOriginSize);
		ArrowBox->SetSize(m_AbilityBoxOriginSize);
		break;
	}
	case Player_Ability::Bomb:
	{
		CImage* BombIcon = m_Window->FindWidget<CImage>("BombIcon");
		BombIcon->SetSize(m_AbilityIconOriginSize * 1.1f);

		CImage* HookIcon = m_Window->FindWidget<CImage>("HookIcon");
		CImage* ArrowIcon = m_Window->FindWidget<CImage>("ArrowIcon");
		CImage* HookBox = m_Window->FindWidget<CImage>("Hook_abilty_box");
		CImage* ArrowBox = m_Window->FindWidget<CImage>("Arrow_abilty_box");
		HookIcon->SetSize(m_AbilityIconOriginSize);
		ArrowIcon->SetSize(m_AbilityIconOriginSize);
		HookBox->SetSize(m_AbilityBoxOriginSize);
		ArrowBox->SetSize(m_AbilityBoxOriginSize);
		break;
	}
	}
}

bool CUIManager::OnSceneChangeKeepUI(CScene* CurScene, CScene* NextScene)
{
	CDDSceneMode* CurSceneMode = dynamic_cast<CDDSceneMode*>(CurScene->GetSceneMode());
	CDDSceneMode* NextSceneMode = dynamic_cast<CDDSceneMode*>(NextScene->GetSceneMode());

	// ���� ���� ���� ���� �ϳ��� DDSceneMode�� �ƴ϶��
	if (!CurSceneMode || !NextSceneMode)
	{
		return false;
	}

	return true;
}

void CUIManager::CreateDeathDoorUI()
{
	CResourceManager::GetInst()->LoadCSV("UI.csv");

	CExcelData* Excel = CResourceManager::GetInst()->FindCSV("DeathDoor_UI");

	const Table& table = Excel->GetTable();
	size_t LabelCount = Excel->GetLabels().size();
	size_t Index = 0;

	auto iter = table.begin();
	auto iterEnd = table.end();

	m_Window = m_Viewport->CreateWidgetWindow<CWidgetWindow>("HUD");
	m_Window->SetPos(0.f, 0.f);

	for (; iter != iterEnd; ++iter)
	{
		Row* row = iter->second;
		std::stringstream ss;
		float PosX = 0;
		float PosY = 0;
		float SizeX = 0;
		float SizeY = 0;
		int ZOrder = 0;
		std::string Path;
		std::string Key = iter->first;

		for (size_t i = 0; i < LabelCount; ++i)
		{
			ss.str((*row)[i]);

			switch (i)
			{
			case 0:
				ss >> PosX;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}

				ss >> PosX;
				break;
			case 1:
				ss >> PosY;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}

				ss >> PosY;
				break;
			case 2:
				ss >> SizeX;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}
				ss >> SizeX;
				break;
			case 3:
				ss >> SizeY;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}
				ss >> SizeY;
				break;
			case 4:
				ss >> Path;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}
				ss >> Path;
				break;
			case 5:
				ss >> ZOrder;
				while (ss.fail())
				{
					ss.clear();
					ss.str((*row)[i]);
				}
				ss >> ZOrder;
				break;
			}

			ss.str("");
		}

		char FileName[MAX_PATH];
		strcpy_s(FileName, Path.c_str());
		TCHAR FileNameTChar[MAX_PATH] = {};

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, FileName, strlen(FileName), FileNameTChar, MAX_PATH);


		CImage* Widget = m_Window->CreateWidget<CImage>(Key);
		Widget->SetTexture(Key, FileNameTChar);
		Widget->SetPos(PosX, PosY);
		Widget->SetSize(SizeX, SizeY);
		Widget->SetZOrder(ZOrder);
	} 

	CEngine::GetInst()->CreateMouse<CDDMouseWidgetWindow>(Mouse_State::Normal, "DeathDoorMouse");
}