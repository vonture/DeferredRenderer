#include "CascadedDirectionalLightRenderer.h"
#include "Logger.h"

const float CascadedDirectionalLightRenderer::CASCADE_SPLITS[NUM_CASCADES] = { 0.125f, 0.25f, 0.5f, 1.0f };
const float CascadedDirectionalLightRenderer::BIAS = 0.005f;

CascadedDirectionalLightRenderer::CascadedDirectionalLightRenderer()
	: _depthVS(NULL), _depthInput(NULL), _depthPropertiesBuffer(NULL),  _unshadowedPS(NULL),
	  _shadowedPS(NULL), _cameraPropertiesBuffer(NULL), _lightPropertiesBuffer(NULL),
	  _shadowPropertiesBuffer(NULL)
{
	for (int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		_shadowMapTextures[i] = NULL;
		_shadowMapDSVs[i] = NULL;
		_shadowMapSRVs[i] = NULL;
	}
}

HRESULT CascadedDirectionalLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, 
	std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	if (GetCount(true) > 0)
	{
		BEGIN_EVENT_D3D(L"Directional Light Shadow Maps");

		// Save the old viewport
		D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
		UINT nViewPorts = 1;
		pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);
	
		// Iterate over the lights and render the shadow maps
		for (UINT i = 0; i < GetCount(true) && i < NUM_SHADOW_MAPS; i++)
		{
			renderDepth(pd3dImmediateContext, GetLight(i, true), i, models, camera, sceneBounds);
		}

		// Re-apply the old viewport
		pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);

		END_EVENT_D3D(L"");
	}
	return S_OK;
}

//--------------------------------------------------------------------------------------
// Used to compute an intersection of the orthographic projection and the Scene AABB
//--------------------------------------------------------------------------------------
struct Triangle 
{
    XMVECTOR pt[3];
    BOOL culled;
};


//--------------------------------------------------------------------------------------
// Computing an accurate near and flar plane will decrease surface acne and Peter-panning.
// Surface acne is the term for erroneous self shadowing.  Peter-panning is the effect where
// shadows disappear near the base of an object.
// As offsets are generally used with PCF filtering due self shadowing issues, computing the
// correct near and far planes becomes even more important.
// This concept is not complicated, but the intersection code is.
//--------------------------------------------------------------------------------------
void CascadedDirectionalLightRenderer::ComputeNearAndFar( FLOAT& fNearPlane, 
                                        FLOAT& fFarPlane, 
                                        FXMVECTOR vLightCameraOrthographicMin, 
                                        FXMVECTOR vLightCameraOrthographicMax, 
                                        XMVECTOR* pvPointsInCameraView ) 
{

    // Initialize the near and far planes
    fNearPlane = FLT_MAX;
    fFarPlane = -FLT_MAX;
    
    Triangle triangleList[16];
    INT iTriangleCnt = 1;

    triangleList[0].pt[0] = pvPointsInCameraView[0];
    triangleList[0].pt[1] = pvPointsInCameraView[1];
    triangleList[0].pt[2] = pvPointsInCameraView[2];
    triangleList[0].culled = false;

    // These are the indices used to tesselate an AABB into a list of triangles.
    static const INT iAABBTriIndexes[] = 
    {
        0,1,2,  1,2,3,
        4,5,6,  5,6,7,
        0,2,4,  2,4,6,
        1,3,5,  3,5,7,
        0,1,4,  1,4,5,
        2,3,6,  3,6,7 
    };

    INT iPointPassesCollision[3];

    // At a high level: 
    // 1. Iterate over all 12 triangles of the AABB.  
    // 2. Clip the triangles against each plane. Create new triangles as needed.
    // 3. Find the min and max z values as the near and far plane.
    
    //This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.
    
    float fLightCameraOrthographicMinX = XMVectorGetX( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxX = XMVectorGetX( vLightCameraOrthographicMax ); 
    float fLightCameraOrthographicMinY = XMVectorGetY( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxY = XMVectorGetY( vLightCameraOrthographicMax );
    
    for( INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter ) 
    {

        triangleList[0].pt[0] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 0 ] ];
        triangleList[0].pt[1] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 1 ] ];
        triangleList[0].pt[2] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 2 ] ];
        iTriangleCnt = 1;
        triangleList[0].culled = FALSE;

        // Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
        //add them to the list.
        for( INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter ) 
        {

            FLOAT fEdge;
            INT iComponent;
            
            if( frustumPlaneIter == 0 ) 
            {
                fEdge = fLightCameraOrthographicMinX; // todo make float temp
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 1 ) 
            {
                fEdge = fLightCameraOrthographicMaxX;
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 2 ) 
            {
                fEdge = fLightCameraOrthographicMinY;
                iComponent = 1;
            } 
            else 
            {
                fEdge = fLightCameraOrthographicMaxY;
                iComponent = 1;
            }

            for( INT triIter=0; triIter < iTriangleCnt; ++triIter ) 
            {
                // We don't delete triangles, so we skip those that have been culled.
                if( !triangleList[triIter].culled ) 
                {
                    INT iInsideVertCount = 0;
                    XMVECTOR tempOrder;
                    // Test against the correct frustum plane.
                    // This could be written more compactly, but it would be harder to understand.
                    
                    if( frustumPlaneIter == 0 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) >
                                XMVectorGetX( vLightCameraOrthographicMin ) ) 
                            { 
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 1 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetX( vLightCameraOrthographicMax ) )
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            { 
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 2 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) > 
                                XMVectorGetY( vLightCameraOrthographicMin ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetY( vLightCameraOrthographicMax ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }

                    // Move the points that pass the frustum test to the begining of the array.
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    if( iPointPassesCollision[2] && !iPointPassesCollision[1] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[1];   
                        triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
                        triangleList[triIter].pt[2] = tempOrder;
                        iPointPassesCollision[1] = TRUE;            
                        iPointPassesCollision[2] = FALSE;                        
                    }
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    
                    if( iInsideVertCount == 0 ) 
                    { // All points failed. We're done,  
                        triangleList[triIter].culled = true;
                    }
                    else if( iInsideVertCount == 1 ) 
                    {// One point passed. Clip the triangle against the Frustum plane
                        triangleList[triIter].culled = false;
                        
                        // 
                        XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
                        XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];
                        
                        // Find the collision ratio.
                        FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex( triangleList[triIter].pt[0], iComponent ) ;
                        // Calculate the distance along the vector as ratio of the hit ratio to the component.
                        FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert1, iComponent );
                        FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert2, iComponent );
                        // Add the point plus a percentage of the vector.
                        vVert0ToVert1 *= fDistanceAlongVector01;
                        vVert0ToVert1 += triangleList[triIter].pt[0];
                        vVert0ToVert2 *= fDistanceAlongVector02;
                        vVert0ToVert2 += triangleList[triIter].pt[0];

                        triangleList[triIter].pt[1] = vVert0ToVert2;
                        triangleList[triIter].pt[2] = vVert0ToVert1;

                    }
                    else if( iInsideVertCount == 2 ) 
                    { // 2 in  // tesselate into 2 triangles
                        

                        // Copy the triangle\(if it exists) after the current triangle out of
                        // the way so we can override it with the new triangle we're inserting.
                        triangleList[iTriangleCnt] = triangleList[triIter+1];

                        triangleList[triIter].culled = false;
                        triangleList[triIter+1].culled = false;
                        
                        // Get the vector from the outside point into the 2 inside points.
                        XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
                        XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];
                        
                        // Get the hit point ratio.
                        FLOAT fHitPointTime_2_0 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent );
                        FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex( vVert2ToVert0, iComponent );
                        // Calcaulte the new vert by adding the percentage of the vector plus point 2.
                        vVert2ToVert0 *= fDistanceAlongVector_2_0;
                        vVert2ToVert0 += triangleList[triIter].pt[2];
                        
                        // Add a new triangle.
                        triangleList[triIter+1].pt[0] = triangleList[triIter].pt[0];
                        triangleList[triIter+1].pt[1] = triangleList[triIter].pt[1];
                        triangleList[triIter+1].pt[2] = vVert2ToVert0;
                        
                        //Get the hit point ratio.
                        FLOAT fHitPointTime_2_1 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent ) ;
                        FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex( vVert2ToVert1, iComponent );
                        vVert2ToVert1 *= fDistanceAlongVector_2_1;
                        vVert2ToVert1 += triangleList[triIter].pt[2];
                        triangleList[triIter].pt[0] = triangleList[triIter+1].pt[1];
                        triangleList[triIter].pt[1] = triangleList[triIter+1].pt[2];
                        triangleList[triIter].pt[2] = vVert2ToVert1;
                        // Cncrement triangle count and skip the triangle we just inserted.
                        ++iTriangleCnt;
                        ++triIter;

                    
                    }
                    else 
                    { // all in
                        triangleList[triIter].culled = false;

                    }
                }// end if !culled loop            
            }
        }
        for( INT index=0; index < iTriangleCnt; ++index ) 
        {
            if( !triangleList[index].culled ) 
            {
                // Set the near and far plan and the min and max z values respectivly.
                for( int vertind = 0; vertind < 3; ++ vertind ) 
                {
                    float fTriangleCoordZ = XMVectorGetZ( triangleList[index].pt[vertind] );
                    if( fNearPlane > fTriangleCoordZ ) 
                    {
                        fNearPlane = fTriangleCoordZ;
                    }
                    if( fFarPlane  <fTriangleCoordZ ) 
                    {
                        fFarPlane = fTriangleCoordZ;
                    }
                }
            }
        }
    }    

}

//--------------------------------------------------------------------------------------
// This function converts the "center, extents" version of an AABB into 8 points.
//--------------------------------------------------------------------------------------
void CascadedDirectionalLightRenderer::CreateAABBPoints( XMVECTOR* vAABBPoints, FXMVECTOR vCenter, FXMVECTOR vExtents )
{
    //This map enables us to use a for loop and do vector math.
    static const XMVECTORF32 vExtentsMap[] = 
    { 
        {1.0f, 1.0f, -1.0f, 1.0f}, 
        {-1.0f, 1.0f, -1.0f, 1.0f}, 
        {1.0f, -1.0f, -1.0f, 1.0f}, 
        {-1.0f, -1.0f, -1.0f, 1.0f}, 
        {1.0f, 1.0f, 1.0f, 1.0f}, 
        {-1.0f, 1.0f, 1.0f, 1.0f}, 
        {1.0f, -1.0f, 1.0f, 1.0f}, 
        {-1.0f, -1.0f, 1.0f, 1.0f} 
    };
    
    for( INT index = 0; index < 8; ++index ) 
    {
        vAABBPoints[index] = XMVectorMultiplyAdd(vExtentsMap[index], vExtents, vCenter ); 
    }

}

//--------------------------------------------------------------------------------------
// This function takes the camera's projection matrix and returns the 8
// points that make up a view frustum.
// The frustum is scaled to fit within the Begin and End interval paramaters.
//--------------------------------------------------------------------------------------
void CascadedDirectionalLightRenderer::CreateFrustumPointsFromCascadeInterval( float fCascadeIntervalBegin, 
                                                        FLOAT fCascadeIntervalEnd, 
                                                        XMMATRIX &vProjection,
                                                        XMVECTOR* pvCornerPointsWorld ) 
{

    Frustum vViewFrust;
	Collision::ComputeFrustumFromProjection( &vViewFrust, &vProjection );
    vViewFrust.Near = fCascadeIntervalBegin;
    vViewFrust.Far = fCascadeIntervalEnd;

    static const XMVECTORU32 vGrabY = {0x00000000,0xFFFFFFFF,0x00000000,0x00000000};
    static const XMVECTORU32 vGrabX = {0xFFFFFFFF,0x00000000,0x00000000,0x00000000};

    XMVECTORF32 vRightTop = {vViewFrust.RightSlope,vViewFrust.TopSlope,1.0f,1.0f};
    XMVECTORF32 vLeftBottom = {vViewFrust.LeftSlope,vViewFrust.BottomSlope,1.0f,1.0f};
    XMVECTORF32 vNear = {vViewFrust.Near,vViewFrust.Near,vViewFrust.Near,1.0f};
    XMVECTORF32 vFar = {vViewFrust.Far,vViewFrust.Far,vViewFrust.Far,1.0f};
    XMVECTOR vRightTopNear = XMVectorMultiply( vRightTop, vNear );
    XMVECTOR vRightTopFar = XMVectorMultiply( vRightTop, vFar );
    XMVECTOR vLeftBottomNear = XMVectorMultiply( vLeftBottom, vNear );
    XMVECTOR vLeftBottomFar = XMVectorMultiply( vLeftBottom, vFar );

    pvCornerPointsWorld[0] = vRightTopNear;
    pvCornerPointsWorld[1] = XMVectorSelect( vRightTopNear, vLeftBottomNear, vGrabX );
    pvCornerPointsWorld[2] = vLeftBottomNear;
    pvCornerPointsWorld[3] = XMVectorSelect( vRightTopNear, vLeftBottomNear,vGrabY );

    pvCornerPointsWorld[4] = vRightTopFar;
    pvCornerPointsWorld[5] = XMVectorSelect( vRightTopFar, vLeftBottomFar, vGrabX );
    pvCornerPointsWorld[6] = vLeftBottomFar;
    pvCornerPointsWorld[7] = XMVectorSelect( vRightTopFar ,vLeftBottomFar, vGrabY );

}

HRESULT CascadedDirectionalLightRenderer::renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
	UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Set up the render targets for the shadow map and clear them
	pd3dImmediateContext->OMSetRenderTargets(0, NULL, _shadowMapDSVs[shadowMapIdx]);
	pd3dImmediateContext->ClearDepthStencilView(_shadowMapDSVs[shadowMapIdx], D3D11_CLEAR_DEPTH, 1.0f, 0);
		
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->VSSetShader(_depthVS, NULL, 0);
	pd3dImmediateContext->PSSetShader(NULL, NULL, 0);

	pd3dImmediateContext->IASetInputLayout(_depthInput);
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthWriteEnabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());

	// Store this for later
	XMFLOAT4X4 fView = camera->GetView();
	XMMATRIX cameraView = XMLoadFloat4x4(&fView);

	XMFLOAT4X4 fViewProj = camera->GetViewProjection();
	XMMATRIX cameraViewProj = XMLoadFloat4x4(&fViewProj);

	XMFLOAT4X4 fProj = camera->GetProjection();
	XMMATRIX cameraProj = XMLoadFloat4x4(&fProj);

	// Compute the inverse of the camera's view
	XMVECTOR det;
	XMMATRIX inverseCameraView = XMMatrixInverse(&det, cameraView);	

	// Calculate the scene aabb in light space
	XMVECTOR lightDir = XMLoadFloat3(&dlight->Direction);
	XMFLOAT3 lightOrigin = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMMATRIX mLightCameraView = XMMatrixLookAtLH(XMLoadFloat3(&lightOrigin), -lightDir, up);

	XMVECTOR vSceneCenter = XMLoadFloat3(&sceneBounds->Center);
	XMVECTOR vSceneExtents = XMLoadFloat3(&sceneBounds->Extents);

	XMVECTOR vSceneAABBPointsLightSpace[8];
	CreateAABBPoints(vSceneAABBPointsLightSpace, vSceneCenter, vSceneExtents);
	for(int i = 0; i < 8; i++) 
	{
		vSceneAABBPointsLightSpace[i] = XMVector4Transform(vSceneAABBPointsLightSpace[i], mLightCameraView); 
	}

	FLOAT fFrustumIntervalBegin, fFrustumIntervalEnd;
	XMVECTOR vLightCameraOrthographicMin;  // light space frustrum aabb 
	XMVECTOR vLightCameraOrthographicMax;
	FLOAT fCameraNearFarRange = camera->GetFarClip() - camera->GetNearClip();

	XMVECTOR vWorldUnitsPerTexel = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	// Cascade offsets
    const XMFLOAT2 offsets[4] = {
        XMFLOAT2(0.0f, 0.0f),
        XMFLOAT2(0.5f, 0.0f),
        XMFLOAT2(0.5f, 0.5f),
        XMFLOAT2(0.0f, 0.5f)
    };

	int numRows = (int)sqrtf((float)NUM_CASCADES);
    float cascadeSize = (float)SHADOW_MAP_SIZE / numRows;

	for (UINT i = 0; i < NUM_CASCADES; i++)
	{
		// Create the viewport
		D3D11_VIEWPORT vp;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = cascadeSize;
		vp.Height = cascadeSize;
		vp.TopLeftX = offsets[i].x * cascadeSize * 2.0f;
        vp.TopLeftY = offsets[i].y * cascadeSize * 2.0f;

		pd3dImmediateContext->RSSetViewports(1, &vp);

		// calc the split depths
        float splitDist = CASCADE_SPLITS[i];

		fFrustumIntervalBegin = 0.0f;
		fFrustumIntervalEnd = splitDist * fCameraNearFarRange;
		
		XMVECTOR vFrustumPoints[8];
		CreateFrustumPointsFromCascadeInterval(fFrustumIntervalBegin, fFrustumIntervalEnd, cameraProj,
			vFrustumPoints);

		vLightCameraOrthographicMin = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
        vLightCameraOrthographicMax = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

		XMVECTOR vTempTranslatedCornerPoint;
        // This next section of code calculates the min and max values for the orthographic projection.
        for(int icpIndex = 0; icpIndex < 8; icpIndex++) 
        {
            // Transform the frustum from camera view space to world space.
            vFrustumPoints[icpIndex] = XMVector4Transform(vFrustumPoints[icpIndex], inverseCameraView);
            // Transform the point from world space to Light Camera Space.
            vTempTranslatedCornerPoint = XMVector4Transform(vFrustumPoints[icpIndex], mLightCameraView);
            // Find the closest point.
            vLightCameraOrthographicMin = XMVectorMin(vTempTranslatedCornerPoint, vLightCameraOrthographicMin);
            vLightCameraOrthographicMax = XMVectorMax(vTempTranslatedCornerPoint, vLightCameraOrthographicMax);
        }

		// This code removes the shimmering effect along the edges of shadows due to
        // the light changing to fit the camera.
        // Fit the ortho projection to the cascades far plane and a near plane of zero. 
        // Pad the projection to be the size of the diagonal of the Frustum partition. 
        // 
        // To do this, we pad the ortho transform so that it is always big enough to cover 
        // the entire camera view frustum.
        XMVECTOR vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
        vDiagonal = XMVector3Length(vDiagonal);
            
        // The bound is the length of the diagonal of the frustum interval.
        FLOAT fCascadeBound = XMVectorGetX(vDiagonal);
            
        // The offset calculated will pad the ortho projection so that it is always the same size 
        // and big enough to cover the entire cascade interval.
        XMVECTOR vBoarderOffset = (vDiagonal - (vLightCameraOrthographicMax - vLightCameraOrthographicMin)) * 0.5f;
        // Set the Z and W components to zero.
		vBoarderOffset = XMVectorSetZ(vBoarderOffset, 0.0f);
        vBoarderOffset = XMVectorSetW(vBoarderOffset, 0.0f);
            
        // Add the offsets to the projection.
        vLightCameraOrthographicMax += vBoarderOffset;
        vLightCameraOrthographicMin -= vBoarderOffset;

        // The world units per texel are used to snap the shadow the orthographic projection
        // to texel sized increments.  This keeps the edges of the shadows from shimmering.
        FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)cascadeSize;
        vWorldUnitsPerTexel = XMVectorSet(fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f);

		//float fLightCameraOrthographicMinZ = XMVectorGetZ(vLightCameraOrthographicMin);

		// We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
        // This is a matter of integer dividing by the world space size of a texel
        vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
        vLightCameraOrthographicMin = XMVectorFloor(vLightCameraOrthographicMin);
        vLightCameraOrthographicMin *= vWorldUnitsPerTexel;
            
        vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
        vLightCameraOrthographicMax = XMVectorFloor(vLightCameraOrthographicMax);
        vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

		// Determine the near and far planes
		FLOAT fNearPlane = 0.0f;
        FLOAT fFarPlane = 10000.0f;
				
		ComputeNearAndFar( fNearPlane, fFarPlane, vLightCameraOrthographicMin, 
                vLightCameraOrthographicMax, vSceneAABBPointsLightSpace);
		

		// Craete the orthographic projection for this cascade.
		XMMATRIX shadowProj = XMMatrixOrthographicOffCenterLH(
			XMVectorGetX(vLightCameraOrthographicMin), XMVectorGetX(vLightCameraOrthographicMax), 
            XMVectorGetY(vLightCameraOrthographicMin), XMVectorGetY(vLightCameraOrthographicMax), 
            fNearPlane, fFarPlane);
		
		XMMATRIX shadowViewProj = XMMatrixMultiply(mLightCameraView, shadowProj);

		// Create the shadow frustum for intersection tests
		Frustum shadowFrust;
		Collision::ComputeFrustumFromProjection(&shadowFrust, &shadowProj);
		shadowFrust.Origin = lightOrigin;
		XMStoreFloat4(&shadowFrust.Orientation, XMQuaternionNormalize(-lightDir));
		
		// Render the depth of all the models in the scene
		for (UINT j = 0; j < models->size(); j++)
		{
			ModelInstance* instance = models->at(j);
			Model* model = instance->GetModel();

			// Preform a collision check
			OrientedBox modelBounds = instance->GetOrientedBox();

			int modelIntersect = Collision::IntersectOrientedBoxFrustum(&modelBounds, &shadowFrust);
			if (!modelIntersect)
			{
				//continue;
			}

			// Prepare the buffer
			XMFLOAT4X4 fWorld = instance->GetWorld();
			XMMATRIX wvp = XMMatrixMultiply(XMLoadFloat4x4(&fWorld), shadowViewProj);
		
			V_RETURN(pd3dImmediateContext->Map(_depthPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES* modelProperties = (CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES*)mappedResource.pData;
			XMStoreFloat4x4(&modelProperties->WorldViewProjection, XMMatrixTranspose(wvp));
			pd3dImmediateContext->Unmap(_depthPropertiesBuffer, 0);

			pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_depthPropertiesBuffer);

			// Render the mesh parts
			for (UINT k = 0; k < model->GetMeshCount(); k++)
			{
				// If the main box is completely within the frust, we can skip the mesh check
				if (modelIntersect != COMPLETELY_INSIDE)
				{
					Mesh mesh = model->GetMesh(k);
					OrientedBox meshBounds = instance->GetMeshOrientedBox(k);
								
					if (!Collision::IntersectOrientedBoxFrustum(&meshBounds, &shadowFrust))
					{
						//continue;
					}
				}

				model->RenderMesh(pd3dImmediateContext, k);
			}
		}

		// Bake the cascade offset and bias into the projection matrix and then store it
		XMMATRIX texScaleBias;
        texScaleBias.r[0] = XMVectorSet(0.5f,  0.0f, 0.0f, 0.0f);
        texScaleBias.r[1] = XMVectorSet(0.0f, -0.5f, 0.0f, 0.0f);
        texScaleBias.r[2] = XMVectorSet(0.0f,  0.0f, 1.0f, 0.0f);
        texScaleBias.r[3] = XMVectorSet(0.5f,  0.5f, -BIAS, 1.0f);
        shadowViewProj = XMMatrixMultiply(shadowViewProj, texScaleBias);
		
		// Calculate the offset/scale matrix, which applies the offset and scale needed to
        // convert the UV coordinate into the proper coordinate for the cascade being sampled in
        // the atlas.
        XMFLOAT4 offset = XMFLOAT4(offsets[i].x, offsets[i].y, 0.0f, 1.0);
        XMMATRIX cascadeOffsetMatrix = XMMatrixScaling(0.5f, 0.5f, 1.0f);
        cascadeOffsetMatrix.r[3] = XMLoadFloat4(&offset);

		XMStoreFloat4x4(&_shadowMatricies[shadowMapIdx][i], XMMatrixTranspose(shadowViewProj));
		XMStoreFloat4x4(&_shadowTexCoordTransforms[shadowMapIdx][i], XMMatrixTranspose(cascadeOffsetMatrix));
		_cascadeSplits[shadowMapIdx][i] = fFrustumIntervalEnd;
	}

	return S_OK;
}

HRESULT CascadedDirectionalLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{	
	if (GetCount(true) + GetCount(false) > 0)
	{
		BEGIN_EVENT_D3D(L"Directional Lights");

		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
	
		// prepare the camera properties buffer
		XMFLOAT4X4 fCamViewProj = camera->GetViewProjection();

		XMVECTOR det;
		XMMATRIX cameraInvViewProj = XMMatrixInverse(&det, XMLoadFloat4x4(&fCamViewProj));

		// Set the global properties for all directional lights
		ID3D11SamplerState* samplers[2] =
		{
			GetSamplerStates()->GetPointClamp(),
			GetSamplerStates()->GetShadowMap(),
		};
		pd3dImmediateContext->PSSetSamplers(0, 2, samplers);
		pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetReverseDepthEnabled(), 0);	

		float blendFactor[4] = {1, 1, 1, 1};
		pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);

		V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));

		// map the camera properties
		V_RETURN(pd3dImmediateContext->Map(_cameraPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES* cameraProperties = (CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES*)mappedResource.pData;
	
		XMStoreFloat4x4(&cameraProperties->InverseViewProjection, XMMatrixTranspose(cameraInvViewProj));
		cameraProperties->CameraPosition = camera->GetPosition();

		pd3dImmediateContext->Unmap(_cameraPropertiesBuffer, 0);

		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_cameraPropertiesBuffer);

		// Begin rendering unshadowed lights
		int numUnshadowed = GetCount(false);
		for (int i = 0; i < numUnshadowed; i++)
		{
			DirectionalLight* light = GetLight(i, false);

			V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES* lightProperties = 
				(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

			lightProperties->LightColor = light->Color;
			lightProperties->LightDirection = light->Direction;

			pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);

			pd3dImmediateContext->PSSetConstantBuffers(1, 1, &_lightPropertiesBuffer);

			_fsQuad.Render(pd3dImmediateContext, _unshadowedPS);
		}
	
		// begin rendering shadowed lights
		int numShadowed = GetCount(true);
		for (int i = 0; i < numShadowed; i++)
		{
			DirectionalLight* light = GetLight(i, true);

			// Prepare the light properties
			V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES* lightProperties = 
				(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

			lightProperties->LightColor = light->Color;
			lightProperties->LightDirection = light->Direction;

			pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);
		
			// Prepare the shadow properties
			V_RETURN(pd3dImmediateContext->Map(_shadowPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES* shadowProperties = 
				(CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES*)mappedResource.pData;

			for (UINT j = 0; j < NUM_CASCADES; j++)
			{
				// Matricies are already transposed
				shadowProperties->CascadeSplits[j] = _cascadeSplits[i][j];
				shadowProperties->ShadowMatricies[j] = _shadowMatricies[i][j];
				shadowProperties->ShadowTexCoordTransforms[j] = _shadowTexCoordTransforms[i][j];
			}
			shadowProperties->CameraClips = XMFLOAT2(camera->GetNearClip(), camera->GetFarClip());
			shadowProperties->ShadowMapSize = XMFLOAT2((float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE);

			pd3dImmediateContext->Unmap(_shadowPropertiesBuffer, 0);

			// Set both constant buffers back to the shader at once
			ID3D11Buffer* constantBuffers[2] = { _lightPropertiesBuffer, _shadowPropertiesBuffer };
			pd3dImmediateContext->PSSetConstantBuffers(1, 2, constantBuffers);

			// Set the shadow map SRV
			pd3dImmediateContext->PSSetShaderResources(4, 1, &_shadowMapSRVs[i]);

			// Finally, render the quad
			_fsQuad.Render(pd3dImmediateContext, _shadowedPS);
		}

		// Null all the SRVs
		ID3D11ShaderResourceView* nullSRV[5] = { NULL, NULL, NULL, NULL, NULL };
		pd3dImmediateContext->PSSetShaderResources(0, 5, nullSRV);

		END_EVENT_D3D(L"");
	}

	return S_OK;
}

HRESULT CascadedDirectionalLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Compile both shaders from file
	ID3DBlob* pBlob = NULL;
	
	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightUnshadowed", "ps_4_0", NULL, &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_unshadowedPS));
	SAFE_RELEASE(pBlob);

	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightShadowed", "ps_4_0", NULL, &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_shadowedPS));
	SAFE_RELEASE(pBlob);
	
	V_RETURN(CompileShaderFromFile(L"Depth.hlsl", "VS_Depth", "vs_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_depthVS));

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN( pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_depthInput));
	SAFE_RELEASE(pBlob);

	// Create the buffers
	D3D11_BUFFER_DESC bufferDesc =
	{
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_depthPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_cameraPropertiesBuffer));
	
	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_lightPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_shadowPropertiesBuffer));

	// Create the shadow textures
	D3D11_TEXTURE2D_DESC shadowMapTextureDesc = 
	{
		SHADOW_MAP_SIZE,//UINT Width;
		SHADOW_MAP_SIZE,//UINT Height;
		1,//UINT MipLevels;
		1,//UINT ArraySize;
		DXGI_FORMAT_R32_TYPELESS,//DXGI_FORMAT Format;
		1,//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};
	
	// Create the shadow map SRVs
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSRVDesc = 
	{
		DXGI_FORMAT_R32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	shadowMapSRVDesc.Texture2D.MipLevels = 1;
	
	// create the shadow map depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDSVDesc =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		V_RETURN(pd3dDevice->CreateTexture2D(&shadowMapTextureDesc, NULL, &_shadowMapTextures[i]));
		V_RETURN(pd3dDevice->CreateShaderResourceView(_shadowMapTextures[i], &shadowMapSRVDesc, &_shadowMapSRVs[i]));
		V_RETURN(pd3dDevice->CreateDepthStencilView(_shadowMapTextures[i], &shadowMapDSVDesc, &_shadowMapDSVs[i]));
	}

	// Load the other IHasContents
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void CascadedDirectionalLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	SAFE_RELEASE(_depthVS);
	SAFE_RELEASE(_depthInput);
	SAFE_RELEASE(_depthPropertiesBuffer);

	SAFE_RELEASE(_unshadowedPS);
	SAFE_RELEASE(_shadowedPS);
	SAFE_RELEASE(_lightPropertiesBuffer);
	SAFE_RELEASE(_cameraPropertiesBuffer);
	SAFE_RELEASE(_shadowPropertiesBuffer);

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		SAFE_RELEASE(_shadowMapTextures[i]);
		SAFE_RELEASE(_shadowMapSRVs[i]);
		SAFE_RELEASE(_shadowMapDSVs[i]);
	}

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT CascadedDirectionalLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void CascadedDirectionalLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();
	_fsQuad.OnD3D11ReleasingSwapChain();
}
