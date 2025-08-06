/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/StripLayer.h>

namespace MuonGMR4{

    std::ostream& operator<<(std::ostream& ostr, const StripLayer& lay) {
        ostr<<"Strip layer transform: "<<Amg::toString(lay.toOrigin())<<", ";
        ostr<<lay.design()<<", ";
        ostr<<"Hash: "<<static_cast<unsigned int>(lay.hash());        
        return ostr;
    }
    void StripLayer::flipPhiRotation() {
        m_phiRot = -m_phiRot;
    }
    StripLayer::StripLayer(TransformPtr layerTransform,
                           StripDesignPtr design,
                           const IdentifierHash hash):
        StripLayer{std::move(layerTransform), design, design, hash} {}
    
    StripLayer::StripLayer(TransformPtr layerTransform,
                           StripDesignPtr etaDesign, StripDesignPtr phiDesign,
                           const IdentifierHash hash):
         m_transform{std::move(layerTransform)},
         m_etaDesign{std::move(etaDesign)},
         m_phiDesign{std::move(phiDesign)},
         m_hash{hash} {       
    }
    bool StripLayer::operator<(const StripLayer& other) const{
        if (hash() != other.hash()) {
            return hash() < other.hash();
        }
        if (m_transform != other.m_transform) {
            return m_transform < other.m_transform;
        }
        if (hasPhiDesign() != hasPhiDesign()){
            return hasPhiDesign();
        }
        if (m_phiDesign != other.m_phiDesign) {
            return m_phiDesign < other.m_phiDesign;
        }
        return m_etaDesign < other.m_etaDesign;
    }
}