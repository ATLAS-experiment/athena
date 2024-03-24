/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The Rpc detector = an assembly module = RPC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/RpcReadoutElement.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>

#include <cmath>
#include <stdexcept>


#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"



#define THROW_EXCEPTION(MSG)                                                                            \
     {                                                                                                  \
        std::stringstream sstr{};                                                                       \
        sstr<<"RpcReadoutElement - "<<idHelperSvc()->toStringDetEl(identify())<<" "<<__LINE__<<": ";    \
        sstr<<MSG;                                                                                      \
        throw std::runtime_error(sstr.str());                                                           \
     }                                                                                                  \


namespace MuonGM {

    RpcReadoutElement::RpcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int /*fi*/, bool is_mirrored,
                                         MuonDetectorManager* mgr) :
        MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::Rpc),
        m_mirrored{is_mirrored}  {
        std::string gVersion = manager()->geometryVersion();

        m_descratzneg = (zi < 0 && !is_mirrored);
        setStationName(stName);
    }

    RpcReadoutElement::~RpcReadoutElement()  = default;

    void RpcReadoutElement::setYTranslation(const double y) { m_y_translation = y; }
    void RpcReadoutElement::setZTranslation(const double z) { m_z_translation = z; }
    void RpcReadoutElement::setNumberOfLayers(const int nlay) { m_nlayers = nlay; }
    void RpcReadoutElement::setDoubletR(int doubletR) { m_dbR = doubletR;}
    void RpcReadoutElement::setDoubletZ(int doubletZ) { m_dbZ = doubletZ; }
    void RpcReadoutElement::setDoubletPhi(int doubletPhi) { m_dbPhi = doubletPhi; }
    double RpcReadoutElement::localStripSCoord(int doubletPhi, bool measphi, int strip) const {
        bool notintheribs = !inTheRibs();
        if ((doubletPhi != m_dbPhi && m_nphistrippanels == 1 && notintheribs) ||
            (m_nphistrippanels != 1 && (doubletPhi < 1 || doubletPhi > m_nphistrippanels))) {
            THROW_EXCEPTION("doublet Z"<<doubletPhi<<" outside range 1-"<<m_nphistrippanels<<" with doubletZ: "<<m_dbPhi);
        }
        if (strip < 1 || strip > Nstrips(measphi)) {
            THROW_EXCEPTION("strip "<<strip<<" outside range 1-"<<Nstrips(measphi)<<" for measphi="<<measphi);
        }

        double local_s = 0.;
        const int dbphi = std::min(doubletPhi, m_nphistrippanels) - 1;
        if (measphi)
            local_s = m_first_phistrip_s[dbphi] + (strip - 1) * StripPitch(measphi);
        else
            local_s = m_etastrip_s[dbphi];


       ATH_MSG_VERBOSE("Ssize, ndvs, nstr/pan, spitch, 1st-strp " << m_Ssize << " " << m_nphistrippanels << " "
                << m_nphistripsperpanel << " " << m_phistrippitch << " " << m_first_phistrip_s[doubletPhi - 1] << std::endl
               << "localStripSCoord: local_s is " << local_s << " for doubletPhi: " << doubletPhi
                << ", measuresPhi: " << measphi   << ", strip: " << strip);
        return local_s;
    }
    double RpcReadoutElement::localStripZCoord(bool measphi, int strip) const {
        if (strip < 1 ||  strip > Nstrips(measphi)) {
            THROW_EXCEPTION("Strip "<<strip<<" outside range 1-"<<Nstrips(measphi)<<" for measphi="<<measphi);
        }

        double local_z{0};
        if (!measphi) {
            local_z = m_first_etastrip_z + (strip - 1) * StripPitch(measphi  );
        } else {           
            local_z = m_phistrip_z;
        }
        ATH_MSG_VERBOSE("Zsize, ndvz, nstr/pan, zpitch, 1st-strp " << m_Zsize << " "
                << m_netastripsperpanel << " " << m_etastrippitch << " " << m_first_etastrip_z << std::endl
                << "localStripZCoord: local_z is " << local_z << " measuresPhi, " << measphi   << ", strip: " << strip );
        return local_z;
    }

    Amg::Vector3D RpcReadoutElement::stripPos(int doubletPhi, int gasGap, bool measphi, int strip) const {

        ATH_MSG_VERBOSE("stripPos for doubletPhi: " << doubletPhi << ", gasGap:"
                << gasGap << ", measuresPhi: " << measphi   << ", strip:" << strip );

        // global position of a generic strip !!!!!
        const Amg::Vector3D localP = localStripPos(doubletPhi, gasGap, measphi  , strip);

        const Amg::Transform3D& rpcTrans{absTransform()};

        ATH_MSG_VERBOSE("RpcReadoutElement::stripPos got localStripPos " << Amg::toString(localP) << std::endl
                        << "RpcReadoutElement::stripPos gl. transl. R, phi "
                << rpcTrans.translation().perp() << " " << rpcTrans.translation().phi() << " R-Rsize/2 "
                << rpcTrans.translation().perp() - m_Rsize * 0.5 );

        return rpcTrans * localP;
    }

    Amg::Vector3D RpcReadoutElement::localStripPos(int doubletPhi, int gasGap, bool measphi,
                                                   int strip) const {

        ATH_MSG_VERBOSE("localstripPos for doubletPhi: " << doubletPhi 
                     << ", gasGap: " << gasGap << ", mesuresPhi: " << measphi  
                     << ", strip: " << strip );


        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // gg numbering is swapped
        // except for BI chambers (with 3 gas gaps -> this is taken into account in localTopGasGap()
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lstrip = strip;
        
        if (!m_hasDEDontop && !measphi) {
            lstrip = NetaStrips() - strip + 1;
        }
        int ldoubletPhi = doubletPhi;
        // if the station is mirrored, the Rpc is rotated by 180deg around its local x axis
        // numbering of phi strips must be reversed;
        // numbering of eta strips is unchanged;
        // numbering of doubletPhi must be reversed if m_nphistrippanels>1
        // numbering of doubletZ   is unchanged;
        if (isMirrored()) {
            if (measphi) lstrip = NphiStrips() - lstrip + 1;
            if (m_nphistrippanels != 1) {
                ldoubletPhi = doubletPhi + 1;
                if (ldoubletPhi > 2) ldoubletPhi = 1;
            }

            ATH_MSG_VERBOSE( "localstrippos  isMirrored =" << isMirrored() << " lstrip, ldoubletPhi " 
                        << lstrip << " " << ldoubletPhi );
        }

        // the special case of chambers at Z<0 but not mirrored
        // numbering of eta strips must be reversed;
        // numbering of phi strips is unchanged;
        // numbering of doubletPhi   is unchanged;
        if (m_descratzneg) {
            if (!measphi) lstrip = NetaStrips() - lstrip + 1;
            ATH_MSG_VERBOSE("localstrippos special not mirrored at eta<0 = lstrip, ldoublerZ " << lstrip);
        }

        // the only RPCs in ATLAS which have 3 gasGaps (layers) are BI RPCs and those only have 1 doubletPhi
        if (m_nlayers == 3 && ldoubletPhi != 1) {
           THROW_EXCEPTION("localStripPos() - found ldoubletPhi=" << ldoubletPhi 
                        << " for BI RPC which cannot be true, setting to 1");
        }
        Amg::Vector3D localP(m_gasGap_xPos[gasGap - 1], 
                             localStripSCoord(ldoubletPhi, measphi, lstrip),
                             localStripZCoord(measphi, lstrip));
        ATH_MSG_VERBOSE("localstrippos = " <<Amg::toString(localP));
        return localP;
    }

    Amg::Vector3D RpcReadoutElement::localStripPos(const Identifier& id) const {
        return localStripPos(m_idHelper.doubletPhi(id), m_idHelper.gasGap(id), 
                             m_idHelper.measuresPhi(id), m_idHelper.strip(id));
    }

    Amg::Vector3D RpcReadoutElement::stripPos(const Identifier& id) const {
        return stripPos(m_idHelper.doubletPhi(id), m_idHelper.gasGap(id), 
                        m_idHelper.measuresPhi(id), m_idHelper.strip(id));
   }

    bool RpcReadoutElement::rotatedRpcModule() const { return (!m_hasDEDontop); }
  
    Amg::Vector3D RpcReadoutElement::gasGapPos(const Identifier& id) const {
        return gasGapPos(m_idHelper.doubletPhi(id), m_idHelper.gasGap(id));
    }

    Amg::Vector3D RpcReadoutElement::gasGapPos(int doubletPhi, int gasgap) const {
        const Amg::Vector3D localP = localGasGapPos(doubletPhi, gasgap);
        ATH_MSG_VERBOSE("RpcReadoutElement::gasGapPos got localGasGapPos" << Amg::toString(localP,3));
        return absTransform() * localP;
    }
    Amg::Vector3D RpcReadoutElement::localGasGapPos(const Identifier& id) const {
        return localGasGapPos(m_idHelper.doubletPhi(id), m_idHelper.gasGap(id));
    }
    Amg::Vector3D RpcReadoutElement::localGasGapPos(int doubletPhi, int gasgap) const {
        double local_s{0.}, local_z{0.};
        /// BI RPC, don't have centered gasGaps. Take hardcoded parameters instead
        if (numberOfLayers() == 3) {  
            local_s = m_y_translation;
            local_z = m_z_translation;
        } else {
           local_s = NphiStripPanels() != 1 ? -m_Ssize / 4. + (doubletPhi -1) *m_Ssize / 2 : 0; 
        }
        const Amg::Vector3D localGasGap{m_gasGap_xPos[gasgap -1], local_s, local_z};
        return localGasGap;
    }
    Amg::Vector3D RpcReadoutElement::localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        return localToGlobalTransf(id) * x;
    }
    Amg::Transform3D RpcReadoutElement::localToGlobalTransf(const Identifier& id) const {
        return localToGlobalTransf(m_idHelper.doubletPhi(id), m_idHelper.gasGap(id));
    }
    Amg::Transform3D RpcReadoutElement::localToGlobalTransf(int dbPhi, int gasGap) const {
        return absTransform() * Amg::Translation3D{localGasGapPos(dbPhi, gasGap)} *
               (rotatedRpcModule() ? Amg::getRotateY3D(180. * CLHEP::deg) : Amg::Transform3D::Identity());     
    }
    Amg::Transform3D RpcReadoutElement::globalToLocalTransf(const Identifier& id) const { return localToGlobalTransf(id).inverse(); }
    Amg::Vector3D RpcReadoutElement::globalToLocalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        return globalToLocalTransf(id) * x;
    }

    double RpcReadoutElement::distanceToPhiReadout(const Amg::Vector3D& P) const {
        // P is a point in the global reference frame
        // we want to have the distance from the side of the phi readout (length travelled along a phi strip) from a signal produced at P)
        // m_set will not be null but be initialized in "initDesign()" earlier
        // if it is null the code should crash! - because we're time-critical here a check for null was not implemented
        unsigned int ndbz{0};
        for (int dbz = 1 ; dbz <= m_idHelper.doubletZMax(identify()); ++dbz) {
            const Identifier dbzId = m_idHelper.channelID(identify(), dbz, 1, 1, 0, 1);
            ndbz+=(manager()->getRpcReadoutElement(dbzId) != nullptr);
        }

        double dist = -999.;
        double zPoint = P.z();
        double Zsizehalf = getZsize() / 2.;
        double recenter = REcenter().z();
        double zLow = recenter - Zsizehalf;
        double zUp = recenter + Zsizehalf;

        if (ndbz == 1) {
            if (zPoint < zLow || zPoint > zUp) {
                ATH_MSG_DEBUG("RpcReadoutElement with id " << idHelperSvc()->toStringDetEl(identify())
                        << " ::distanceToPhiReadout --- z of the Point  " << P.z() << " is out of the rpc-module range (" << zLow << ","
                        << zUp << ")");
                /// std clamp results in differences... Interesting
                zPoint = std::clamp(zPoint,zLow, zUp);
            }
            if (sideC())
                dist = zUp - zPoint;
            else
                dist = zPoint - zLow;

        } else {
            if (zPoint < zLow || zPoint > zUp) {
                ATH_MSG_DEBUG("RpcReadoutElement with id " << idHelperSvc()->toStringDetEl(identify())
                        << " ::distanceToPhiReadout --- z of the Point  " << P.z() << " is out of the rpc-module range (" << zLow << ","
                        << zUp << ") ");
                /// std clamp results in differences... Interesting
                zPoint = std::clamp(zPoint, zLow, zUp);                
            }
            if (m_dbZ == 1 || m_dbZ == 3) {
                if (sideC())
                    dist = zUp - zPoint;
                else
                    dist = zPoint - zLow;

            } else if (m_dbZ == 2) {
                if (sideC())
                    dist = zPoint - zLow;
                else
                    dist = zUp - zPoint;
            }
        }
        return dist;
    }

    double RpcReadoutElement::distanceToEtaReadout(const Amg::Vector3D& P) const { 

        double dist = -999999.;
        double pAmdbL = GlobalToAmdbLRSCoords(P).x();
        double myCenterAmdbL = GlobalToAmdbLRSCoords(REcenter()).x();
        double sdistToCenter = pAmdbL - myCenterAmdbL;
        if (std::abs(sdistToCenter) > getSsize() * 0.5) {            
           ATH_MSG_DEBUG("RpcReadoutElement with id " << idHelperSvc()->toStringDetEl(identify())
                    << " ::distanceToEtaReadout --- in amdb local frame x of the point  " << pAmdbL 
                    << " is out of the rpc-module range ("<< myCenterAmdbL - getSsize() * 0.5 << "," 
                    << myCenterAmdbL + getSsize() * 0.5 << ")");

            if (sdistToCenter > 0) {
                sdistToCenter = getSsize() * 0.5;
                ATH_MSG_DEBUG("setting distance to " << sdistToCenter);
            } else if (sdistToCenter < 0) {
                sdistToCenter = -getSsize() * 0.5;
                ATH_MSG_DEBUG("setting distance to " << sdistToCenter);
            }
        }
        if (m_nphistrippanels == 2) {
            dist = getSsize() * 0.5 - std::abs(sdistToCenter);
        } else {
            // assumes readout is at smallest phi
            dist = getSsize() * 0.5 + (sdistToCenter);
        }
        return dist;
    }

    void RpcReadoutElement::initDesign() {
        m_phiDesigns.reserve(NphiStripPanels());
        m_etaDesigns.reserve(NphiStripPanels());

        for (int doubletPhi = 1; doubletPhi <= NphiStripPanels(); ++doubletPhi) {
            const Amg::Transform3D gToSurf = MuonClusterReadoutElement::transform(surfaceHash(doubletPhi, 1, 1)).inverse();
            const Amg::Vector3D locStripPos1 = gToSurf * stripPos(doubletPhi, 1, true, 1);
            const Amg::Vector3D locStripPos2 = gToSurf * stripPos(doubletPhi, 1, true, 2);

            const Amg::Transform3D gToSurfEta = MuonClusterReadoutElement::transform(surfaceHash(doubletPhi, 1, 0)).inverse();
            const Amg::Vector3D locStripPosEta1 = gToSurfEta * stripPos(doubletPhi, 1, false, 1);
            const Amg::Vector3D locStripPosEta2 = gToSurfEta * stripPos(doubletPhi, 1, false, 2);

            MuonStripDesign phiDesign;
            phiDesign.nstrips = NphiStrips();
            phiDesign.stripPitch = StripPitch(1);
            phiDesign.stripLength = StripLength(1);
            phiDesign.stripWidth = StripWidth(1);

            phiDesign.firstStripPos = locStripPos1.block<2,1>(0,0);
            if (locStripPos2.x() - locStripPos1.x() < 0.) {
                phiDesign.stripPitch *= -1.;
                THROW_EXCEPTION("Define a positive definite phi pitch...");
            }
            phiDesign.invStripPitch = 1. / phiDesign.stripPitch;

            Amg::Vector2D pos1{Amg::Vector2D::Zero()}, pos2{Amg::Vector2D::Zero()};
            phiDesign.stripPosition(1, pos1);
            phiDesign.stripPosition(2, pos2);

            if (std::abs(pos1.x() - locStripPos1.x()) > 1e-6) {
                THROW_EXCEPTION(" bad local strip pos "<<
                    std::endl<<" phi local strip positions " << Amg::toString(locStripPos1) << "   " 
                             <<Amg::toString(locStripPos2) << " first strip " << phiDesign.firstStripPos 
                             << " pitch " << phiDesign.stripPitch << " from calc " << locStripPos2.x() - locStripPos1.x() <<
                    std::endl<<" checking strip position: phi design  " << Amg::toString(pos1) << " " << Amg::toString(pos2));
            }

            m_phiDesigns.push_back(std::move(phiDesign));

            MuonStripDesign etaDesign;
            etaDesign.nstrips = NetaStrips();
            etaDesign.stripPitch = StripPitch(0);
            etaDesign.stripLength = StripLength(0);
            etaDesign.stripWidth = StripWidth(0);

            etaDesign.firstStripPos = locStripPosEta1.block<2,1>(0,0);
            if (locStripPosEta2.x() - locStripPosEta1.x() < 0.) {
                etaDesign.stripPitch *= -1.;
            }
            etaDesign.invStripPitch = 1. / etaDesign.stripPitch;

            etaDesign.stripPosition(1, pos1);
            etaDesign.stripPosition(2, pos2);

            if (std::abs(pos1.x() - locStripPosEta1.x()) > 1e-6) {                
                THROW_EXCEPTION(" bad local strip pos "<<std::endl
                              <<" eta local strip positions " << Amg::toString(locStripPosEta1) 
                              << "   " <<Amg::toString(locStripPosEta2)<< " first strip "
                              << etaDesign.firstStripPos << " pitch " << etaDesign.stripPitch << " from calc "
                              << locStripPosEta2.x() - locStripPosEta1.x() <<std::endl
                              <<" checking strip position: eta design  " << Amg::toString(pos1) 
                              << " " << Amg::toString(pos2));
            }

            m_etaDesigns.push_back(std::move(etaDesign));
        }
    }


    void RpcReadoutElement::fillCache() {

        if (m_surfaceData) {
            ATH_MSG_WARNING("calling fillCache on an already filled cache");
            return;
        }
        m_surfaceData = std::make_unique<SurfaceData>();
        for (int dbPhi = 1; dbPhi <= NphiStripPanels(); ++dbPhi) {
            for (int gasGap = 1; gasGap <= numberOfLayers(true); ++gasGap) {
                const Amg::Translation3D xfp{localGasGapPos(dbPhi, gasGap)};
                Amg::Transform3D trans3D = localToGlobalTransf(dbPhi, gasGap);

                // surface()
                Amg::RotationMatrix3D muonTRotation(trans3D.rotation());
                if (isMirrored()) muonTRotation = muonTRotation * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitX());

                Amg::RotationMatrix3D surfaceTRotation;
                surfaceTRotation.col(0) = muonTRotation.col(1);
                surfaceTRotation.col(1) = muonTRotation.col(2);
                surfaceTRotation.col(2) = muonTRotation.col(0);

                for (bool measphi  : { true, false}) {
                    const Identifier id = m_idHelper.channelID(identify(), getDoubletZ(), dbPhi, gasGap, measphi  , 1);

                    Amg::Transform3D trans(surfaceTRotation);
                    if (!measphi)  trans = trans * Amg::getRotateZ3D(isMirrored()? -M_PI_2 : M_PI_2);
                    trans.pretranslate(trans3D.translation());

                    m_surfaceData->m_layerTransforms.push_back(trans);
                    m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this, id));

                    if (measphi) {
                        /// Changing this line to transform().translation which is effectively the same as multipltying the same 
                        /// thing by the null vector triggers the FT0V check.... My train station...
                        m_surfaceData->m_layerCenters.push_back(m_surfaceData->m_layerTransforms.back().translation());
                        m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() *
                                                                Amg::Vector3D::UnitZ());
                    }
                }
            }
        }
        m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RectangleBounds>(0.5* StripLength(true), 
                                                                                        0.5* StripLength(false)));
        m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RectangleBounds>(0.5* StripLength(false), 
                                                                                        0.5* StripLength(true)));
    }

    bool RpcReadoutElement::containsId(const Identifier& id) const {
        if (m_idHelper.doubletR(id) != getDoubletR() ||
            m_idHelper.doubletZ(id) != getDoubletZ()) return false;

      
        int doubletPhi = m_idHelper.doubletPhi(id);
        if (doubletPhi != getDoubletPhi() && m_nphistrippanels == 1) { return false; }
        if (doubletPhi < 1 || doubletPhi > m_nphistrippanels) {
            if (doubletPhi != 2 || !inTheRibs()) { return false; }
        }
        int gasgap = m_idHelper.gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;
        int strip = m_idHelper.channel(id);
        if (strip < 1 || strip > Nstrips(m_idHelper.measuresPhi(id))) return false;
        return true;
    }

    bool RpcReadoutElement::inTheRibs() const {
        return ((getStationName().substr(0, 3) == "BMS") && (getTechnologyName() == "RPC07" || getTechnologyName() == "RPC08"));
    }

    int RpcReadoutElement::surfaceHash(int dbPhi, int gasGap, bool measphi ) const {
        // if there is only one doublet phi we should always use one in the hash calculation
        if (m_nphistrippanels == 1) dbPhi = 1;
        if (dbPhi > NphiStripPanels() || gasGap > numberOfLayers(true)) {            
            ATH_MSG_WARNING(" surfaceHash: identifier out of range dbphi " << dbPhi << " max " << NphiStripPanels() << " ch dbphi "
                    << getDoubletPhi() << " gp " << gasGap << " max " << numberOfLayers());
            return -1;
        }
        return (dbPhi - 1) * (2 * NphiStripPanels()) + 2 * (gasGap - 1) + (measphi   ? 0 : 1);
    }

    int RpcReadoutElement::layerHash(int dbPhi, int gasGap) const {
        if (m_nphistrippanels == 1) dbPhi = 1;

        if (dbPhi > NphiStripPanels() || gasGap > numberOfLayers(true)) {
            ATH_MSG_WARNING(" layerHash: identifier out of range dbphi " << dbPhi << " max " << NphiStripPanels() << " ch dbphi "
                << getDoubletPhi() << " gp " << gasGap << " max " << numberOfLayers());
            return -1;
        }
        return (dbPhi - 1) * (NphiStripPanels()) + (gasGap - 1);
    }

    const MuonStripDesign* RpcReadoutElement::getDesign(const Identifier& id) const {
        unsigned int phipanel = std::min(m_nphistrippanels, m_idHelper.doubletPhi(id));
        if (phipanel > m_phiDesigns.size()) {           
            ATH_MSG_WARNING(" bad identifier, no MuonStripDesign found ");
            return nullptr;
        }
        return m_idHelper.measuresPhi(id) ? &m_phiDesigns[phipanel - 1] : &m_etaDesigns[phipanel - 1];
    }

}  // namespace MuonGM
