#include "Standard3DWireFrameShader.h"

CStandard3DWireFrameShader::CStandard3DWireFrameShader()
{
	SetTypeID<CStandard3DWireFrameShader>();
}

CStandard3DWireFrameShader::~CStandard3DWireFrameShader()
{
}

bool CStandard3DWireFrameShader::Init()
{
	if (!LoadVertexShader("Standard3DWireFrameVS", TEXT("Standard3D.fx"), SHADER_PATH))
		return false;

	if (!LoadPixelShader("Standard3DWireFramePS", TEXT("Standard3D.fx"), SHADER_PATH))
		return false;

	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0);
	AddInputDesc("BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0);

	if (!CreateInputLayout())
		return false;

	return true;
}
