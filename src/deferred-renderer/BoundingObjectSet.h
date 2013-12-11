#pragma once

#include "PCH.h"
#include "xnaCollision.h"

class BoundingObjectSet
{
private:
    std::vector<Sphere> _spheres;
    std::vector<AxisAlignedBox> _aabbs;
    std::vector<OrientedBox> _obbs;
    std::vector<Frustum> _frustums;

public:
    BoundingObjectSet();

    UINT GetSphereCount() const { return _spheres.size(); }
    Sphere* GetSphere(UINT i) { return (i < _spheres.size()) ? &_spheres[i] : NULL; }
    void AddSphere(const Sphere& sphere) { _spheres.push_back(sphere); }

    UINT GetAxisAlignedBoxeCount() const { return _aabbs.size(); }
    AxisAlignedBox* GetAxisAlignedBox(UINT i) { return (i < _aabbs.size()) ? &_aabbs[i] : NULL; }
    void AddAxisAlignedBox(const AxisAlignedBox& aabb) { _aabbs.push_back(aabb); }

    UINT GetOrientedBoxCount() const { return _obbs.size(); }
    OrientedBox* GetOrientedBox(UINT i) { return (i < _obbs.size()) ? &_obbs[i] : NULL; }
    void AddOrientedBox(const OrientedBox& obb) { _obbs.push_back(obb); }

    UINT GetFrustumCount() const { return _frustums.size(); }
    Frustum* GetFrustum(UINT i) { return (i < _frustums.size()) ? &_frustums[i] : NULL; }
    void AddFrustum(const Frustum& frust) { _frustums.push_back(frust); }

    void Merge(BoundingObjectSet* set);

    void Clear() { _spheres.clear(); _aabbs.clear(); _obbs.clear(); _frustums.clear(); }
};