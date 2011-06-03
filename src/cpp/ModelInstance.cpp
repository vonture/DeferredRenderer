#include "ModelInstance.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _path(path), _transformedMeshBoxes(NULL), _dirty(true)
{
	_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	_scale = 1.0f;
	_orientation = XMQuaternionIdentity();
}

ModelInstance::~ModelInstance()
{
}

//-----------------------------------------------------------------------------
// Transform an oriented box by an angle preserving transform.
//-----------------------------------------------------------------------------
VOID ModelInstance::TransformOrientedBox( OrientedBox* pOut, const OrientedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
                           FXMVECTOR Translation )
{
    XMASSERT( pOut );
    XMASSERT( pIn );
    //XMASSERT( XMQuaternionIsUnit( Rotation ) );

    // Load the box.
    XMVECTOR Center = XMLoadFloat3( &pIn->Center );
    XMVECTOR Extents = XMLoadFloat3( &pIn->Extents );
    XMVECTOR Orientation = XMLoadFloat4( &pIn->Orientation );

    //XMASSERT( XMQuaternionIsUnit( Orientation ) );

    // Composite the box rotation and the transform rotation.
    Orientation = XMQuaternionMultiply( Orientation, Rotation );

    // Transform the center.
    XMVECTOR VectorScale = XMVectorReplicate( Scale );
    Center = XMVector3Rotate( Center * VectorScale, Rotation ) + Translation;

    // Scale the box extents.
    Extents = Extents * VectorScale;

    // Store the box.
    XMStoreFloat3( &pOut->Center, Center );
    XMStoreFloat3( &pOut->Extents, Extents );
    XMStoreFloat4( &pOut->Orientation, Orientation );

    return;
}

void ModelInstance::clean()
{
	XMMATRIX translate = XMMatrixTranslationFromVector(_position);
	XMMATRIX rotate = XMMatrixRotationQuaternion(_orientation);
	XMMATRIX scale = XMMatrixScaling(_scale, _scale, _scale);

	_world = XMMatrixMultiply(scale, XMMatrixMultiply(rotate, translate));
	
	// Create the bounding boxes...

	// Create an oriented box that sits in the same location as the model's boxes
	// and then transform it
	AxisAlignedBox modelMainBox = _model.GetBoundingBox();	
	_transformedMainBox.Center = modelMainBox.Center;
	_transformedMainBox.Extents = modelMainBox.Extents;
	XMStoreFloat4(&_transformedMainBox.Orientation, XMQuaternionIdentity());

	TransformOrientedBox(&_transformedMainBox, &_transformedMainBox, _scale,
			_orientation, _position);

	UINT meshCount = _model.GetMeshCount();
	for (UINT i = 0; i < meshCount; i++)
	{
		AxisAlignedBox modelMeshBox = _model.GetMeshBoundingBox(i);
		_transformedMeshBoxes[i].Center = modelMeshBox.Center;
		_transformedMeshBoxes[i].Extents = modelMeshBox.Extents;
		XMStoreFloat4(&_transformedMeshBoxes[i].Orientation, XMQuaternionIdentity());

		TransformOrientedBox(&_transformedMeshBoxes[i], &_transformedMeshBoxes[i], _scale,
			_orientation, _position);
	}

	_dirty = false;
}

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_model.CreateFromSDKMeshFile(pd3dDevice, _path));
	
	UINT meshCount = _model.GetMeshCount();
	_transformedMeshBoxes = new OrientedBox[meshCount];

	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice()
{
	_model.Destroy();

	SAFE_DELETE_ARRAY(_transformedMeshBoxes);
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain()
{
}