#pragma once

#include "../../Ref.h"

class CSound :
    public CRef
{
    friend class CSoundManager;

private:
    CSound();
    ~CSound();

private:
    FMOD::System* m_System;
    FMOD::Sound* m_Sound;
    FMOD::ChannelGroup* m_Group;
    FMOD::Channel* m_Channel;
    bool        m_Play;
    bool        m_Loop;
    int         m_Volume;
    std::string m_ChannelGroupName;

public:
    bool IsPlay()   const
    {
        return m_Play;
    }

    bool IsLoop()   const
    {
        return m_Loop;
    }

    void SetVolume(int Volume)
    {
        m_Volume = Volume;
    }

    int GetVolume() const
    {
        return m_Volume;
    }

    const std::string& GetChannelGroupName()    const
    {
        return m_ChannelGroupName;
    }

public:
    bool LoadSound(FMOD::System* System, FMOD::ChannelGroup* Group,
        bool Loop, const std::string& Name, const char* FileName, const std::string& PathName = SOUND_PATH);
    void Play();
    void Stop();
    void Pause();
    void Resume();
};

