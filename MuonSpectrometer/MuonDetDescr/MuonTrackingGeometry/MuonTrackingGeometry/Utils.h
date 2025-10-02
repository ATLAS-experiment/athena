/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKINGGEOMETRY_TRANSFORMUTILS_H
#define MUONTRACKINGGEOMETRY_TRANSFORMUTILS_H

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <vector>
#include <memory>

namespace Muon{

    inline std::unique_ptr<Amg::Transform3D> makeTransform(const Amg::Transform3D& trf) {
        return std::make_unique<Amg::Transform3D>(trf);
    }
    template <class ObjType>
        std::vector<ObjType *> release(std::vector<std::unique_ptr<ObjType>>& objVec) {
        std::vector<ObjType*> outVec{};
        outVec.reserve(objVec.size());
        for (std::unique_ptr<ObjType>& obj : objVec) {
            outVec.push_back(obj.release());
        }
        objVec.clear();
        return outVec;
    }
    template<class ObjPtr>
    std::vector<std::vector<ObjPtr*>>
        release(std::vector<std::vector<std::unique_ptr<ObjPtr>>>& inVec){
        std::vector<std::vector<ObjPtr*>> outVec{};
        outVec.reserve(inVec.size());
        for (auto& in : inVec){
            outVec.emplace_back(release(in));
        }
        return outVec;
    }
    template <class ObjType>
        std::vector<std::unique_ptr<ObjType>> toVec(const std::vector<ObjType*>* vecPtr) {
        std::vector<std::unique_ptr<ObjType>> outVec{};
        outVec.reserve(vecPtr->size());
        for (auto obj : *vecPtr) {
            outVec.emplace_back(obj);
        }
        delete vecPtr;
        return outVec;
    }
}
#endif
