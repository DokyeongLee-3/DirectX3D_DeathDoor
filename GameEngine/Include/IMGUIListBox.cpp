#include "IMGUIListBox.h"

CIMGUIListBox::CIMGUIListBox()  :
    m_Select(false),
    m_SelectIndex(-1),
    m_PageItemCount(3)
{
}

CIMGUIListBox::~CIMGUIListBox()
{
    m_vecItem.clear();
    m_vecItemUTF8.clear();
}

bool CIMGUIListBox::Init()
{
    return true;
}

void CIMGUIListBox::Render()
{
    ImVec2  Size = ImVec2(m_Size.x, m_PageItemCount * ImGui::GetTextLineHeightWithSpacing());

    //ImGui::Text(m_Name.c_str());

    if (ImGui::BeginListBox(m_Name.c_str(), Size))
    {
        size_t  ItemCount = m_vecItemUTF8.size();

        for (size_t i = 0; i < ItemCount; ++i)
        {
            m_Select = m_SelectIndex == i;

            if (ImGui::Selectable(m_vecItemUTF8[i].c_str(), m_Select))
            {
                if (m_SelectIndex != i && m_SelectCallback)
                    m_SelectCallback((int)i, m_vecItem[i].c_str());

                m_SelectIndex = (int)i;
            }

            if (m_Select)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndListBox();
    }
}
