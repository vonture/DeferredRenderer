#include "ModelInstance.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _path(path), _transformedMeshOrientedBoxes(NULL), _transformedMeshAxisBoxes(NULL),
	  _dirty(true)
{
	_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	_scale = 1.0f;
	_orientation = XMQuaternionIdentity();
}

ModelInstance::~ModelInstance()
{
}

static const XMVECTOR g_UnitQuaternionEpsilon =
{
    1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f
};

//-----------------------------------------------------------------------------
// Return TRUE if the quaterion is a unit quaternion.
//-----------------------------------------------------------------------------
static inline BOOL XMQuaternionIsUnit( FXMVECTOR Q )
{
    XMVECTOR Difference = XMVector4Length( Q ) - XMVectorSplatOne();

    return XMVector4Less( XMVectorAbs( Difference ), g_UnitQuaternionEpsilon );
}

//-----------------------------------------------------------------------------
// Transform an axis aligned box by an angle preserving transform.
//-----------------------------------------------------------------------------
VOID Collision::TransformAxisAlignedBox( AxisAlignedBox* pOut, const AxisAlignedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
                              FXMVECTOR Translation )
{
    XMASSERT( pOut );
    XMASSERT( pIn );
    XMASSERT( XMQuaternionIsUnit( Rotation ) );

    static XMVECTOR Offset[8] =
    {
        { -1.0f, -1.0f, -1.0f, 0.0f },
        { -1.0f, -1.0f,  1.0f, 0.0f },
        { -1.0f,  1.0f, -1.0f, 0.0f },
        { -1.0f,  1.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, -1.0f, 0.0f },
        {  1.0f, -1.0f,  1.0f, 0.0f },
        {  1.0f,  1.0f, -1.0f, 0.0f },
        {  1.0f,  1.0f,  1.0f, 0.0f }
    };

    // Load center and extents.
    XMVECTOR Center = XMLoadFloat3( &pIn->Center );
    XMVECTOR Extents = XMLoadFloat3( &pIn->Extents );

    XMVECTOR VectorScale = XMVectorReplicate( Scale );

    // Compute and transform the corners and find new min/max bounds.
    XMVECTOR Corner = Center + Extents * Offset[0];
    Corner = XMVector3Rotate( Corner * VectorScale, Rotation ) + Translation;

    XMVECTOR Min, Max;
    Min = Max = Corner;

    for( INT i = 1; i < 8; i++ )
    {
        Corner = Center + Extents * Offset[i];
        Corner = XMVector3Rotate( Corner * VectorScale, Rotation ) + Translation;

        Min = XMVectorMin( Min, Corner );
        Max = XMVectorMax( Max, Corner );
    }

    // Store center and extents.
    XMStoreFloat3( &pOut->Center, ( Min + Max ) * 0.5f );
    XMStoreFloat3( &pOut->Extents, ( Max - Min ) * 0.5f );

    return;
}

//-----------------------------------------------------------------------------
// Transform an oriented box by an angle preserving transform.
//-----------------------------------------------------------------------------
VOID Collision::TransformOrientedBox( OrientedBox* pOut, const OrientedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
                           FXMVECTOR Translation )
{
    XMASSERT( pOut );
    XMASSERT( pIn );
    XMASSERT( XMQuaternionIsUnit( Rotation ) );

    // Load the box.
    XMVECTOR Center = XMLoadFloat3( &pIn->Center );
    XMVECTOR Extents = XMLoadFloat3( &pIn->Extents );
    XMVECTOR Orientation = XMLoadFloat4( &pIn->Orientation );

    XMASSERT( XMQuaternionIsUnit( Orientation ) );

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
	_transformedMainAxisBox = _model.GetAxisAlignedBox();	
	_transformedMainOrientedBox.Center = _transformedMainAxisBox.Center;
	_transformedMainOrientedBox.Extents = _transformedMainAxisBox.Extents;
	XMStoreFloat4(&_transformedMainOrientedBox.Orientation, XMQuaternionIdentity());

	Collision::TransformAxisAlignedBox(&_transformedMainAxisBox, &_transformedMainAxisBox, 
		_scale, _orientation, _position);
	Collision::TransformOrientedBox(&_transformedMainOrientedBox, &_transformedMainOrientedBox, 
		_scale, _orientation, _position);

	UINT meshCount = _model.GetMeshCount();
	for (UINT i = 0; i < meshCount; i++)
	{
		_transformedMeshAxisBoxes[i] = _model.GetMeshAxisAlignedBox(i);
		_transformedMeshOrientedBoxes[i].Center = _transformedMeshAxisBoxes[i].Center;
		_transformedMeshOrientedBoxes[i].Extents = _transformedMeshAxisBoxes[i].Extents;
		XMStoreFloat4(&_transformedMeshOrientedBoxes[i].Orientation, XMQuaternionIdentity());

		Collision::TransformAxisAlignedBox(&_transformedMeshAxisBoxes[i], &_transformedMeshAxisBoxes[i],
			_scale, _orientation, _position);
		Collision::TransformOrientedBox(&_transformedMeshOrientedBoxes[i], &_transformedMeshOrientedBoxes[i],
			_scale, _orientation, _position);
	}

	_dirty = false;
}

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_model.CreateFromSDKMeshFile(pd3dDevice, _path));
	
	UINT meshCount = _model.GetMeshCount();
	_transformedMeshOrientedBoxes = new OrientedBox[meshCount];
	_transformedMeshAxisBoxes = new AxisAlignedBox[meshCount];

	_dirty = true;

	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice()
{
	_model.Destroy();

	SAFE_DELETE_ARRAY(_transformedMeshOrientedBoxes);
	SAFE_DELETE_ARRAY(_transformedMeshAxisBoxes);

	_dirty = true;
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain()
{
}