/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The Mdt detector = a multilayer = MDT in amdb
 ----------------------------------------------------
 ***************************************************************************/

#include "MuonReadoutGeometry/MdtReadoutElement.h"

#include <limits>
#include <utility>

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include "GeoModelHelpers/GeoShapeUtils.h"

#include "MuonAlignmentData/BLinePar.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

#include "CxxUtils/inline_hints.h"
// From Dan Levin: MDT
// linear density of wire: lambda=wireLinearDensity=19.3 [gm/cm^3] * PI*
//(25 *10^-4 )^2 [CLHEP::cm^2] = 378.954 microgram/CLHEP::cm
// From Dan Levin: MDT
// wireTen=350 for most chambers,  285 gm for some NIKHEF chambers (BOL ?),

#define verbose_Bline false
// Typical b-line par  values
// m_bz = 0.01;   // 10 microns
// m_bp = 0.1;    // 100 microns
// m_bn = 0.1;    // 100 microns
// m_sp = 0.001;  // 1 micron
// m_sn = 0.001;  // 1 micron
// m_tw = 0.1;    // 100 microns
// m_pg = 0.1;    // 100 microns
// m_tr = 0.1;    // 100 microns
// m_eg = 1.0e-4; // 100 ppm
// m_ep = 1.0e-5; // 10 ppm
// m_en = 1.0e-5; // 10 ppm

namespace {
    // the tube number of a tube in a tubeLayer in encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    constexpr unsigned int const maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;
    constexpr double linearDensity = 378.954;
}  // namespace

namespace MuonGM {

    MdtReadoutElement::MdtReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, MuonDetectorManager* mgr) :
        MuonReadoutElement(pv, mgr, Trk::DetectorElemType::Mdt) {
        // get the setting of the caching flag from the manager
        m_inBarrel = stName[0]== 'B';

        setStationName(stName);
    }
    void MdtReadoutElement::setMultilayer(const int ml) { m_multilayer = ml; }
    void MdtReadoutElement::setNLayers(const int nl) { m_nlayers = nl; }
    void MdtReadoutElement::clearBLinePar() { m_BLinePar = nullptr; }
    std::vector<const Trk::Surface*> MdtReadoutElement::surfaces() const {
        std::vector<const Trk::Surface*> elementSurfaces;
        elementSurfaces.reserve(m_tubeSurfaces.size() + 1);
        if (m_associatedSurface) { elementSurfaces.push_back(m_associatedSurface.get()); }
        for (const auto& s : m_tubeSurfaces) {
            if (s) elementSurfaces.push_back(s.get());
        }
        return elementSurfaces;
    }

    bool MdtReadoutElement::getWireFirstLocalCoordAlongZ(int tubeLayer, double& coord) const {
        coord = -9999.;
        if (tubeLayer > getNLayers() || tubeLayer < 1) return false;
        coord = m_firstwire_x[tubeLayer - 1];
        return true;
    }
    bool MdtReadoutElement::getWireFirstLocalCoordAlongR(int tubeLayer, double& coord) const {
        coord = -9999.;
        if (tubeLayer > getNLayers() || tubeLayer < 1) return false;
        coord = m_firstwire_y[tubeLayer - 1];
        return true;
    }

    void MdtReadoutElement::geoInitDone() {
        m_tubeGeo.resize(m_nlayers * m_ntubesperlayer);
        m_deformTransf.resize(m_nlayers * m_ntubesperlayer);
        m_tubeSurfaces.resize(m_nlayers * m_ntubesperlayer);

        int ntot_steps = m_nsteps;
        if (hasCutouts() && manager()->MinimalGeoFlag() == 0) { ntot_steps = m_nlayers * m_ntubesperlayer; }
        m_tubeBounds.resize(ntot_steps);
    }

    double MdtReadoutElement::getTubeLengthForCaching(const int tubeLayer, const int tube) const {
        double nominalTubeLength = 0.;
        if (barrel())
            nominalTubeLength = m_Ssize;
        else {
            int istep = int((tube - 1) / m_ntubesinastep);
            if (istep < 0 || istep >= m_nsteps) {
                ATH_MSG_WARNING( "getTubeLenght for Element with tech. " << getTechnologyType()
                    << " DEid = " << idHelperSvc()->toStringDetEl(identify()) << " called with: tubeL, tube " << tubeLayer << " " << tube
                    << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep );
                istep = 0;
            }
            nominalTubeLength = m_tubelength[istep];
        }

        double tlength = nominalTubeLength;

        if (hasCutouts()) {
            if (manager()->MinimalGeoFlag() == 0) {
                ATH_MSG_VERBOSE( " MdtReadoutElement " <<idHelperSvc()->toStringDetEl(identify())
                                << " has cutouts, check for real tube length for tubeLayer, tube " << tubeLayer << " " << tube );
                PVConstLink cv = getMaterialGeom();  // it is "Multilayer"
                int nGrandchildren = cv->getNChildVols();
                if (nGrandchildren <= 0) return tlength;
                // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
                int ii = (tubeLayer - 1) * m_ntubesperlayer + tube;
                // BIS78 only (the BIS7 of Run1/2 has no cutouts, thus, this block won't be reached)
                if ((getStationIndex() == m_stIdx_BIS && std::abs(getStationEta()) == 7)) --ii;
                if (m_idHelper.isBMG(identify())) {
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tubeLayer;
                    int kk = 0;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                ii = kk;
                                found = true;
                            }
                            ++kk;
                        }, cv);
                    if (found) {
                        ATH_MSG_DEBUG( " MdtReadoutElement tube match found for BMG - input : tube(" << tube << "), layer("
                            << tubeLayer << ") - output match : tube(" << ii % maxNTubesPerLayer << "), layer(" << ii / maxNTubesPerLayer
                            << ")" );
                    }
                }
                if (ii >= nGrandchildren) {
                    ATH_MSG_WARNING( " MdtReadoutElement " << idHelperSvc()->toStringDetEl(identify()) << " has cutouts, nChild = " << nGrandchildren
                        << " --- getTubeLength is looking for child with index ii=" << ii << " for tubeL and tube = " << tubeLayer << " "
                        << tube );
                    ATH_MSG_WARNING( "returning nominalTubeLength " );
                    return tlength;
                }
                if (ii < 0) {
                    ATH_MSG_WARNING( " MdtReadoutElement " << idHelperSvc()->toStringDetEl(identify()) << " has cutouts, nChild = " << nGrandchildren
                        << " --- getTubeLength is looking for child with index ii=" << ii << " for tubeL and tube = " << tubeLayer << " "
                        << tube );
                    ATH_MSG_WARNING( "returning nominalTubeLength " );
                    return tlength;
                }
                PVConstLink physChild = cv->getChildVol(ii);
                const GeoShape* shape = physChild->getLogVol()->getShape();
                if (shape == nullptr) return tlength;
                const GeoTube* theTube = dynamic_cast<const GeoTube*>(shape);
                if (theTube != nullptr)
                    tlength = 2. * theTube->getZHalfLength();
                else
                    ATH_MSG_WARNING( "PhysChild with index " << ii
                        << " out of (tubeLayer-1)*m_ntubesperlayer+tube with tl=" << tubeLayer << " tubes/lay=" << m_ntubesperlayer
                        << " t=" << tube << " for  MdtReadoutElement " << idHelperSvc()->toStringDetEl(identify()) );
            }
            if (std::abs(tlength - nominalTubeLength) > 0.1) {
                ATH_MSG_VERBOSE( "Tube " << tube << " in tubeLayer = " << tubeLayer
                    << " is affected by a cutout: eff. length =  " << tlength << " while nominal = " << nominalTubeLength
                    << " in station " << idHelperSvc()->toStringDetEl(identify()));
            } else {
                ATH_MSG_VERBOSE( "Tube " << tube << " in tubeLayer = " << tubeLayer
                        << " is NOT affected by the cutout: eff. length =  " << tlength << " while nominal = " << nominalTubeLength);
            }
        }
        return tlength;
    }

    double MdtReadoutElement::distanceFromRO(const Amg::Vector3D& x, int tubeLayer, int tube) const {
        // x is given in the global reference frame
        const Amg::Vector3D cPos = center(tubeLayer, tube);
        const Amg::Vector3D roPos = ROPos(tubeLayer, tube);
        const Amg::Vector3D c_ro = cPos - roPos;
        const Amg::Vector3D x_ro = x - roPos;

        double scalprod = c_ro.dot(x_ro);
        double wlen = getWireLength(tubeLayer, tube);
        if (wlen > 10. * CLHEP::mm)
            scalprod = std::abs(2. * scalprod / getWireLength(tubeLayer, tube));
        else {
            ATH_MSG_WARNING( " Distance of Point " <<Amg::toString(x) << " from RO side cannot be calculated (=0) since wirelength = " << wlen );
            scalprod = 0.;
        }
        return scalprod;
    }

    int MdtReadoutElement::isAtReadoutSide(const Amg::Vector3D& GlobalHitPosition, int tubeLayer, int tube) const {
        double distance = distanceFromRO(GlobalHitPosition, tubeLayer, tube);
        if (distance < 0) {
            ATH_MSG_WARNING( "isAtReadoutSide() - GlobalHitPosition appears to be outside the tube volume " << distance );
            return 1;
        } else if (distance <= getWireLength(tubeLayer, tube) / 2.)
            return 1;
        else if (distance < getWireLength(tubeLayer, tube))
            return -1;
        else {
            ATH_MSG_WARNING( "isAtReadoutSide() - GlobalHitPosition appears to be outside the tube volume " << distance );
            return -1;
        }
    }
    double MdtReadoutElement::RODistanceFromTubeCentre(const int tubeLayer, const int tube) const {
        return getWireLength(tubeLayer, tube) / 2.;
    }
    double MdtReadoutElement::signedRODistanceFromTubeCentre(const int tubeLayer, const int tube) const {
        // it is a signed quantity:
        // the sign corresponds to the sign of the z coordinate of the RO endplug in the tube
        // reference frame
        int amdb_plus_minus1 = 1;
        if (!m_zsignRO_tubeFrame.isValid()) {
            const MuonStation* ms = parentMuonStation();
            if (std::abs(ms->xAmdbCRO()) > 10.) {
                Amg::Vector3D tem = ms->xAmdbCRO()* Amg::Vector3D::UnitX();
                Amg::Transform3D amdbToGlobal{ms->getAmdbLRSToGlobal()};
                Amg::Vector3D temGlo = amdbToGlobal * tem;
                Amg::Vector3D ROtubeFrame = nodeform_globalToLocalTransf(tubeLayer, tube) * temGlo;
                if (ROtubeFrame.z() < 0)
                    m_zsignRO_tubeFrame.set(-1);
                else
                    m_zsignRO_tubeFrame.set(1);
            }
        }
        // if no CRO in a chamber in AMDB (BIS in layout R), use the standard convention for RO-HV side
        if (!m_zsignRO_tubeFrame.isValid()) {  
            int sign = 0;
            if (barrel()) {
                if (sideA()) {
                    if (largeSector())
                        sign = -1;
                    else
                        sign = 1;
                } else {
                    if (largeSector())
                        sign = 1;
                    else
                        sign = -1;
                }
                // a special case is BIS in sector 12
                if (getStationName().substr(0, 3) == "BIS" && getStationPhi() == 6) sign = -sign;
            } else {
                if (sideA()) {
                    if (largeSector())
                        sign = 1;
                    else
                        sign = -1;
                } else {
                    if (largeSector())
                        sign = -1;
                    else
                        sign = 1;
                }
            }
            m_zsignRO_tubeFrame.set(sign);
        }
        amdb_plus_minus1 = *m_zsignRO_tubeFrame.ptr();
        if (amdb_plus_minus1 == 0) {
            ATH_MSG_WARNING( "Unable to get the sign of RO side; signedRODistancefromTubeCenter returns 0" );
        }

        return amdb_plus_minus1 * getWireLength(tubeLayer, tube) / 2.;
    }

    Amg::Vector3D MdtReadoutElement::tubeFrame_localROPos(const int tubeLayer, const int tube) const {
        return signedRODistanceFromTubeCentre(tubeLayer, tube) * Amg::Vector3D::UnitZ();
    }
    Amg::Vector3D MdtReadoutElement::localROPos(const int tubeLayer, const int tube) const {
        return tubeToMultilayerTransf(tubeLayer, tube) * tubeFrame_localROPos(tubeLayer, tube);
    }
    Amg::Vector3D MdtReadoutElement::ROPos(const int tubeLayer, const int tube) const {
        return transform(tubeLayer, tube) * tubeFrame_localROPos(tubeLayer, tube);
    }
    Amg::Vector3D MdtReadoutElement::localTubePos(const int tubeLayer, const int tube) const {
        return fromIdealToDeformed(tubeLayer, tube) * nodeform_localTubePos(tubeLayer, tube);
    }

    Amg::Vector3D MdtReadoutElement::nodeform_localTubePos(const int tubeLayer, const int tube) const {
        ATH_MSG_VERBOSE( " Computing LocalTubePos for "<< idHelperSvc()->toStringDetEl(identify())
                                                       << "/" << tubeLayer << "/" << tube );
        double xtube{0.}, ytube{0.}, ztube{0.};
        if (barrel()) {
            xtube = -m_Rsize / 2. + m_firstwire_y[tubeLayer - 1];
            ztube = -m_Zsize / 2. + m_firstwire_x[tubeLayer - 1] + (tube - 1) * m_tubepitch;
        } else {
            xtube = -m_Zsize / 2. + m_firstwire_y[tubeLayer - 1];
            ztube = -m_Rsize / 2. + m_firstwire_x[tubeLayer - 1] + (tube - 1) * m_tubepitch;
        }
        Amg::Vector3D tubePos{xtube, ytube, ztube};
        if (hasCutouts()) {
            if (manager()->MinimalGeoFlag() == 0) {
                ATH_MSG_DEBUG( " MdtReadoutElement " << idHelperSvc()->toStringDetEl(identify()) << " has cutouts, check for real position of tubes " );
                PVConstLink cv = getMaterialGeom();  // it is "Multilayer"
                int nGrandchildren = cv->getNChildVols();
                // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
                int ii = (tubeLayer - 1) * m_ntubesperlayer + tube;

                // BIS78 only (the BIS7 of Run1/2 has no cutouts, thus, this block won't be reached)
                if ((getStationIndex() == m_stIdx_BIS && std::abs(getStationEta()) == 7)) --ii;
                if (m_idHelper.isBMG(identify())) {
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tubeLayer;
                    int kk = 0;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                ii = kk;
                                found = true;
                            }
                            ++kk;
                        },
                        &*cv);
                    if (found) {
                        ATH_MSG_DEBUG( " MdtReadoutElement tube match found for BMG - input : tube(" << tube << "), layer("
                            << tubeLayer << ") - output match : tube(" << ii % maxNTubesPerLayer << "), layer(" << ii / maxNTubesPerLayer
                            << ")" );
                    }
                }
                GeoTrf::Transform3D tubeTrans = cv->getXToChildVol(ii);
                PVConstLink tv = cv->getChildVol(ii);
                constexpr double maxtol = 0.0000001;

                if (std::abs(xtube - tubeTrans(0, 3)) > maxtol || std::abs(ztube - tubeTrans(2, 3)) > maxtol) {
                    THROW_EXCEPTION(__func__<<"("<<tubeLayer<<","<<tube<<")"<<" - mismatch between local from tube-id/pitch/cutout position"<<
                           Amg::toString(tubePos)<<" and GeoModel "<< Amg::toString(tubeTrans.linear().col(3))<<" for detector element "<<
                            idHelperSvc()->toStringDetEl(identify()) <<"There are "<<nGrandchildren<<" child volumes and "<<
                                       (m_ntubesperlayer * m_nlayers)<<" are expected. There should be "<<m_nlayers<<" and "<<
                                           m_ntubesperlayer<<" tubes per layer");
                }
                if (tubeTrans(1, 3) > maxtol) {

                    ATH_MSG_DEBUG( "This a tube with cutout stName/Eta/Phi/ml/tl/t = " << idHelperSvc()->toStringDetEl(identify())
                                         << "/" << tubeLayer << "/" << tube);
                    // check only for tubes actually shifted
                    if (std::abs(m_cutoutShift - tubeTrans(1, 3)) > maxtol) {
                        THROW_EXCEPTION(__func__<<"("<<tubeLayer<<","<<tube<<")"<<" - mismatch between local from tube-id/pitch/cutout position"<<
                           Amg::toString(tubePos)<<" and GeoModel "<< Amg::toString(tubeTrans.linear().col(3))<<" for detector element "<<
                            idHelperSvc()->toStringDetEl(identify()) <<"There are "<<nGrandchildren<<" child volumes and "<<
                                       (m_ntubesperlayer * m_nlayers)<<" are expected. There should be "<<m_nlayers<<" and "<<
                                           m_ntubesperlayer<<" tubes per layer");
                    }
                }

                Amg::Vector3D x = tubeTrans.translation();
                if (tube > m_ntubesperlayer || tubeLayer > m_nlayers) { x = tubePos; }
                return x;
            }
        }
        return tubePos;
    }

    Amg::Vector3D MdtReadoutElement::nodeform_tubePos(int tubeLayer, int tube) const {
        return transform() * nodeform_localTubePos(tubeLayer, tube);
    }
    Amg::Vector3D MdtReadoutElement::tubePos(int tubeLayer, int tube) const {
      return transform() * localTubePos(tubeLayer, tube);
    }
    Amg::Transform3D MdtReadoutElement::tubeToMultilayerTransf(const int tubeLayer, const int tube) const {
        return tubeToMultilayerTransf(nodeform_localTubePos(tubeLayer, tube),
                                      fromIdealToDeformed(tubeLayer, tube));
    }
#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif

    Amg::Transform3D MdtReadoutElement::globalTransform(const Amg::Vector3D& tubePos, const Amg::Transform3D& toDeform) const {
        return transform() * tubeToMultilayerTransf(tubePos, toDeform);
    }
    Amg::Transform3D MdtReadoutElement::tubeToMultilayerTransf(const Amg::Vector3D& tubePos, const Amg::Transform3D& toDeform) {
        return toDeform * Amg::Translation3D{tubePos} * Amg::getRotateX3D(90. * CLHEP::deg);
    }
    Amg::Transform3D MdtReadoutElement::nodeform_localToGlobalTransf(const int tubeLayer, const int tube) const {
        return globalTransform(nodeform_localTubePos(tubeLayer, tube), Amg::Transform3D::Identity());
    }
    Amg::Transform3D MdtReadoutElement::globalToLocalTransf(const int tubeLayer, const int tube) const {
        return transform(tubeLayer, tube).inverse();
    }

    Amg::Transform3D MdtReadoutElement::nodeform_globalToLocalTransf(const int tubeLayer, const int tube) const {
        return nodeform_localToGlobalTransf(tubeLayer, tube).inverse();
    }
    double MdtReadoutElement::getNominalTubeLengthWoCutouts(const int tubeLayer, const int tube) const {
        if (barrel())
            return m_Ssize;
        else {
            int istep = int((tube - 1) / m_ntubesinastep);
            if (istep < 0 || istep >= m_nsteps) {
                ATH_MSG_WARNING( "getNominalTubeLengthWoCutouts for Element named " << idHelperSvc()->toStringDetEl(identify())
                         << " called with: tubeL, tube " << tubeLayer
                    << " " << tube << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep);
                istep = 0;
            }
            return m_tubelength[istep];
        }
    }
    Amg::Vector3D MdtReadoutElement::localNominalTubePosWoCutouts(const int tubeLayer, const int tube) const {
        double xtube{0.}, ztube{0.};
        if (barrel()) {
            xtube = -m_Rsize / 2. + m_firstwire_y[tubeLayer - 1];
            ztube = -m_Zsize / 2. + m_firstwire_x[tubeLayer - 1] + (tube - 1) * m_tubepitch;
        } else {
            xtube = -m_Zsize / 2. + m_firstwire_y[tubeLayer - 1];
            ztube = -m_Rsize / 2. + m_firstwire_x[tubeLayer - 1] + (tube - 1) * m_tubepitch;
        }
        return Amg::Vector3D{xtube, 0., ztube};
    }

    const Amg::Transform3D& MdtReadoutElement::fromIdealToDeformed(const int tubeLayer, const int tube) const {
        size_t itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        if (itube >= m_deformTransf.size()) {
            ATH_MSG_WARNING(__func__<<"() :"<<__LINE__<< " called with tubeLayer or tube out of range in chamber "
                << idHelperSvc()->toStringDetEl(identify()) << " : layer " << tubeLayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute deformation for first tube in this chamber" );
            ATH_MSG_WARNING( "Please run in DEBUG mode to get extra diagnostic" );
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<Amg::Transform3D>& ptr = m_deformTransf.at(itube);
        if (!ptr) {
            Amg::Transform3D trans = deformedTransform(tubeLayer, tube);
            ptr.set(std::make_unique<Amg::Transform3D>(std::move(trans)));
            if (!m_haveDeformTransf) m_haveDeformTransf = true;
        }
        return *ptr;
    }

#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
    Amg::Transform3D
    MdtReadoutElement::deformedTransform(int tubeLayer, int tube) const {

        const MuonStation* ms = parentMuonStation();
        if ( !ms->hasBLines() && !ms->hasMdtAsBuiltParams()) {
            return Amg::Transform3D::Identity();
        }
        const Amg::Vector3D fixedPoint = ms->getBlineFixedPointInAmdbLRS();


        // Chamber parameters
        double width_narrow = m_Ssize;
        double width_wide = m_LongSsize;
        double height = barrel() ? ms->ZsizeMdtStation() : ms->RsizeMdtStation();
        double thickness = barrel() ? ms->RsizeMdtStation() : ms->ZsizeMdtStation();


       
        ATH_MSG_VERBOSE("Calculate deformed transform "<<idHelperSvc()->toStringDetEl(identify())
                        <<", layer: "<<tubeLayer<<", tube: "<<tube
                         <<", fixedPoint: "<<Amg::toString(fixedPoint)<<" / "
                        <<Amg::toString(ms->getGeoTransform()->getDefTransform() *ms->getNativeToAmdbLRS().inverse()* fixedPoint)
                         <<", height: "<<height<<", thickness: "<<thickness
                         <<", ideal tube: "<<Amg::toString(localNominalTubePosWoCutouts(tubeLayer,tube)));

#ifndef NDEBUG
        double heightML = barrel() ? m_Zsize : m_Rsize;
        double thicknessML = barrel() ? m_Rsize : m_Zsize;
        if (std::abs(height - heightML) > 10.) {
            ATH_MSG_DEBUG( "RE " << idHelperSvc()->toStringDetEl(identify())
                << "Different ML and MDTStation length in the precision coord.direction  ---  MultiLayerHeight, MDTstationHeigh "
                << heightML << " " << height << " MultiLayerThickness, MDTstationThickness " << thicknessML << " " << thickness);
        }
#endif
        // Chamber dimension in Z is defined with extraneous glue width. Correct for it
        double glue = (tubePitch() - 2. * outerTubeRadius());
        height -= glue;

        // Calculate transformation from native to AMDB.
        const Amg::Transform3D toAMDB = ms->getNativeToAmdbLRS() * toParentStation();
        const Amg::Transform3D fromAMDB = toAMDB.inverse();

        // Get positions of the wire center and end without deformations
        Amg::Vector3D pt_center = localNominalTubePosWoCutouts(tubeLayer, tube);
        double halftubelen = 0.5 * getNominalTubeLengthWoCutouts(tubeLayer, tube);

        // Compute tube ends in AMDB coordinates
        Amg::Vector3D pt_end1 = toAMDB * pt_center + halftubelen * Amg::Vector3D::UnitX();  // s>0 side
        Amg::Vector3D pt_end2 = toAMDB * pt_center - halftubelen * Amg::Vector3D::UnitX();  // s>0 side

        Amg::Vector3D pt_end1_new = pt_end1;
        Amg::Vector3D pt_end2_new = pt_end2;

        // if there are as built parameters ... apply them here
        if (ms->hasMdtAsBuiltParams()) {
            wireEndpointsAsBuilt(pt_end1_new, pt_end2_new, tubeLayer, tube);
        }

        // if there are deformation parameters ... apply them here
        if (ms->hasBLines()) {

            // Get positions after the deformations applied
            // first wire end point
            pt_end1_new = posOnDefChamWire(pt_end1_new, width_narrow, width_wide, height, thickness, fixedPoint);

            // second wire end point
            pt_end2_new = posOnDefChamWire(pt_end2_new, width_narrow, width_wide, height, thickness, fixedPoint);
        }

        // Switch tube ends back to MGM coordinates
        pt_end1 = fromAMDB * pt_end1;
        pt_end2 = fromAMDB * pt_end2;
        pt_end1_new = fromAMDB * pt_end1_new;
        pt_end2_new = fromAMDB * pt_end2_new;

        // Calculate deformation. Make sure that the wire length stays the same.
        // Code in positionOnDeformedChamber does not provide this by default.
        // Break transformation into translation of the wire center and the rotation of the wire
        // Move to the coordinate system originated at the wire center, then rotate the wire, then
        // move wire center to the new position
        const Amg::Vector3D pt_center_new = 0.5 * (pt_end1_new + pt_end2_new);
        const Amg::Transform3D to_center{Amg::getTranslate3D(-pt_center)};
        const Amg::Transform3D from_center{Amg::getTranslate3D(pt_center_new)};
        const Amg::Vector3D old_direction = (pt_end2 - pt_end1).unit();
        const Amg::Vector3D new_direction = (pt_end2_new - pt_end1_new).unit();
        const Amg::Vector3D rotation_vector = old_direction.cross(new_direction);
        

        Amg::Transform3D deformedTransform{Amg::Transform3D::Identity()};
        if (rotation_vector.mag() > 10. * std::numeric_limits<double>::epsilon()) {
            const Amg::AngleAxis3D wire_rotation(std::asin(rotation_vector.mag()), rotation_vector.unit());
            deformedTransform = from_center * wire_rotation * to_center;
        } else {
            deformedTransform = from_center * to_center;
        }
        ATH_MSG_VERBOSE("To center "<<GeoTrf::toString(to_center)<<" from: "<<GeoTrf::toString(from_center)<<
                  " -- direction: "<<GeoTrf::toString(old_direction)<<" vs. "<<GeoTrf::toString(new_direction)
                  <<" --> rot: "<<GeoTrf::toString(rotation_vector)<<" ==> "<<GeoTrf::toString(deformedTransform,true));
        return deformedTransform;
    }

    //   //Correspondence to AMDB parameters -TBM
    //   //cpl_x is tr "trapezoidal effect"
    //   //cpl_y is sy "Longbeam vertical sagitta"
    //   //cpl_z is sz "Longbeam horizontal sagitta"
    //   //sag_x is ?? "shearing deformation"
    //   //sag_y is so,sv "RO crossplate sag, HV crossplate sag"
    //   //sag_z is ?? "different long-beam bow for short/long side"
    //   //the_g is tw "common twist angle for HV and RO side"
    //   //the_c is tw "torsion along tube axis"
    //   //the_m is tw "torsion along tube axis"
    //   //tem_g is T, "temperature"
    //   //tem_c is ev "HV elongation"
    //   //tem_m is eo "RO elongation"

    //   /*
    //     CPM: mask-side cross plate (=readout side in endcap)
    //     CPC: CCD-side cross plate (=HV side in endcap)
    //     CPL: lens cross plate (=central cross plate)

    //     note that nearly all deformation parameter names are meaningless
    //   */
    Amg::Vector3D MdtReadoutElement::posOnDefChamWire(const Amg::Vector3D& locAMDBPos,
                                                      const double width_narrow,
                                                      const double width_wide,
                                                      const double height,
                                                      const double thickness,
                                                      const Amg::Vector3D& fixedPoint) const {

        using Parameter = BLinePar::Parameter;
        const double sp = m_BLinePar->getParameter(Parameter::sp);
        const double sn = m_BLinePar->getParameter(Parameter::sn);
        const double tw = m_BLinePar->getParameter(Parameter::tw);
        const double eg = m_BLinePar->getParameter(Parameter::eg);
        double ep = m_BLinePar->getParameter(Parameter::ep);
        double en = m_BLinePar->getParameter(Parameter::en);
        // S.Spagnolo Feb.6, 2011, modified by P.F Giraud, 2015-01-17
        // This version does not implement deformations modifying only the second
        // coordinate, or leaving the end-plug positions unchanged.
        // This version should be called only with the coordinates of the end-plugs
        // as argument, as is done in fromIdealToDeformed

        // MDT deformations like tube bow: bz,bp,bn bend the tube while the wire endpoints are not affected
        //                                 => the wire keeps it's nominal straight line trajectory but it is not concentric to the tube
        //                                 ==> in this function bz, bp and bn are ignored or set to 0
        // MDT deformations that extend/contract the wire longitudinally (while keeping it straight):
        //                                 delta_s from eg and tr are irrelevant for the tube geometry
        //                                 and for the wire trajectory => set to 0 here
        //                                 (should be applied as a correction to the
        //                                 tube lenght => tube surface bounds
        //                                 =++>>>> effect in tracking just through the gravitational sagging TOTALLY NEGLIGIBLE=> ignore)
        // pg is irrelevant for tracking purposes and (at least for the endcaps) is applies to the internal bars only, not to the tubes !!!
        //                                 =++>>>> IGNORE IT
        // ep,en: bend the tube by moving (differently) the endplugs ===> the wire straight trajectory is moved w.r.t. the nominal one
        //                                 in addition the tubes keep their nominal position at the center => the wire is not concentric to
        //                                 the tube delta_s from ep,en must also be considered for the implementation of the realistic tube
        //                                 trajectory induced by ep,en
        // tw,sp,sn,pg (for deltaT and deltaZ) are geometrical effects, that impact on tracking and keep the wire straight.



        Amg::Vector3D deformPos(locAMDBPos);

        // NOTE s0,z0,t0 are the coord. in the amdb frame of this point: the origin of the frame can be different than the fixed point for
        // deformations s0mdt,z0mdt,t0mdt
        //    (always equal to the point at lowest t,z and s=0 of the MDT stack)
        double s0 = locAMDBPos.x();
        double z0 = locAMDBPos.y();
        double t0 = locAMDBPos.z();
        ATH_MSG_VERBOSE( "** In "<<__func__<<" - width_narrow, width_wide, length, thickness, " << width_narrow << " " << width_wide
            << " " << height << " " << thickness << " " );
        ATH_MSG_VERBOSE( "** In "<<__func__<<" - going to correct for B-line the position of Point at " << s0 << " " << z0 << " "
            << t0 << " in the amdb-szt frame" );

        double s0mdt = s0;  // always I think !
        if (std::abs(fixedPoint.x()) > 0.01) s0mdt = s0 - fixedPoint.x();
        double z0mdt = z0;  // unless in the D section of this station there's a dy diff. from 0 for the innermost MDT multilayer (sometimes
                            // in the barrel)
        if (std::abs(fixedPoint.y()) > 0.01) z0mdt = z0 - fixedPoint.y();
        double t0mdt =
            t0;  // unless in the D section of this station there's a dz diff. from 0 for the innermost MDT multilayer (often in barrel)
        if (std::abs(fixedPoint.z()) > 0.01) t0mdt = t0 - fixedPoint.z();
        if (z0mdt < 0 || t0mdt < 0) {
            ATH_MSG_WARNING(""<<__func__<<": correcting the local position of a point outside the mdt station (2 multilayers) volume -- RE "
                << idHelperSvc()->toStringDetEl(identify()) << " local point: szt=" << s0 << " " << z0 << " " << t0
                << " fixedPoint " <<Amg::toString(fixedPoint) );
        }
        ATH_MSG_VERBOSE( "** In "<<__func__<<" - correct for offset of B-line fixed point " << s0mdt << " " << z0mdt << " " << t0mdt);

        double ds{0.},dz{0.},dt{0.};
        double width_actual = width_narrow + (width_wide - width_narrow) * (z0mdt / height);
        double s_rel = s0mdt / (width_actual / 2.);
        double z_rel = (z0mdt - height / 2.) / (height / 2.);
        double t_rel = (t0mdt - thickness / 2.) / (thickness / 2.);

        ATH_MSG_VERBOSE( "** In "<<__func__<<" - width_actual, s_rel, z_rel, t_rel  " << width_actual << " " << s_rel << " "
                                 << z_rel << " " << t_rel );

        // sp, sn - cross plate sag out of plane
        if ((sp != 0) || (sn != 0)) {
            double ztmp = z_rel * z_rel - 1;
            dt += 0.5 * (sp + sn) * ztmp + 0.5 * (sp - sn) * ztmp * s_rel;
        }

        // tw     - twist
        if (tw != 0) {
            dt -= tw * s_rel * z_rel;
            dz += tw * s_rel * t_rel * thickness / height;
            ATH_MSG_VERBOSE( "** In "<<__func__<<": tw=" << tw << " dt, dz " << dt << " " << dz );
        }

        // eg     - global expansion
        if (eg != 0) {
            double egppm = eg / 1000.;
            ds += 0.;
            dz += z0mdt * egppm;
            dt += t0mdt * egppm;
        }

        // ep, en - local expansion
        //
        // Imporant note: the chamber height and length, as they denoted in Christoph's talk,
        // correspond to thickness and height parameters of this function;
        //

        if ((ep != 0) || (en != 0)) {
            ep = ep / 1000.;
            en = en / 1000.;
            double phi = 0.5 * (ep + en) * s_rel * s_rel + 0.5 * (ep - en) * s_rel;
            double localDt = phi * (t0mdt - thickness / 2.);
            double localDz = phi * (z0mdt - height / 2.);
            dt += localDt;
            dz += localDz;
        }

        ATH_MSG_VERBOSE( "posOnDefChamStraighWire: ds,z,t = " << ds << " " << dz << " " << dt );
        deformPos[0] = s0 + ds;
        deformPos[1] = z0 + dz;
        deformPos[2] = t0 + dt;

        return deformPos;
    }

// Function to apply AsBuilt parameter correction to wire center and end position
// For definitions of AsBuilt parameters see Florian Bauer's talk:
// http://atlas-muon-align.web.cern.ch/atlas-muon-align/endplug/asbuilt.pdf
#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
    void
    MdtReadoutElement::wireEndpointsAsBuilt(Amg::Vector3D& locAMDBWireEndP, Amg::Vector3D& locAMDBWireEndN,
                                            const int tubeLayer, const int tube) const {
        const MdtAsBuiltPar* params = parentMuonStation()->getMdtAsBuiltParams();
        if (!params) {
            ATH_MSG_WARNING( "Cannot find Mdt AsBuilt parameters for chamber " << idHelperSvc()->toStringDetEl(identify()) );
            return;
        }

        ATH_MSG_VERBOSE( "Applying as-built parameters for chamber " << idHelperSvc()->toStringDetEl(identify())
                        << " tubeLayer=" << tubeLayer << " tube=" << tube);

        constexpr int nsid = 2;
        using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
        using multilayer_t = MdtAsBuiltPar::multilayer_t;
        std::array<MdtAsBuiltPar::tubeSide_t, nsid> tubeSide{tubeSide_t::POS, tubeSide_t::NEG};
        std::array<Amg::Vector3D, nsid> wireEnd{locAMDBWireEndP, locAMDBWireEndN};
        multilayer_t ml = (getMultilayer() == 1) ? multilayer_t::ML1 : multilayer_t::ML2;

        const double xmin = *std::min_element(m_firstwire_x.begin(), m_firstwire_x.begin() + m_nlayers) - outerTubeRadius();
        const int ref_layer = (ml == multilayer_t::ML1) ? m_nlayers : 1;
        const double y_offset = (ml == multilayer_t::ML1) ? outerTubeRadius() : -outerTubeRadius();

        for (int isid = 0; isid < nsid; ++isid) {  // first s>0 then s<0
            // Compute the reference for the as-built parameters
            double xref{0.}, yref{0.}, zref{0.};
            if (barrel()) {
                xref = -m_Rsize / 2. + m_firstwire_y[ref_layer - 1] + y_offset;
                zref = -m_Zsize / 2. + xmin;
            } else {
                xref = -m_Zsize / 2. + m_firstwire_y[ref_layer - 1] + y_offset;
                zref = -m_Rsize / 2. + xmin;
            }
            const Amg::Transform3D toAMDB = parentMuonStation()->getNativeToAmdbLRS() * toParentStation();
            Amg::Vector3D reference_point{xref, yref, zref};

            reference_point = toAMDB * reference_point;

            if (isid == 0)
                reference_point = reference_point + 0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1) * Amg::Vector3D::UnitX();
            else
                reference_point = reference_point -0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1) * Amg::Vector3D::UnitX();

            ATH_MSG_VERBOSE("AMDB transform "<<idHelperSvc()->toStringDetEl(identify())<<
                            " "<<GeoTrf::toString(toAMDB, true)<<", reference point: "<<Amg::toString(reference_point));

            int layer_delta = tubeLayer;
            if (ml == multilayer_t::ML1) layer_delta = m_nlayers + 1 - tubeLayer;

            // Get the As-Built parameters for this ML and side of the chamber
            double zpitch = params->zpitch(ml, tubeSide[isid]);
            double ypitch = params->ypitch(ml, tubeSide[isid]);
            double stagg = (double)params->stagg(ml, tubeSide[isid]);
            double alpha = params->alpha(ml, tubeSide[isid]);
            double y0 = params->y0(ml, tubeSide[isid]);
            double z0 = params->z0(ml, tubeSide[isid]);

            // Find the vector from the reference_point to the endplug
            int do_stagg = (layer_delta - 1) % 2;  // 0 for layer 1 and 3, 1 for layer 2 and 4
            double offset_stagg = ((double)do_stagg) * 0.5 * zpitch * stagg;
            Amg::Vector3D end_plug(0., (tube - 1.0) * zpitch + offset_stagg, (layer_delta - 1) * ypitch);

            const Amg::Transform3D amgTransf{Amg::getRotateX3D(-alpha)};
            end_plug = amgTransf * end_plug;

            // Calculate x position, which varies for endcap chambers
            double xshift = 0.;
            if (isid == 0)
                xshift = 0.5 * getNominalTubeLengthWoCutouts(tubeLayer, tube) - 0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1);
            else
                xshift = -0.5 * getNominalTubeLengthWoCutouts(tubeLayer, tube) + 0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1);

            Amg::Vector3D ret(reference_point.x() + xshift, reference_point.y() + z0 + end_plug.y(),
                              reference_point.z() + y0 + end_plug.z());

            // Warn if result of calculation is too far off
            // BIL1A13 has as-built parameters up to 3mm off, giving the size of the cut
            if ((ret - wireEnd[isid]).mag() > 3. * CLHEP::mm) {
                ATH_MSG_WARNING( "Large as-built correction for chamber " << idHelperSvc()->toStringDetEl(identify()) << ", side "
                    << isid << ", Delta " << Amg::toString(ret - wireEnd[isid]) );
            }
            ATH_MSG_VERBOSE(( tubeSide[isid] == tubeSide_t::POS ? "positive" : "negative")<<" wire end has moved from "
                        <<Amg::toString(wireEnd[isid])<<" to "<<Amg::toString(ret));

            // Save the result
            if (tubeSide[isid] == tubeSide_t::POS)
                locAMDBWireEndP = ret;
            else
                locAMDBWireEndN = ret;
        }
    }
    std::unique_ptr<MdtReadoutElement::GeoInfo> MdtReadoutElement::makeGeoInfo(const int tubeLayer, const int tube) const {
        Amg::Transform3D transform = globalTransform(nodeform_localTubePos(tubeLayer, tube), fromIdealToDeformed(tubeLayer, tube));
        return std::make_unique<GeoInfo>(std::move(transform));
    }

    const MdtReadoutElement::GeoInfo& MdtReadoutElement::geoInfo(const int tubeLayer, const int tube) const {
        size_t itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        if (itube >= m_tubeGeo.size()) {
           ATH_MSG_WARNING(__func__<<"() :"<<__LINE__<<"  called with tubeLayer or tube out of range in chamber "
                << idHelperSvc()->toStringDetEl(identify()) << " : layer " << tubeLayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute transform for first tube in this chamber" );
            ATH_MSG_WARNING( "Please run in DEBUG mode to get extra diagnostic" );
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<GeoInfo>& ptr = m_tubeGeo.at(itube);
        if (!ptr) {
            ptr.set(makeGeoInfo(tubeLayer, tube));
            if (!m_haveTubeGeo) m_haveTubeGeo = true;
        }
        return *ptr;
    }

    const Amg::Transform3D& MdtReadoutElement::transform(const int tubeLayer, const int tube) const {
        return geoInfo(tubeLayer, tube).m_transform;
    }

    const Trk::SaggedLineSurface& MdtReadoutElement::surface(const int tubeLayer, const int tube) const {

        int ntot_tubes = m_nlayers * m_ntubesperlayer;
        int itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        // consistency checks
        if (itube >= ntot_tubes) {
            ATH_MSG_WARNING( "surface called with tubeLayer or tube out of range in chamber "
                << idHelperSvc()->toStringDetEl(identify()) << " : layer " << tubeLayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute surface for first tube in this chamber" );
            ATH_MSG_WARNING( "Please run in DEBUG mode to get extra diagnostic" );
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<Trk::SaggedLineSurface>& ptr = m_tubeSurfaces.at(itube);
        if (!ptr) {
            double wireTension = 350;
            if (getStationIndex() == m_stIdx_BOL) wireTension = 285;
            Identifier id = m_idHelper.channelID(identify(), getMultilayer(), tubeLayer, tube);
            ptr.set(std::make_unique<Trk::SaggedLineSurface>(*this, id, getWireLength(tubeLayer, tube), wireTension, linearDensity));
            if (!m_haveTubeSurfaces) m_haveTubeSurfaces = true;
        }
        return *ptr;
    }
    const Trk::CylinderBounds& MdtReadoutElement::bounds(const int tubeLayer, const int tube) const {
        int istep = 0;
        int ntot_steps = m_nsteps;

        if (hasCutouts() && manager()->MinimalGeoFlag() == 0) {
            ntot_steps = m_nlayers * m_ntubesperlayer;
            istep = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        } else {
            if (endcap()) istep = int((tube - 1) / m_ntubesinastep);

            if (istep < 0 || istep >= ntot_steps) {
                ATH_MSG_WARNING( "bounds for Element named "<< " with tech. " << getTechnologyType()
                    << " DEid = " << idHelperSvc()->toStringDetEl(identify()) << " called with: tubeL, tube " << tubeLayer << " " << tube
                    << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep );
                ATH_MSG_WARNING( "Please run in DEBUG mode to get extra diagnostic; setting istep = 0" );
            }
        }
        if ((unsigned int)istep >= m_tubeBounds.size()) {
            THROW_EXCEPTION(__func__<<"("<<tubeLayer<<","<<tube<<") but m_tubeBounds.size()="<<m_tubeBounds.size()<<" for "<<
                             idHelperSvc()->toStringDetEl(identify()));
        }
        const CxxUtils::CachedUniquePtr<Trk::CylinderBounds>& ptr = m_tubeBounds.at(istep);
        if (!ptr) {
            double tubelength = getTubeLengthForCaching(tubeLayer, tube);
            ptr.set(std::make_unique<Trk::CylinderBounds>(innerTubeRadius(), 0.5 * tubelength - m_deadlength));
            if (!m_haveTubeBounds) m_haveTubeBounds = true;
        }
        return *ptr;
    }
    const Amg::Vector3D& MdtReadoutElement::center(const int tubeLayer, const int tube) const { return geoInfo(tubeLayer, tube).m_center; }
    const Amg::Vector3D& MdtReadoutElement::normal() const {
        if (!m_elemNormal.isValid()) { m_elemNormal.set(transform().linear() * Amg::Vector3D::UnitX()); }
        return *m_elemNormal.ptr();
    }

    const Trk::Surface& MdtReadoutElement::surface() const {
        if (!m_associatedSurface) {
            Amg::RotationMatrix3D muonTRotation(transform().rotation());
            Amg::RotationMatrix3D surfaceTRotation;
            surfaceTRotation.col(0) = muonTRotation.col(1);
            surfaceTRotation.col(1) = muonTRotation.col(2);
            surfaceTRotation.col(2) = muonTRotation.col(0);

            Amg::Transform3D trans3D(surfaceTRotation);
            trans3D.pretranslate(transform().translation());

            if (barrel()) {
                m_associatedSurface.set(std::make_unique<Trk::PlaneSurface>(Amg::Transform3D(trans3D), getSsize() * 0.5,
                                                                            getZsize() * 0.5));
            } else {
                m_associatedSurface.set(std::make_unique<Trk::PlaneSurface>(Amg::Transform3D(trans3D), getSsize() * 0.5,
                                                                            getLongSsize() * 0.5,
                                                                            getRsize() * 0.5));
            }
        }
        return *m_associatedSurface;
    }

    const Amg::Vector3D& MdtReadoutElement::center() const { return surface().center(); }

    const Trk::SurfaceBounds& MdtReadoutElement::bounds() const {
        if (!m_associatedBounds) {
            if (barrel()) {
                m_associatedBounds.set(
                    std::make_unique<Trk::RectangleBounds>(getSsize() / 2., getZsize() / 2.));
            } else {
                m_associatedBounds.set(std::make_unique<Trk::TrapezoidBounds>(
                    getSsize() / 2., getLongSsize() / 2., getRsize() / 2.));
            }
        }
        return *m_associatedBounds;
    }
    void MdtReadoutElement::clearCache() {
        ATH_MSG_DEBUG( "Clearing cache for ReadoutElement " << idHelperSvc()->toStringDetEl(identify()) );
        if (m_associatedSurface) {
            m_associatedSurface.release();
        }
        else  ATH_MSG_VERBOSE( "no associated surface to be deleted" );

        if (m_associatedBounds) {
            m_associatedBounds.release();
        } ATH_MSG_VERBOSE( "no associated bounds to be deleted" );
        m_elemNormal.reset();
        if (m_haveTubeSurfaces) {
            m_haveTubeSurfaces = false;
            for (auto& s : m_tubeSurfaces) { s.release(); }
        }
        if (m_haveTubeGeo) {
            m_haveTubeGeo = false;
            for (auto& g : m_tubeGeo) { g.release(); }
        }
        if (m_haveTubeBounds) {
            m_haveTubeBounds = false;
            for (auto& b : m_tubeBounds) { b.release(); }
        }
        // reset here the deform-related transforms        
        if (m_haveDeformTransf) {
            m_haveDeformTransf = false;
            for (auto& d : m_deformTransf) { d.release(); }
        }
    }

    void MdtReadoutElement::setBLinePar(const BLinePar* bLine) {
        ATH_MSG_DEBUG( "Setting B-line for " << idHelperSvc()->toStringDetEl(identify()) );
        if (m_BLinePar == bLine) {
            return;
        }
        m_BLinePar = bLine;
        refreshCache();
    }

    void MdtReadoutElement::fillCache() {
        ATH_MSG_DEBUG( "Filling cache for ReadoutElement " << idHelperSvc()->toStringDetEl(identify()));

        const Trk::PlaneSurface* tmpSurface = dynamic_cast<const Trk::PlaneSurface*>(&surface());  //<! filling m_associatedSurface
        const Trk::SurfaceBounds* tmpBounds = nullptr;                         //<! filling m_associatedBounds
        if (barrel())
            tmpBounds = dynamic_cast<const Trk::RectangleBounds*>(&bounds());
        else
            tmpBounds = dynamic_cast<const Trk::TrapezoidBounds*>(&bounds());
        ATH_MSG_VERBOSE( "global Surface / Bounds pointers " << tmpSurface << " " << tmpBounds );
        ATH_MSG_VERBOSE( "global Normal " << normal() );

        const Trk::CylinderBounds* tmpCil = nullptr;
        const Trk::SaggedLineSurface* tmpSaggL = nullptr;
        Amg::Vector3D myPoint{Amg::Vector3D::Zero()};
        Amg::Transform3D myTransform{Amg::Transform3D::Identity()};
        for (int tl = 1; tl <= getNLayers(); ++tl) {
            for (int tube = 1; tube <= getNtubesperlayer(); ++tube) {
                // in case of BMG chambers, do not check the 'dead' tubes
                // (the tubes are numbered from 1-54 for each however there are cutouts for the
                // alignment system where no tubes are built-in, meaning, those tubes do not exist/are 'dead')
                if (manager()->mdtIdHelper()->isBMG(identify())) {
                    PVConstLink cv = getMaterialGeom();
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tl;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                myTransform = transform(tl, tube);                                           //<! filling m_tubeTransf
                                myPoint = center(tl, tube);                                                  //<! filling m_tubeCenter
                                tmpCil = dynamic_cast<const Trk::CylinderBounds*>(&bounds(tl, tube));        //<! filling m_tubeBounds
                                tmpSaggL = dynamic_cast<const Trk::SaggedLineSurface*>(&surface(tl, tube));  //<! filling m_tubeSurfaces
                                found = true;
                            }
                        },
                        &*cv);
                    if (found) {
                        ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " transform at origin  "
                            << Amg::toString(myTransform.linear()) );
                        ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube center          " << myPoint );
                        ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube bounds pointer  " << tmpCil );
                        ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube surface pointer " << tmpSaggL );
                    }
                } else {
                    // print in order to compute !!!
                    myTransform = transform(tl, tube);                                           //<! filling m_tubeTransf
                    myPoint = center(tl, tube);                                                  //<! filling m_tubeCenter
                    tmpCil = dynamic_cast<const Trk::CylinderBounds*>(&bounds(tl, tube));        //<! filling m_tubeBounds
                    tmpSaggL = dynamic_cast<const Trk::SaggedLineSurface*>(&surface(tl, tube));  //<! filling m_tubeSurfaces
                    ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " transform at origin  "
                                    << Amg::toString(myTransform.translation()) );
                    ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube center          " << myPoint );
                    ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube bounds pointer  " << tmpCil );
                    ATH_MSG_VERBOSE( "tubeLayer/tube " << tl << " " << tube << " tube surface pointer " << tmpSaggL );
                }
            }
        }
    }

    bool MdtReadoutElement::containsId(const Identifier& id) const {
        if (idHelperSvc()->detElId(id) != identify()) return false;
        int layer = m_idHelper.tubeLayer(id);
        if (layer < 1 || layer > getNLayers()) return false;
        int tube = m_idHelper.tube(id);
        return tube >= 1 && tube <= getNtubesperlayer();
    }

    // **************************** interfaces related to Tracking *****************************************************)

}  // namespace MuonGM
