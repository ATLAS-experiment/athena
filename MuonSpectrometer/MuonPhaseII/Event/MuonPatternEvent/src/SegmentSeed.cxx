/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternEvent/SegmentSeed.h"
using namespace Acts;
namespace MuonR4{
    using namespace SegmentFit;
     SegmentSeed::SegmentSeed(double tanTheta, double interceptY, double tanPhi,
                              double interceptX, double counts,
                              std::vector<HitType>&& hits,
                              const SpacePointBucket* bucket):
        m_parent{bucket},
        m_hits{std::move(hits)},
        m_hasPhiExt{true},
        m_counts{counts}{
            const Amg::Vector3D dir = dirFromTangents(tanPhi, tanTheta);
            m_pars[toUnderlying(ParamDefs::x0)] = interceptX;
            m_pars[toUnderlying(ParamDefs::y0)] = interceptY;
            m_pars[toUnderlying(ParamDefs::theta)] = dir.theta();
            m_pars[toUnderlying(ParamDefs::phi)] = dir.phi();
    }
    SegmentSeed::SegmentSeed(const HoughMaximum& toCopy):
        m_parent{toCopy.parentBucket()},
        m_hits{toCopy.getHitsInMax()},
        m_counts{toCopy.getCounts()}{
            const Amg::Vector3D dir = dirFromTangents(0., toCopy.tanTheta());
            m_pars[toUnderlying(ParamDefs::y0)]    = toCopy.interceptY();
            m_pars[toUnderlying(ParamDefs::theta)] = dir.theta();
            m_pars[toUnderlying(ParamDefs::phi)] = dir.phi();
    }
    double SegmentSeed::tanPhi() const {  return houghTanPhi(localDirection()); }
    double SegmentSeed::tanTheta() const { return houghTanTheta(localDirection()); }
    double SegmentSeed::interceptX() const { return m_pars[toUnderlying(ParamDefs::x0)]; }
    double SegmentSeed::interceptY() const {return m_pars[toUnderlying(ParamDefs::y0)]; }
    const Parameters& SegmentSeed::parameters() const { return m_pars; }
    double SegmentSeed::getCounts() const{ return m_counts;}
    const std::vector<SegmentSeed::HitType>& SegmentSeed::getHitsInMax() const { return m_hits; }
    const SpacePointBucket* SegmentSeed::parentBucket() const{ return m_parent; }
    const MuonGMR4::SpectrometerSector* SegmentSeed::msSector() const{ return m_parent->msSector(); }
    bool SegmentSeed::hasPhiExtension() const{ return m_hasPhiExt; }
    Amg::Vector3D SegmentSeed::localPosition() const{ return Amg::Vector3D{interceptX(), interceptY(),0.}; }
    Amg::Vector3D SegmentSeed::localDirection() const { 
        return Amg::dirFromAngles(m_pars[toUnderlying(ParamDefs::phi)], m_pars[toUnderlying(ParamDefs::theta)]);
    }
}