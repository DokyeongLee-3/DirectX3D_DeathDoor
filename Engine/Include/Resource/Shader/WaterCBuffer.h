#pragma once

#include "ConstantBufferBase.h"

class CWaterCBuffer :
    public CConstantBufferBase
{
public:
	CWaterCBuffer();
	CWaterCBuffer(const CWaterCBuffer& Buffer);
	virtual ~CWaterCBuffer();

protected:
	WaterCBuffer m_BufferData;

public:
	virtual bool Init();
	virtual void UpdateCBuffer();
	virtual CWaterCBuffer* Clone();

public:
	void SetSpeed(float Speed)
	{
		m_BufferData.Speed = Speed;
	}

	void SetFoamDepth(float Depth)
	{
		m_BufferData.FoamDepthThreshold = Depth;
	}

	float GetSpeed() const
	{
		return m_BufferData.Speed;
	}

	float GetFoamDepth() const
	{
		return m_BufferData.FoamDepthThreshold;
	}
};

