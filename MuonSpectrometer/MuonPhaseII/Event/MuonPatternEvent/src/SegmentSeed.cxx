/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternEvent/SegmentSeed.h"

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
            m_pars[toInt(AxisDefs::x0)] = interceptX;
            m_pars[toInt(AxisDefs::y0)] = interceptY;
            m_pars[toInt(AxisDefs::tanTheta)] = tanTheta;
            m_pars[toInt(AxisDefs::tanPhi)] = tanPhi;
        }
    SegmentSeed::SegmentSeed(const HoughMaximum& toCopy):
        m_parent{toCopy.parentBucket()},
        m_hits{toCopy.getHitsInMax()},
        m_counts{toCopy.getCounts()}{
            m_pars[toInt(AxisDefs::y0)] = toCopy.interceptY();
            m_pars[toInt(AxisDefs::tanTheta)] = toCopy.tanTheta();
        }
    double SegmentSeed::tanPhi() const{ return m_pars[toInt(AxisDefs::tanPhi)]; }
    double SegmentSeed::interceptX() const { return m_pars[toInt(AxisDefs::x0)]; }
    double SegmentSeed::tanTheta() const { return m_pars[toInt(AxisDefs::tanTheta)]; }
    double SegmentSeed::interceptY() const {return m_pars[toInt(AxisDefs::y0)]; }
    const Parameters& SegmentSeed::parameters() const{ return m_pars;}
    double SegmentSeed::getCounts() const{ return m_counts;}
    const std::vector<SegmentSeed::HitType>& SegmentSeed::getHitsInMax() const { return m_hits; }
    const SpacePointBucket* SegmentSeed::parentBucket() const{ return m_parent; }
    const MuonGMR4::MuonChamber* SegmentSeed::chamber() const{ return m_parent->chamber(); }
    bool SegmentSeed::hasPhiExtension() const{ return m_hasPhiExt; }
    Amg::Vector3D SegmentSeed::positionInChamber() const{ return Amg::Vector3D{interceptX(), interceptY(),0.}; }
    Amg::Vector3D SegmentSeed::directionInChamber() const{ return Amg::Vector3D(tanPhi(),tanTheta(),1.).unit();}
}