//-------------------------------------------------------------------------------------
// XNACollision.h
//
// An opimtized Collision library based on XNAMath
//
// Microsoft XNA Developer Connection
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------

#pragma once

#include "PCH.h"

struct Sphere
{
    XMFLOAT3 Center;            // Center of the sphere.
    FLOAT Radius;               // Radius of the sphere.
};

struct AxisAlignedBox
{
    XMFLOAT3 Center;            // Center of the box.
    XMFLOAT3 Extents;           // Distance from the center to each side.
};

struct OrientedBox
{
    XMFLOAT3 Center;            // Center of the box.
    XMFLOAT3 Extents;           // Distance from the center to each side.
    XMFLOAT4 Orientation;       // Unit quaternion representing rotation (box -> world).
};

struct Frustum
{
    XMFLOAT3 Origin;            // Origin of the frustum (and projection).
    XMFLOAT4 Orientation;       // Unit quaternion representing rotation.

    FLOAT RightSlope;           // Positive X slope (X/Z).
    FLOAT LeftSlope;            // Negative X slope.
    FLOAT TopSlope;             // Positive Y slope (Y/Z).
    FLOAT BottomSlope;          // Negative Y slope.
    FLOAT Near, Far;            // Z of the near plane and far plane.
};

struct Ray
{
    XMFLOAT3 Origin;
    XMFLOAT3 Direction;
};

// Some xna collision defines
#define NO_INTERSECTION 0
#define INTERSECTION 1
#define COMPLETELY_INSIDE 2

class Collision
{
public:
    //-----------------------------------------------------------------------------
    // Bounding volume construction.
    //-----------------------------------------------------------------------------
    static VOID ComputeBoundingSphereFromPoints( Sphere* pOut, UINT Count, const XMFLOAT3* pPoints, UINT Stride );
    static VOID ComputeBoundingAxisAlignedBoxFromPoints( AxisAlignedBox* pOut, UINT Count, const XMFLOAT3* pPoints, UINT Stride );
    static VOID ComputeBoundingOrientedBoxFromPoints( OrientedBox* pOut, UINT Count, const XMFLOAT3* pPoints, UINT Stride );
    static VOID ComputeFrustumFromProjection( Frustum* pOut, XMMATRIX* pProjection );
    static VOID ComputePlanesFromFrustum( const Frustum* pVolume, XMVECTOR* pPlane0, XMVECTOR* pPlane1, XMVECTOR* pPlane2,
        XMVECTOR* pPlane3, XMVECTOR* pPlane4, XMVECTOR* pPlane5 );

    //-----------------------------------------------------------------------------
    // Corner calculation.
    //-----------------------------------------------------------------------------
    static VOID ComputeAxisAlignedBoxCorners( const AxisAlignedBox* pVolume, XMVECTOR* pCorner1, XMVECTOR* pCorner2,
        XMVECTOR* pCorner3, XMVECTOR* pCorner4, XMVECTOR* pCorner5,
        XMVECTOR* pCorner6,  XMVECTOR* pCorner7, XMVECTOR* pCorner8);
    static VOID ComputeFrustumCorners( const Frustum* pVolume, XMVECTOR* pCorner1, XMVECTOR* pCorner2,
        XMVECTOR* pCorner3, XMVECTOR* pCorner4, XMVECTOR* pCorner5,
        XMVECTOR* pCorner6,  XMVECTOR* pCorner7, XMVECTOR* pCorner8);

    //-----------------------------------------------------------------------------
    // Bounding volume merging.
    //-----------------------------------------------------------------------------
    static VOID MergeAxisAlignedBoxes(AxisAlignedBox* pOut, AxisAlignedBox* pInA, AxisAlignedBox* pInB);

    //-----------------------------------------------------------------------------
    // Bounding volume transforms.
    //-----------------------------------------------------------------------------
    static VOID TransformSphere( Sphere* pOut, const Sphere* pIn, FLOAT Scale, FXMVECTOR Rotation, FXMVECTOR Translation );
    static VOID TransformAxisAlignedBox( AxisAlignedBox* pOut, const AxisAlignedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
        FXMVECTOR Translation );
    static VOID TransformOrientedBox( OrientedBox* pOut, const OrientedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
        FXMVECTOR Translation );
    static VOID TransformFrustum( Frustum* pOut, const Frustum* pIn, FLOAT Scale, FXMVECTOR Rotation, FXMVECTOR Translation );

    //-----------------------------------------------------------------------------
    // Intersection testing routines.
    //-----------------------------------------------------------------------------
    static BOOL IntersectPointSphere( FXMVECTOR Point, const Sphere* pVolume );
    static BOOL IntersectPointAxisAlignedBox( FXMVECTOR Point, const AxisAlignedBox* pVolume );
    static BOOL IntersectPointOrientedBox( FXMVECTOR Point, const OrientedBox* pVolume );
    static BOOL IntersectPointFrustum( FXMVECTOR Point, const Frustum* pVolume );
    static BOOL IntersectRayTriangle( FXMVECTOR Origin, FXMVECTOR Direction, FXMVECTOR V0, CXMVECTOR V1, CXMVECTOR V2,
        FLOAT* pDist );
    static BOOL IntersectRaySphere( FXMVECTOR Origin, FXMVECTOR Direction, const Sphere* pVolume, FLOAT* pDist );
    static BOOL IntersectRayAxisAlignedBox( FXMVECTOR Origin, FXMVECTOR Direction, const AxisAlignedBox* pVolume, FLOAT* pDist );
    static BOOL IntersectRayOrientedBox( FXMVECTOR Origin, FXMVECTOR Direction, const OrientedBox* pVolume, FLOAT* pDist );
    static BOOL IntersectTriangleTriangle( FXMVECTOR A0, FXMVECTOR A1, FXMVECTOR A2, CXMVECTOR B0, CXMVECTOR B1, CXMVECTOR B2 );
    static BOOL IntersectTriangleSphere( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, const Sphere* pVolume );
    static BOOL IntersectTriangleAxisAlignedBox( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, const AxisAlignedBox* pVolume );
    static BOOL IntersectTriangleOrientedBox( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, const OrientedBox* pVolume );
    static BOOL IntersectSphereSphere( const Sphere* pVolumeA, const Sphere* pVolumeB );
    static BOOL IntersectSphereAxisAlignedBox( const Sphere* pVolumeA, const AxisAlignedBox* pVolumeB );
    static BOOL IntersectSphereOrientedBox( const Sphere* pVolumeA, const OrientedBox* pVolumeB );
    static BOOL IntersectAxisAlignedBoxAxisAlignedBox( const AxisAlignedBox* pVolumeA, const AxisAlignedBox* pVolumeB );
    static BOOL IntersectAxisAlignedBoxOrientedBox( const AxisAlignedBox* pVolumeA, const OrientedBox* pVolumeB );
    static BOOL IntersectOrientedBoxOrientedBox( const OrientedBox* pVolumeA, const OrientedBox* pVolumeB );

    //-----------------------------------------------------------------------------
    // Frustum intersection testing routines.
    // Return values: 0 = no intersection,
    //                1 = intersection,
    //                2 = A is completely inside B
    //-----------------------------------------------------------------------------
    static INT IntersectTriangleFrustum( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, const Frustum* pVolume );
    static INT IntersectSphereFrustum( const Sphere* pVolumeA, const Frustum* pVolumeB );
    static INT IntersectAxisAlignedBoxFrustum( const AxisAlignedBox* pVolumeA, const Frustum* pVolumeB );
    static INT IntersectOrientedBoxFrustum( const OrientedBox* pVolumeA, const Frustum* pVolumeB );
    static INT IntersectFrustumFrustum( const Frustum* pVolumeA, const Frustum* pVolumeB );

    //-----------------------------------------------------------------------------
    // Test vs six planes (usually forming a frustum) intersection routines.
    // The intended use for these routines is for fast culling to a view frustum.
    // When the volume being tested against a view frustum is small relative to the
    // view frustum it is usually either inside all six planes of the frustum or
    // outside one of the planes of the frustum. If neither of these cases is true
    // then it may or may not be intersecting the frustum. Outside a plane is
    // defined as being on the positive side of the plane (and inside negative).
    // Return values: 0 = volume is outside one of the planes (no intersection),
    //                1 = not completely inside or completely outside (intersecting),
    //                2 = volume is inside all the planes (completely inside)
    //-----------------------------------------------------------------------------
    static INT IntersectTriangle6Planes( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, CXMVECTOR Plane0, CXMVECTOR Plane1,
        CXMVECTOR Plane2, CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5 );
    static INT IntersectSphere6Planes( const Sphere* pVolume, FXMVECTOR Plane0, FXMVECTOR Plane1, FXMVECTOR Plane2,
        CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5 );
    static INT IntersectAxisAlignedBox6Planes( const AxisAlignedBox* pVolume, FXMVECTOR Plane0, FXMVECTOR Plane1,
        FXMVECTOR Plane2, CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5 );
    static INT IntersectOrientedBox6Planes( const OrientedBox* pVolume, FXMVECTOR Plane0, FXMVECTOR Plane1, FXMVECTOR Plane2,
        CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5 );
    static INT IntersectFrustum6Planes( const Frustum* pVolume, FXMVECTOR Plane0, FXMVECTOR Plane1, FXMVECTOR Plane2,
        CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5 );

    //-----------------------------------------------------------------------------
    // Volume vs plane intersection testing routines.
    // Return values: 0 = volume is outside the plane (on the positive sideof the plane),
    //                1 = volume intersects the plane,
    //                2 = volume is inside the plane (on the negative side of the plane)
    //-----------------------------------------------------------------------------
    static INT IntersectTrianglePlane( FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, CXMVECTOR Plane );
    static INT IntersectSpherePlane( const Sphere* pVolume, FXMVECTOR Plane );
    static INT IntersectAxisAlignedBoxPlane( const AxisAlignedBox* pVolume, FXMVECTOR Plane );
    static INT IntersectOrientedBoxPlane( const OrientedBox* pVolume, FXMVECTOR Plane );
    static INT IntersectFrustumPlane( const Frustum* pVolume, FXMVECTOR Plane );
};