/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The MM detector = an assembly module = STGC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/MMReadoutElement.h"

#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoShape.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoDefinitions.h>
#include <cstdlib>

#include <cmath>
#include <memory>
#include <utility>

#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelHelpers/getChildNodesWithTrf.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelHelpers/TransformToStringConverter.h"

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoTrd.h"
#include "Identifier/IdentifierHash.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"

#include "GaudiKernel/ISvcLocator.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"


#define THROW_EXCEPTION_MM(MSG)                                                                            \
     {                                                                                                  \
        std::stringstream sstr{};                                                                       \
        sstr<<"MMReadoutElement - "<<idHelperSvc()->toStringDetEl(identify())<<" "<<__LINE__<<": ";   \
        sstr<<MSG;                                                                                      \
        throw std::runtime_error(sstr.str());                                                           \
     }                                                                                                  \

namespace {
    template <class ObjType, size_t N> void assign(const std::vector<ObjType>& in, 
                                                  std::array<ObjType, N>& out) {
        for (size_t k =0 ; k < std::min(in.size(), N) ; ++k){
            out[k] = in[k];
        }
    }
}

using namespace GeoStrUtils;
namespace MuonGM {

    //============================================================================
    MMReadoutElement::MMReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, MuonDetectorManager* mgr) 
    : MuonClusterReadoutElement{pv,mgr, Trk::DetectorElemType::MM},
      m_ml(mL) {
      
        std::string fixName = (stName[2] == 'L') ? "MML" : "MMS";
        setStationName(fixName);       
        setChamberLayer(mL);
        Identifier id = mgr->mmIdHelper()->channelID(fixName, zi, fi, mL, 1, 1);
        setIdentifier(id);


        if (mgr->MinimalGeoFlag()) {
            return;
        } 
        bool foundShape = false;   
        const PVConstLink pvc {getMaterialGeom()};
        const GeoTrd* trd=dynamic_cast<const GeoTrd *> (pvc->getLogVol()->getShape());
        if (trd) {
            setSsize(2*trd->getYHalfLength1());
            setLongSsize( 2*trd->getYHalfLength2());
            setRsize(2*trd->getZHalfLength());
            setZsize(trd->getXHalfLength1());

        } else {
            ATH_MSG_DEBUG("Expected a GeoTrd but got "<<printGeoShape(pvc->getLogVol()->getShape()));
        }

        std::vector<GeoChildNodeWithTrf> children{getAllSubVolumes(pvc)};
        for (const GeoChildNodeWithTrf& child : children) {
            ATH_MSG_VERBOSE("Child node "<<child.nodeName<<" "<<child.volume->getLogVol()->getName()); 
            if (child.volume->getLogVol()->getName().find("Sensitive") == std::string::npos &&
                /// Active gas volume in the R4 like description
                child.volume->getLogVol()->getName() != "actMicroMegaGas") {
                continue;
            }
            ++m_nlayers;
            if (m_nlayers > 4) {
                THROW_EXCEPTION_MM("number of MM layers > 4: increase transform array size" );
            }
            m_Xlg[m_nlayers - 1] =  child.transform;
            // save layer dimensions
            if (foundShape) {
                continue;
            }
            const GeoShape* childShape = child.volume->getLogVol()->getShape();
            while (childShape->typeID() != GeoTrd::getClassTypeID()){
                auto [opA, opB] = getOps(childShape);
                ATH_MSG_VERBOSE("Operands are "<<printGeoShape(opA)<<", "<<printGeoShape(opB));
                childShape = opA;
            } 
            const GeoTrd* trd = dynamic_cast<const GeoTrd*>(childShape);
            m_halfX = trd->getZHalfLength();
            // adjust phi dimensions according to the active area
            m_minHalfY = trd->getYHalfLength1();
            m_maxHalfY = trd->getYHalfLength2();
            foundShape = true;
        }
    
        if (!foundShape) {
            THROW_EXCEPTION_MM(" failed to initialize dimensions of this chamber " );
        }
    }


    //============================================================================
    MMReadoutElement::~MMReadoutElement() = default;

    //============================================================================
    void MMReadoutElement::initDesignSqLite(){
      
        SmartIF<IGeoDbTagSvc> geoDbTag{Gaudi::svcLocator()->service("GeoDbTagSvc")};
        if (!geoDbTag) {
            THROW_EXCEPTION_MM("Could not locate GeoDbTagSvc");
        }
        SmartIF<IRDBAccessSvc> accessSvc{Gaudi::svcLocator()->service(geoDbTag->getParamSvcName())};
        if (!accessSvc) {            
            THROW_EXCEPTION_MM("Could not locate " << geoDbTag->getParamSvcName() );
        }
        const char sector_l = getStationName()[2];
        IRDBRecordset_ptr wmmRec = accessSvc->getRecordsetPtr("WMM","","");
        for (unsigned int ind = 0; ind < wmmRec->size(); ind++) {
            std::string WMM_TYPE       = (*wmmRec)[ind]->getString("WMM_TYPE");               
            if (sector_l != WMM_TYPE[4]){ 
                continue;
            }
            if (std::abs(getStationEta())!=(int) (WMM_TYPE[6]-'0')) {
                continue;
            }
            if (m_ml != (int) (WMM_TYPE[12]-'0')){
                continue;
            }
            const double Tck = (*wmmRec)[ind]->getDouble("Tck");                    
            const double activeBottomLength = (*wmmRec)[ind]->getDouble("activeBottomLength");     
            const double activeH = (*wmmRec)[ind]->getDouble("activeH");                
            const double activeTopLength = (*wmmRec)[ind]->getDouble("activeTopLength");        
            const double gasTck = (*wmmRec)[ind]->getDouble("gasTck");                 
            const int nMissedBottomEta       = (*wmmRec)[ind]->getInt("nMissedBottomEta");          
            const int nMissedBottomStereo    = (*wmmRec)[ind]->getInt("nMissedBottomStereo");       
            const int nMissedTopEta          = (*wmmRec)[ind]->getInt("nMissedTopEta");             
            const int nMissedTopStereo       = (*wmmRec)[ind]->getInt("nMissedTopStereo");          
            
            assign(tokenizeInt((*wmmRec)[ind]->getString("readoutSide"), ";"), m_readoutSide); 
            const std::vector<double> stereoAngle{tokenizeDouble((*wmmRec)[ind]->getString("stereoAngle"), ";")};
            const double stripPitch = (*wmmRec)[ind]->getDouble("stripPitch");             
            const int totalStrips = (*wmmRec)[ind]->getInt   ("totalStrips");            
            const double ylFrame = (*wmmRec)[ind]->getDouble("ylFrame");                
            const double ysFrame = (*wmmRec)[ind]->getDouble("ysFrame");                
        
            setZsize(Tck);                                      // thickness (full chamber)
            m_halfX         = activeH / 2;                      // 0.5*radial_size (active area)
            m_minHalfY      = activeBottomLength / 2;           // 0.5*bottom length (active area)
            m_maxHalfY      = activeTopLength / 2;              // 0.5*top length (active area)
            m_offset        = -0.5*(ylFrame - ysFrame);         // radial dist. of active area center w.r.t. chamber center
            ATH_MSG_DEBUG(idHelperSvc()->toStringDetEl(identify())<<", ylFrame: "<<ylFrame<<", ysFrame: "<<ysFrame<<", offset: "<<m_offset);
            for (int il = 0; il < m_nlayers; il++) {
                // identifier of the first channel to retrieve max number of strips
                Identifier id = m_idHelper.channelID(identify(), m_ml, il + 1, 1);
                int chMax = m_idHelper.channelMax(id);
                if (chMax < 0) {
                    THROW_EXCEPTION_MM("MMReadoutElement -- Max number of strips not a valid value" );
                }
                MuonChannelDesign& design = m_etaDesign[il];
          
                design.type = MuonChannelDesign::ChannelType::etaStrip;
                design.detType = MuonChannelDesign::DetType::MM;
                design.inputPitch = stripPitch;
                design.thickness = gasTck; 
                design.nMissedTopEta = nMissedTopEta;  // #of eta strips that are not connected to any FE board
                design.nMissedBottomEta = nMissedBottomEta;
                design.nMissedTopStereo = nMissedTopStereo;  // #of stereo strips that are not connected to any FE board
                design.nMissedBottomStereo = nMissedBottomStereo;
                design.totalStrips = totalStrips;   
                /// The stereo angle is defined clock-wise from the y-axis
                design.defineTrapezoid(m_minHalfY, m_maxHalfY,m_halfX, stereoAngle[il]);
                /// Input width is defined as the distance between two channels
                design.inputWidth = stripPitch * std::cos(design.stereoAngle());
          
                if (!design.hasStereoAngle()) {  // eta layers
                    design.nch  = design.totalStrips - design.nMissedBottomEta - design.nMissedTopEta;
                    design.setFirstPos(-0.5 * design.xSize() + stripPitch);
                } else {  // stereo layers
                    design.nch = design.totalStrips - design.nMissedBottomStereo - design.nMissedTopStereo;
                    design.setFirstPos( -0.5 * design.xSize() + (1 + design.nMissedBottomStereo - design.nMissedBottomEta) * stripPitch);
                }
                ATH_MSG_DEBUG("initDesign:" <<idHelperSvc()->toStringDetEl(identify())<< " layer " << il 
                           << ", strip pitch " << design.inputPitch << ", nstrips " << design.nch 
                           << " stereo " << design.stereoAngle() / Gaudi::Units::degree
                        <<", "<<design.xSize()<<", "<<design.maxYSize()<<" "<<design.minYSize()
                        <<", firstPos: "<<design.firstPos()<<", first pitch: "<<design.firstPitch);
            }
        }
    }
    
    
    void MMReadoutElement::initDesign() {
        if (m_ml < 1 || m_ml > 2) {
           THROW_EXCEPTION_MM("MMReadoutElement -- Unexpected Multilayer: m_ml= " << m_ml );
           return;
       }
       // Get the detector configuration.
       SmartIF<IGeoDbTagSvc> geoDbTag{Gaudi::svcLocator()->service("GeoDbTagSvc")};
       if (!geoDbTag) {
            THROW_EXCEPTION_MM("Could not locate GeoDbTagSvc");
       }
       if (geoDbTag->getSqliteReader()) {
            initDesignSqLite();
            return;
       }
       char side     = getStationEta() < 0 ? 'C' : 'A';
       char sector_l = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
       // Initialize from database:
  
       MMDetectorHelper aHelper;
       MMDetectorDescription* mm   = aHelper.Get_MMDetector(sector_l, std::abs(getStationEta()), getStationPhi(), m_ml, side);
       MMReadoutParameters roParam = mm->GetReadoutParameters();
      
       double ylFrame  = mm->ylFrame();
       double ysFrame  = mm->ysFrame();
       double pitch    = roParam.stripPitch;
       
       
       
       
       setSsize(mm->sWidth());                           // bottom base length (full chamber)
       setLongSsize(mm->lWidth());                       // top base length (full chamber)
       setRsize(mm->Length());                           // height of the trapezoid (full chamber)
       setZsize(mm->Tck());                              // thickness (full chamber)
       m_halfX         = roParam.activeH / 2;            // 0.5*radial_size (active area)
       m_minHalfY      = roParam.activeBottomLength / 2; // 0.5*bottom length (active area)
       m_maxHalfY      = roParam.activeTopLength / 2;    // 0.5*top length (active area)
       m_offset        = -0.5*(ylFrame - ysFrame);       // radial dist. of active area center w.r.t. chamber center
       ATH_MSG_DEBUG(idHelperSvc()->toStringDetEl(identify())<<", ylFrame: "<<ylFrame<<", ysFrame: "<<ysFrame<<", offset: "<<m_offset);
       assign(roParam.readoutSide, m_readoutSide);
      
       for (int il = 0; il < m_nlayers; il++) {
            // identifier of the first channel to retrieve max number of strips
            Identifier id = m_idHelper.channelID(identify(), m_ml, il + 1, 1);
            int chMax = m_idHelper.channelMax(id);
            if (chMax < 0) {
                THROW_EXCEPTION_MM("MMReadoutElement -- Max number of strips not a valid value" );
            }
            MuonChannelDesign& design = m_etaDesign[il];
        
            design.type                = MuonChannelDesign::ChannelType::etaStrip;
            design.detType             = MuonChannelDesign::DetType::MM;
            design.inputPitch          = pitch;
            design.thickness           = roParam.gasThickness; 
            design.nMissedTopEta       = roParam.nMissedTopEta;  // #of eta strips that are not connected to any FE board
            design.nMissedBottomEta    = roParam.nMissedBottomEta;
            design.nMissedTopStereo    = roParam.nMissedTopStereo;  // #of stereo strips that are not connected to any FE board
            design.nMissedBottomStereo = roParam.nMissedBottomStereo;
            design.totalStrips         = roParam.tStrips;   
            /// The stereo angle is defined clock-wise from the y-axis
            design.defineTrapezoid(m_minHalfY, m_maxHalfY,m_halfX, - roParam.stereoAngle.at(il));
            /// Input width is defined as the distance between two channels
            design.inputWidth          = pitch * std::cos(design.stereoAngle());
        
            if (!design.hasStereoAngle()) {  // eta layers
                design.nch = design.totalStrips - design.nMissedBottomEta - design.nMissedTopEta;
                design.setFirstPos(-0.5 * design.xSize() + pitch);
            } else {  // stereo layers
                design.nch = design.totalStrips - design.nMissedBottomStereo - design.nMissedTopStereo;
                design.setFirstPos( -0.5 * design.xSize() + 
                                   (1 + design.nMissedBottomStereo - design.nMissedBottomEta) * pitch);
            }
            ATH_MSG_DEBUG("initDesign:" <<idHelperSvc()->toStringDetEl(identify())<< " layer " << il 
            << ", strip pitch " << design.inputPitch << ", nstrips " << design.nch 
            << " stereo " << design.stereoAngle() / Gaudi::Units::degree
            <<", "<<design.xSize()<<", "<<design.maxYSize()<<" "<<design.minYSize()
            <<", firstPos: "<<design.firstPos()<<", first pitch: "<<design.firstPitch);
        }
    }


    //============================================================================
    void MMReadoutElement::fillCache() {
      
        if (m_surfaceData) {
            ATH_MSG_WARNING("calling fillCache on an already filled cache" );
            return;
        }
        m_surfaceData = std::make_unique<SurfaceData>();


        for (int layer = 0; layer < m_nlayers; ++layer) {
            // identifier of the first channel
            Identifier id = m_idHelper.channelID(identify(), m_ml, layer + 1, 1);
            const double sAngle = m_etaDesign[layer].stereoAngle();
            m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this,id));
            m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RotatedTrapezoidBounds>(m_halfX, m_minHalfY, m_maxHalfY, sAngle));
      
            m_surfaceData->m_layerTransforms.push_back(
                             absTransform()                         // transformation from chamber to ATLAS frame
                             * m_delta                              // rotations (a-lines) from the alignment group
                             * m_Xlg[layer]                         // x-shift of the gas-gap center w.r.t. quadruplet center
                             * Amg::getTranslateZ3D(m_offset) // z-shift to volume center
                             * Amg::getRotateY3D(-90. * CLHEP::deg) // x<->z because of GeoTrd definition
                             * Amg::getRotateZ3D(sAngle)); 
      
            // surface info (center, normal)
            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * (-Amg::Vector3D::UnitZ()));
      
            ATH_MSG_DEBUG("MMReadoutElement layer " << layer << " sAngle " << sAngle << " phi direction MM eta strip "
                        << (m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D::UnitY()).phi() );
        }
    }


    //============================================================================
    bool MMReadoutElement::containsId(const Identifier& id) const {
        if (m_idHelper.stationEta(id) != getStationEta()) return false;
        if (m_idHelper.stationPhi(id) != getStationPhi()) return false;

        if (m_idHelper.multilayer(id) != m_ml) return false;

        int gasgap = m_idHelper.gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        int strip = m_idHelper.channel(id);
        return strip >= 1 && strip <= m_etaDesign[gasgap - 1].totalStrips;
    }


    //============================================================================
    Amg::Vector3D MMReadoutElement::localToGlobalCoords(const Amg::Vector3D& locPos, const Identifier& id) const {
        int gg = m_idHelper.gasGap(id);
        //const MuonChannelDesign* design = getDesign(id);
        Amg::Vector3D locPos_ML = (m_Xlg[gg - 1]) * Amg::getTranslateZ3D(m_offset) * 
                                 //   (design->hasStereoAngle() ?  
                                 //    Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D::UnitY()) * Amg::AngleAxis3D(design->stereoAngle(), Amg::Vector3D::UnitZ())  *
                                 //    Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D::UnitY())  : AmgSymMatrix(3)::Identity())*
                                  locPos;
       
        ATH_MSG_DEBUG("position coordinates in the gas-gap r.f.:    "  << Amg::toString(locPos) );
        ATH_MSG_DEBUG("position coordinates in the multilayer r.f.: " <<  Amg::toString(locPos_ML) );
        return absTransform() * m_delta * locPos_ML;
    }


    //============================================================================
    void MMReadoutElement::setDelta(const ALinePar& aline) {
        // amdb frame (s, z, t) = chamber frame (y, z, x)
        if (aline) {
            const Amg::Transform3D permute{GeoTrf::GeoRotation{90.*Gaudi::Units::deg,90.*Gaudi::Units::deg, 0.}};
            // The origin of the rotation axes is at the center of the active area 
            // in the z (radial) direction. Account for this shift in the definition 
            // of m_delta so that it can be applied on chamber frame coordinates.
            m_ALinePar  = &aline;
            m_delta     = Amg::getTranslateZ3D(m_offset)* permute*aline.delta()*
                          permute.inverse()*Amg::getTranslateZ3D(-m_offset);
            ATH_MSG_DEBUG(idHelperSvc()->toStringDetEl(identify())<<" setup new alignment: "<<GeoTrf::toString(m_delta,true));
            refreshCache();
        } else {  
            clearALinePar();
        }
    }
    //============================================================================
    void MMReadoutElement::clearALinePar() {
        if (has_ALines()) {
            m_ALinePar = nullptr; 
            m_delta = Amg::Transform3D::Identity(); 
            refreshCache();
        }
    }

    //============================================================================
    void MMReadoutElement::setBLinePar(const BLinePar& bLine) {
        ATH_MSG_VERBOSE("Setting B-line for " << idHelperSvc()->toStringDetEl(identify())<<" "<<bLine);
        m_BLinePar = &bLine;
    }
    //============================================================================
    void MMReadoutElement::posOnDefChamber(Amg::Vector3D& locPosML) const {

        // note: amdb frame (s, z, t) = chamber frame (y, z, x)
        if (!has_BLines()) return;

        double t0    = locPosML.x();
        double s0    = locPosML.y();
        double z0    = locPosML.z();
        double width = getSsize() + (getLongSsize() - getSsize())*(z0/getRsize() + 0.5); // because z0 is in [-length/2, length/2]

        double s_rel = s0/(width/2.);           // in [-1, 1]
        double z_rel = z0/(getRsize()/2.); // in [-1, 1]
        double t_rel = t0/(getZsize()/2.);    // in [-1, 1]

        // b-line parameters
        using Parameter = BLinePar::Parameter;
        double bp    = m_BLinePar->getParameter(Parameter::bp);
        double bn    = m_BLinePar->getParameter(Parameter::bn);
        double sp    = m_BLinePar->getParameter(Parameter::sp);
        double sn    = m_BLinePar->getParameter(Parameter::sn);
        double tw    = m_BLinePar->getParameter(Parameter::tw);
        double eg    = m_BLinePar->getParameter(Parameter::eg)*1.e-3;
        double ep    = m_BLinePar->getParameter(Parameter::ep)*1.e-3;
        double en    = m_BLinePar->getParameter(Parameter::en)*1.e-3;

        double ds{0.}, dz{0.}, dt{0.};

        if (bp != 0 || bn != 0)
            dt += 0.5*(s_rel*s_rel - 1)*((bp + bn) + (bp - bn)*z_rel);

        if (sp != 0 || sn != 0)
            dt += 0.5*(z_rel*z_rel - 1)*((sp + sn) + (sp - sn)*s_rel);

        if (tw != 0) {
            dt -= tw*s_rel*z_rel;
            dz += tw*s_rel*t_rel*getZsize()/getRsize();
        }

        if (eg != 0) {
            dt += t0*eg;
            ds += s0*eg;
            dz += z0*eg;
        }

        if (ep != 0 || en != 0) {
            // the formulas below differ from those in Christoph's talk
            // because are origin for NSW is at the center of the chamber, 
            // whereas in the talk (i.e. MDTs), it is at the bottom!
            double delta = s_rel*s_rel * ((ep + en)*s_rel/6 + (ep - en)/4);
            double phi   = s_rel * ((ep + en)*s_rel + (ep - en)) / 2;
            dt += phi*t0;
            ds += delta*width/2;
            dz += phi*z0;
        }

        locPosML[0] += dt;
        locPosML[1] += ds;
        locPosML[2] += dz;
    }


    //============================================================================
    bool MMReadoutElement::spacePointPosition(const Identifier& layerId, const Amg::Vector2D& lpos, Amg::Vector3D& pos) const {

        pos = Amg::Vector3D(lpos.x(), lpos.y(), 0.);

        const MuonChannelDesign* design = getDesign(layerId);
        if (!design) {
            ATH_MSG_WARNING("Unable to get MuonChannelDesign, therefore cannot provide position corrections. Returning." );
            return false;
        }

        bool conditionsApplied{false};
        Amg::Transform3D trfToML{Amg::Transform3D::Identity()};

#ifndef SIMULATIONBASE
        //*********************
        // As-Built (MuonNswAsBuilt is not included in AthSimulation)
        //*********************
        const NswAsBuilt::StripCalculator* sc = manager()->getMMAsBuiltCalculator();
        if (sc) {

            // express the local position w.r.t. the nearest active strip
            Amg::Vector2D rel_pos;
            int istrip = design->positionRelativeToStrip(lpos, rel_pos); 
            if (istrip < 0) {
                ATH_MSG_WARNING("As-built corrections are provided only within the active area. Returning." );                
                return false;
            } 

            // setup strip calculator
            NswAsBuilt::stripIdentifier_t strip_id;
            strip_id.quadruplet = { (largeSector() ? NswAsBuilt::quadrupletIdentifier_t::MML : NswAsBuilt::quadrupletIdentifier_t::MMS), getStationEta(), getStationPhi(), m_ml };
            strip_id.ilayer     = m_idHelper.gasGap(layerId);
            strip_id.istrip     = istrip;

            // get the position coordinates, in the chamber frame, from NswAsBuilt.
            // Applying a 2.75mm correction along the layer normal, since NswAsBuilt considers the layer 
            // on the readout strips, whereas Athena wants it at the middle of the drift gap.
            NswAsBuilt::StripCalculator::position_t calcPos = sc->getPositionAlongStrip(NswAsBuilt::Element::ParameterClass::CORRECTION, strip_id, rel_pos.y(), rel_pos.x());
            
            if (calcPos.isvalid == NswAsBuilt::StripCalculator::IsValid::VALID) {
                pos     = calcPos.pos;
                pos[0] += strip_id.ilayer%2 ? -2.75 : 2.75;
            
                // signal that pos is now in the chamber reference frame
                // (don't go back to the layer frame yet, since we may apply b-lines later on)
                trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);
                conditionsApplied = true;
            } else {
                ATH_MSG_DEBUG( "No as-built corrections provided for "<<idHelperSvc()->toStringDetEl(identify())<<" layer: "<<strip_id.ilayer);
            }
        }
#endif 

        //*********************
        // B-Lines
        //*********************
        if (has_BLines()) {
          // go to the multilayer reference frame if we are not already there
          if (!conditionsApplied) {
             trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);
             pos = trfToML*pos;
             
             // signal that pos is now in the multilayer reference frame
             conditionsApplied = true; 
          }
          posOnDefChamber(pos);
       }

        // back to the layer reference frame from where we started
        if (conditionsApplied) pos = trfToML.inverse()*pos;
        
        return true;
    }
    
}  // namespace MuonGM
