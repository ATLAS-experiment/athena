/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternEvent/SegmentSeed.h"

#include "Acts/Utilities/UnitVectors.hpp"

using namespace Acts;
namespace MuonR4{
     SegmentSeed::SegmentSeed(double tanBeta, double interceptY, double tanAlpha,
                              double interceptX, double counts,
                              std::vector<HitType>&& hits,
                              const SpacePointBucket* bucket):
        m_parent{bucket},
        m_hits{std::move(hits)},
        m_hasPhiExt{true},
        m_counts{counts}{
            const Amg::Vector3D dir = makeDirectionFromAxisTangents(tanAlpha, tanBeta);
            using enum SegmentFit::ParamDefs;
            m_pars[toUnderlying(x0)] = interceptX;
            m_pars[toUnderlying(y0)] = interceptY;
            m_pars[toUnderlying(theta)] = dir.theta();
            m_pars[toUnderlying(phi)] = dir.phi();
    }
    SegmentSeed::SegmentSeed(const HoughMaximum& toCopy):
        m_parent{toCopy.parentBucket()},
        m_hits{toCopy.getHitsInMax()},
        m_counts{toCopy.getCounts()}{
            const Amg::Vector3D dir = makeDirectionFromAxisTangents(0., toCopy.tanBeta());
            using enum SegmentFit::ParamDefs;
            m_pars[toUnderlying(y0)]    = toCopy.interceptY();
            m_pars[toUnderlying(theta)] = dir.theta();
            m_pars[toUnderlying(phi)] = dir.phi();
    }
    double SegmentSeed::tanAlpha() const {  return houghTanAlpha(localDirection()); }
    double SegmentSeed::tanBeta() const { return houghTanBeta(localDirection()); }
    double SegmentSeed::interceptX() const { 
        using enum SegmentFit::ParamDefs;
        return m_pars[toUnderlying(x0)]; 
    }
    double SegmentSeed::interceptY() const {
        using enum SegmentFit::ParamDefs;
        return m_pars[toUnderlying(y0)]; 
    }
    const SegmentFit::Parameters& SegmentSeed::parameters() const { return m_pars; }
    double SegmentSeed::getCounts() const{ return m_counts;}
    const std::vector<SegmentSeed::HitType>& SegmentSeed::getHitsInMax() const { return m_hits; }
    const SpacePointBucket* SegmentSeed::parentBucket() const{ return m_parent; }
    const MuonGMR4::SpectrometerSector* SegmentSeed::msSector() const{ return m_parent->msSector(); }
    bool SegmentSeed::hasPhiExtension() const{ return m_hasPhiExt; }
    Amg::Vector3D SegmentSeed::localPosition() const{ return Amg::Vector3D{interceptX(), interceptY(),0.}; }
    Amg::Vector3D SegmentSeed::localDirection() const { 
        using enum SegmentFit::ParamDefs;;
        return  makeDirectionFromPhiTheta(m_pars[toUnderlying(phi)], m_pars[toUnderlying(theta)]);
    }
}