/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/TgcReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <cassert>
#include <cstdlib>

#include <cmath>
#include <ostream>

#include "EventPrimitives/EventPrimitivesToStringConverter.h"

#include "CxxUtils/StringUtils.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

#define THROW_EXCEPT(MESSAGE)                                    \
      {                                                          \
         std::stringstream except_sstr{};                        \
         except_sstr<<__FILE__<<":"<<__LINE__<<" (";             \
         except_sstr<<idHelperSvc()->toString(identify())<<") "; \
         except_sstr<<MESSAGE;                                   \
         throw std::runtime_error(except_sstr.str());            \
      }

namespace MuonGM {

    TgcReadoutElement::TgcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, MuonDetectorManager* mgr) :
        MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::Tgc) {
        setStationName(stName);
        // get the setting of the caching flag from the manager
    }
    void TgcReadoutElement::setPlaneZ(double value, int gasGap) { m_gasPlaneZ[gasGap - 1] = value; }
    void TgcReadoutElement::setReadOutName(const std::string& rName) { m_readout_name = rName; }
    void TgcReadoutElement::setReadOutParams(GeoModel::TransientConstSharedPtr <TgcReadoutParams> pars) { 
        m_readoutParams = std::move(pars); 
    }
    void TgcReadoutElement::setFrameThickness(const double frameH, const double frameAB) {
        m_frameH = frameH;
        m_frameAB = frameAB;
    }
    Amg::Vector3D TgcReadoutElement::localGasGapPos(int gg) const { 
        return m_gasPlaneZ[gg - 1] * Amg::Vector3D::UnitX(); 
    }

    double TgcReadoutElement::length() const { return getRsize() - 2. * frameZwidth(); }
    double TgcReadoutElement::frameZwidth() const { return m_frameH; }
    double TgcReadoutElement::frameXwidth() const { return m_frameAB; }
    double TgcReadoutElement::wireGangBottomX(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif
        return wireGangLocalX(gasGap, gang) - 0.5* gangRadialLength(gasGap, gang);
    }

    double TgcReadoutElement::wireGangTopX(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif
        return wireGangLocalX(gasGap, gang) + 0.5 * gangRadialLength(gasGap, gang);
    }

    // Access to wire gang dimensions (simple trapezoid with length along z)

    double TgcReadoutElement::gangRadialLength(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif
        return wirePitch() * nWires(gasGap, gang);
    }

    double TgcReadoutElement::gangShortWidth(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif        
        return chamberWidth(wireGangBottomX(gasGap, gang)) - frameXwidth() * 2.;
    }

    double TgcReadoutElement::gangLongWidth(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif
        return chamberWidth(wireGangTopX(gasGap, gang)) - frameXwidth() * 2.;
    }
    double TgcReadoutElement::gangCentralWidth(int gasGap, int gang) const {
#ifndef NDEBUG
        if (!validGang(gasGap, gang)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire gang "<<gang<<" given.");
#endif
        return chamberWidth(wireGangLocalX(gasGap, gang)) - frameXwidth() * 2.;
    }

    double TgcReadoutElement::stripDeltaPhi() const {
        // number of strips in exclusive phi coverage of a chamber in T[1-3] and T4
        constexpr std::array<double, 4> nDivInChamberPhi{29.5, 29.5, 29.5, 31.5};
        double dphi{0.};

        int iStation = CxxUtils::atoi(&getStationType()[1]);
        if (iStation != 4 || getStationType()[2] != 'E') {  // except for station T4E
            dphi = 360. * CLHEP::degree / (nPhiChambers()) / nDivInChamberPhi[iStation - 1];
        } else {  // T4E
            dphi = 360. * CLHEP::degree / 36. / nDivInChamberPhi[iStation - 1];
        }
        return dphi;
    }

    double TgcReadoutElement::stripDeltaPhi(int gasGap, int strip) const {
        double dphi = stripDeltaPhi();
        // half strip
        if ((strip >= 31 && ((getStationEta() > 0 && gasGap == 1) || (getStationEta() < 0 && gasGap != 1))) ||
            (strip <= 2 && ((getStationEta() > 0 && gasGap != 1) || (getStationEta() < 0 && gasGap == 1))))
            dphi = dphi / 2.;

        return dphi;
    }

    double TgcReadoutElement::stripCenterLocX(int gasGap, int strip, double z) const {
#ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & strip "<<strip<<" given.");
#endif
        const auto [flip, pickStrip] = stripNumberToFetch(gasGap, strip);
        return flip * stripLocalX(gasGap, pickStrip, z, m_readoutParams->stripCenter(pickStrip, gasGap)); 
    }
    double TgcReadoutElement::stripLowEdgeLocX(int gasGap, int strip, double z) const {
#ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & wire strip "<<strip<<" given.");
#endif
        const auto [flip, pickStrip] = stripNumberToFetch(gasGap, strip);
        ATH_MSG_VERBOSE(__func__<<"() "<<idHelperSvc()->toStringDetEl(identify())
                      <<", gasGap: "<<gasGap<<", strip: "<<strip<<", pickStrip: "<<pickStrip);
        return flip * stripLocalX(gasGap,  pickStrip, z, 0.5*(stripPosOnLargeBase(pickStrip, gasGap) + stripPosOnShortBase(pickStrip, gasGap)));
    }

    double TgcReadoutElement::stripHighEdgeLocX(int gasGap, int strip, double z) const {
#ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & strip "<<strip<<" given.");
#endif
        const int nextStrip = strip +1;
        auto [flip, pickStrip] = stripNumberToFetch(gasGap, nextStrip);
        pickStrip += (nextStrip != pickStrip);
        ATH_MSG_VERBOSE(__func__<<"() "<<idHelperSvc()->toStringDetEl(identify())
                      <<", gasGap: "<<gasGap<<", strip: "<<strip<<", pickStrip: "<<pickStrip);
        return flip * stripLocalX(gasGap, pickStrip, z, 0.5*(stripPosOnLargeBase(pickStrip, gasGap) + stripPosOnShortBase(pickStrip, gasGap)));
    }

    double TgcReadoutElement::stripWidth(int gasGap, int strip) const {
        //!< stripWidth depends on global R (almost pointing geometry+trapezoidal strips); returns the width at the center of the plane
        return std::abs(stripShortWidth(gasGap, strip) + stripLongWidth(gasGap, strip)) / 2.;
    }
    double TgcReadoutElement::stripShortWidth(int gasGap, int strip) const {
        // projection of strip on local X axis at min(Z)
#ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & strip "<<strip<<" given.");
#endif
        if ((getStationEta() > 0 && gasGap == 1) || (getStationEta() < 0 && gasGap != 1) || m_readoutParams->nStripLayers() > 1) {
            return stripPosOnShortBase(strip + 1, gasGap) - stripPosOnShortBase(strip, gasGap);
        } else {
            return -(stripPosOnShortBase(33 - strip, gasGap) - stripPosOnShortBase(33 - (strip - 1), gasGap));
        }
    }

    double TgcReadoutElement::stripLongWidth(int gasGap, int strip) const {
        // projection of strip on local X axis at max(Z)
#ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & strip "<<strip<<" given.");
#endif

        if ((getStationEta() > 0 && gasGap == 1) || (getStationEta() < 0 && gasGap != 1) || m_readoutParams->nStripLayers() > 1) {
            return stripPosOnLargeBase(strip + 1, gasGap) - stripPosOnLargeBase(strip, gasGap);
        } else {
            return -(stripPosOnLargeBase(33 - strip, gasGap) - stripPosOnLargeBase(33 - (strip - 1), gasGap));
        }
    }


    double TgcReadoutElement::stripPitch(int gasGap, int strip) const {
        return stripPitch(gasGap, strip, 0.);
    }

    double TgcReadoutElement::stripPitch(int gasGap, int strip, double z) const {
        //!< strip pitch depending on local z position (R in global coordinate)
 #ifndef NDEBUG
        if (!validStrip(gasGap, strip)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" & strip "<<strip<<" given.");
 #endif
        if (1 < strip && strip < 32) {
            double stripCurrCen = stripCenterLocX(gasGap, strip, z);
            double stripNextCen = stripCenterLocX(gasGap, strip + 1, z);
            double stripPrevCen = stripCenterLocX(gasGap, strip - 1, z);
            return std::max(stripNextCen - stripCurrCen , stripCurrCen - stripPrevCen);
        } else if (strip == 1) {
            return stripCenterLocX(gasGap, strip + 1, z) - stripCenterLocX(gasGap, strip, z);
        } else if (strip == 32) {
            return stripCenterLocX(gasGap, strip, z) - stripCenterLocX(gasGap, strip - 1, z);
        } 
        return 0.;
    }

    // Monte Carlo debug (returns strip or gang corresponding with the position)
    int TgcReadoutElement::findGang(int gasGap, const Amg::Vector3D& localPos) const {
#ifndef NDEBUG
        if (!validGap(gasGap)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" given.");
#endif
        double z = localPos.z();
        for (int gang = 1; gang <= nWireGangs(gasGap); ++gang) {
            if (z < wireGangTopX(gasGap, gang)) return gang;
        }
        return nWireGangs(gasGap);
    }

    int TgcReadoutElement::findStrip(int gasGap, const Amg::Vector3D& localPos) const {
#ifndef NDEBUG
        if (!validGap(gasGap)) THROW_EXCEPT("Invalid gasGap "<<gasGap<<" given.");
#endif
        double x = localPos.x();
        double z = localPos.z();
        for (int strip = 1; strip <= nStrips(gasGap); ++strip) {
            if (stripLowEdgeLocX(gasGap, strip, z) < x && x < stripHighEdgeLocX(gasGap, strip, z)) return strip;
        }
        return nStrips(gasGap);
    }

    bool TgcReadoutElement::validGap(int gasGap) const {
        bool isValid = (1 <= gasGap && gasGap <= nGasGaps());
        if (!isValid) {
            ATH_MSG_WARNING( " gas gap is out of range; limits are 1-" << nGasGaps() );
        }
        return isValid;
    }

    bool TgcReadoutElement::validGang(int gasGap, int gang) const {
        return validGap(gasGap) && (1 <= gang && gang <= (nWireGangs(gasGap) + 1));
    }

    bool TgcReadoutElement::validStrip(int gasGap, int strip) const {
        return validGap(gasGap) && (1 <= strip && strip <= nStrips(gasGap));
    }

    void TgcReadoutElement::fillCache() {
        if (m_surfaceData) {
            ATH_MSG_WARNING( "calling fillCache on an already filled cache" );
            return;
        }
        m_surfaceData = std::make_unique<SurfaceData>();
        m_stripSlope = 1. / (getRsize() - 2. * physicalDistanceFromBase());
        m_locMinPhi = -std::atan2((getLongSsize() - getSsize()) / 2, length());
        m_locMaxPhi = std::atan2((getLongSsize() - getSsize()) / 2, length());

        // loop over all gas gaps
        for (int gp = 1; gp <= nGasGaps(); ++gp) {
            // loop over phi/eta projections
            for (bool isStrip : {false, true}) {
                Identifier id = m_idHelper.channelID(identify(), gp, isStrip, 1);
                const Amg::Translation3D xfp{localGasGapPos(gp)};
                Amg::Transform3D trans3D = absTransform() * xfp;
                Amg::RotationMatrix3D muonTRotation(trans3D.rotation());
                Amg::RotationMatrix3D surfaceTRotation{Amg::RotationMatrix3D::Identity()};
                surfaceTRotation.col(0) = muonTRotation.col(1);
                surfaceTRotation.col(1) = muonTRotation.col(2);
                surfaceTRotation.col(2) = muonTRotation.col(0);
                Amg::Transform3D trans(surfaceTRotation);
                if (!isStrip) trans = trans * Amg::getRotateZ3D(M_PI / 2);
                Amg::Translation3D t(trans3D.translation());
                trans.pretranslate(trans3D.translation());
                m_surfaceData->m_layerTransforms.push_back(std::move(trans));
                m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this, id));

                if (isStrip) {
                    m_surfaceData->m_layerCenters.push_back(m_surfaceData->m_layerTransforms.back().translation());
                    m_surfaceData->m_layerNormals.push_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D::UnitZ());
                }
            }
        }

        m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::TrapezoidBounds>(getSsize() / 2., 
                                                                                        getLongSsize() / 2., 
                                                                                        getRsize() / 2.));  // phi measurement
        m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RotatedTrapezoidBounds>(getRsize() / 2., 
                                                                                               getSsize() / 2., 
                                                                                               getLongSsize() / 2.));  // eta measurement
    }

    bool TgcReadoutElement::containsId(const Identifier& id) const {
        int gasGap = m_idHelper.gasGap(id);
        if (gasGap < 1 || gasGap > nGasGaps()) return false;

        int isStrip = m_idHelper.isStrip(id);
        int ch = m_idHelper.channel(id);
        if (isStrip == 0) {
            if (ch < 1 || ch > nWireGangs(gasGap)) return false;
        } else if (isStrip == 1) {
            if (ch < 1 || ch > nStrips(gasGap)) return false;
        } else
            return false;

        return true;
    }

    double TgcReadoutElement::distanceToReadout(const Amg::Vector2D&, const Identifier&) const {
        THROW_EXCEPT("distanceToReadout() -- method not implemented");
        return 0.;
    }
    int TgcReadoutElement::stripNumber(const Amg::Vector2D&, const Identifier&) const {
        THROW_EXCEPT("stripNumber() -- method not implemented");
        return 0. ;
    }
    bool TgcReadoutElement::stripPosition(const Identifier& id, Amg::Vector2D& pos) const {
        /** please don't copy the inefficient code below!! Look at the RpcReadoutElement for a proper implementation */
        Amg::Vector3D gpos = channelPos(id);
        if (!surface(id).globalToLocal(gpos, gpos, pos)) {
            ATH_MSG_WARNING( " stripPosition:: globalToLocal failed " <<Amg::toString(surface(id).transform().inverse() * gpos,2));
            return false;
        }
        return true;
    }
    Amg::Vector3D TgcReadoutElement::localSpacePoint(const Identifier& stripId, 
                                                     const Amg::Vector3D& etaHitPos,
                                                     const Amg::Vector3D& phiHitPos) const {
        
        
        const Amg::Transform3D globToLoc{transform(stripId).inverse()};
        const Amg::Vector3D lEtaHitPos = globToLoc * etaHitPos;
        const Amg::Vector3D lPhiHitPos = globToLoc * phiHitPos;
        const Amg::Vector3D phiHitDir = localStripDir(stripId);
 
        ATH_MSG_VERBOSE("local space piont 2D "<<idHelperSvc()->toString(stripId)
                      <<"eta hit: "<<Amg::toString(lEtaHitPos)<<" phi hit:"<<Amg::toString(lPhiHitPos)
                      <<"phi dir: "<<Amg::toString(phiHitDir));
        return lPhiHitPos +
               Amg::intersect<3>(lEtaHitPos, Amg::Vector3D::UnitX(), lPhiHitPos, phiHitDir).value_or(0) * phiHitDir;

    }
    bool TgcReadoutElement::spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const {
        Amg::Vector3D gpos{Amg::Vector3D::Zero()};
        if (!spacePointPosition(phiId, etaId, gpos) ||
            !surface(phiId).globalToLocal(gpos, gpos, pos)) {
            ATH_MSG_WARNING( " stripPosition:: globalToLocal failed " << Amg::toString(surface(phiId).transform().inverse() * gpos,2));
            return false;
        }
        return true;
    }
    bool TgcReadoutElement::spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const {
        // get orientation angle of strip to rotate back from local frame to strip
        if (nStrips(m_idHelper.gasGap(phiId)) <= 1) {
            ATH_MSG_VERBOSE("No strips defined for plane "<<idHelperSvc()->toString(phiId));
            pos = Amg::Vector3D::Zero();
            return false;
        }
        pos = transform(phiId) * localSpacePoint(phiId, channelPos(etaId), channelPos(phiId));
        return true;
    }

}  // namespace MuonGM
#undef THROW_EXCEPT
