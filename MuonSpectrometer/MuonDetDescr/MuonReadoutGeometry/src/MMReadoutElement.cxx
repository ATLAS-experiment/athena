/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The MM detector = an assembly module = STGC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/MMReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoShape.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <cstdlib>

#include <cmath>
#include <memory>
#include <utility>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
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

namespace Trk {
    class SurfaceBounds;
}
namespace MuonGM {

    //============================================================================
    MMReadoutElement::MMReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, MuonDetectorManager* mgr, const NswPassivationDbData* passivData) 
    : MuonClusterReadoutElement(pv,mgr, Trk::DetectorElemType::MM),
      m_passivData(passivData),
      m_ml(mL) {
      

        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        std::string fixName = (stName[2] == 'L') ? "MML" : "MMS";
        setStationName(fixName);       
        setChamberLayer(mL);
        Identifier id = mgr->mmIdHelper()->channelID(fixName, zi, fi, mL, 1, 1);
        setIdentifier(id);
        bool foundShape = false;

        if (!mgr->MinimalGeoFlag()) {
            if (GeoFullPhysVol* pvc = dynamic_cast<GeoFullPhysVol*>(pv)) {
                const GeoLogVol *lvol=pvc->getLogVol();
                const GeoShape  *shape=lvol->getShape();
                const GeoTrd    *trd=dynamic_cast<const GeoTrd *> (shape);
                if (trd) {
                m_sWidthChamber = 2*trd->getYHalfLength1();       // bottom base length (full chamber)
                m_lWidthChamber = 2*trd->getYHalfLength2();       // top base length (full chamber)
                m_lengthChamber = 2*trd->getZHalfLength();        // height of the trapezoid (full chamber)         
            }
        

                unsigned int nchildvol = pvc->getNChildVols();
                int llay = 0;
                std::string::size_type npos;
                for (unsigned ich = 0; ich < nchildvol; ++ich) {
                    PVConstLink pc = pvc->getChildVol(ich);
                    std::string childname = (pc->getLogVol())->getName();
                    if ((npos = childname.find("Sensitive")) != std::string::npos) {
                        ++llay;
                        if (llay > 4) {
                            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                            log << MSG::WARNING << "number of MM layers > 4: increase transform array size" << endmsg;
                            continue;
                        }
                        m_Xlg[llay - 1] = pvc->getXToChildVol(ich);
                        // save layer dimensions
                        if (llay == 1) {
                            if (pc->getLogVol()->getShape()->type() == "Trd") {
                                const GeoTrd* trd = dynamic_cast<const GeoTrd*>(pc->getLogVol()->getShape());
                                m_halfX = trd->getZHalfLength();
                                // adjust phi dimensions according to the active area
                                m_minHalfY = trd->getYHalfLength1();
                                m_maxHalfY = trd->getYHalfLength2();
                                foundShape = true;
                            } else if (pc->getLogVol()->getShape()->type() == "Subtraction") {
                                const GeoShapeSubtraction* sh = dynamic_cast<const GeoShapeSubtraction*>(pc->getLogVol()->getShape());
                                const GeoShape* sub = sh->getOpA();

                                while (sub->type() == "Subtraction") {
                                    sh = dynamic_cast<const GeoShapeSubtraction*>(sub);
                                    sub = sh->getOpA();
                                }
                                const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sub);

                                if (!trd) {
                                    MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                                    log << MSG::WARNING << "MM layer base shape not trapezoid? " << sub->type() << endmsg;
                                } else {
                                    m_halfX = trd->getZHalfLength();
                                    m_minHalfY = trd->getYHalfLength1();
                                    m_maxHalfY = trd->getYHalfLength2();
                                    foundShape = true;
                                }
                            } else {
                                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                                log << MSG::WARNING << "MM layer shape not recognized:" << pc->getLogVol()->getShape()->type() << endmsg;
                            }
                        }
                    }
                }
                m_nlayers = llay;
            } else {
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                log << MSG::WARNING << "Cannot perform a dynamic cast ! " << endmsg;
            }
        }
        if (!foundShape) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << " failed to initialize dimensions of this chamber " << endmsg;
        }
    }


    //============================================================================
    MMReadoutElement::~MMReadoutElement() { clearCache(); }

    //============================================================================
    void MMReadoutElement::initDesign() {
       m_etaDesign.clear();
       m_etaDesign.resize(m_nlayers);
       
       // Get the detector configuration.
       
       MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
       ISvcLocator* svcLocator = Gaudi::svcLocator(); // from Bootstrap
       IGeoDbTagSvc* geoDbTag{nullptr};
       StatusCode sc = svcLocator->service("GeoDbTagSvc",geoDbTag);
       if (sc.isFailure()) log << MSG::FATAL << "Could not locate GeoDbTagSvc" << endmsg;
       GeoModelIO::ReadGeoModel* sqliteReader = geoDbTag->getSqliteReader();

       
       if (m_ml < 1 || m_ml > 2) {
     MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
     log << MSG::WARNING << "MMReadoutElement -- Unexpected Multilayer: m_ml= " << m_ml << endmsg;
     return;
       }
        
       char side     = getStationEta() < 0 ? 'C' : 'A';
       char sector_l = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
     // Initialize from database:
    if (sqliteReader) {
      IRDBAccessSvc *accessSvc{nullptr};
      StatusCode sc=svcLocator->service(geoDbTag->getParamSvcName(),accessSvc);
      if (sc.isFailure()) log << MSG::FATAL << "Could not locate " << geoDbTag->getParamSvcName() << endmsg;
      IRDBRecordset_ptr wmmRec = accessSvc->getRecordsetPtr("WMM","","");
      for (unsigned int ind = 0; ind < wmmRec->size(); ind++) {
        std::string WMM_TYPE       = (*wmmRec)[ind]->getString("WMM_TYPE");               
        if (sector_l != WMM_TYPE[4])                       continue;
        if (abs(getStationEta())!=(int) (WMM_TYPE[6]-'0')) continue;
        if (m_ml != (int) (WMM_TYPE[12]-'0'))              continue;

        double Tck                 = (*wmmRec)[ind]->getDouble("Tck");                    
        double activeBottomLength  = (*wmmRec)[ind]->getDouble("activeBottomLength");     
        double activeH             = (*wmmRec)[ind]->getDouble("activeH");                
        double activeTopLength     = (*wmmRec)[ind]->getDouble("activeTopLength");        
        double gasTck              = (*wmmRec)[ind]->getDouble("gasTck");                 
        int nMissedBottomEta       = (*wmmRec)[ind]->getInt("nMissedBottomEta");          
        int nMissedBottomStereo    = (*wmmRec)[ind]->getInt("nMissedBottomStereo");       
        int nMissedTopEta          = (*wmmRec)[ind]->getInt("nMissedTopEta");             
        int nMissedTopStereo       = (*wmmRec)[ind]->getInt("nMissedTopStereo");          
        std::string  readoutSide   = (*wmmRec)[ind]->getString("readoutSide");            
        std::string  stereoAngle   = (*wmmRec)[ind]->getString("stereoAngle");            
        double       stripPitch    = (*wmmRec)[ind]->getDouble("stripPitch");             
        int          totalStrips   = (*wmmRec)[ind]->getInt   ("totalStrips");            
        double       ylFrame       = (*wmmRec)[ind]->getDouble("ylFrame");                
        double       ysFrame       = (*wmmRec)[ind]->getDouble("ysFrame");                
        
        m_tckChamber    = Tck;                              // thickness (full chamber)
        m_halfX         = activeH / 2;                      // 0.5*radial_size (active area)
        m_minHalfY      = activeBottomLength / 2;           // 0.5*bottom length (active area)
        m_maxHalfY      = activeTopLength / 2;              // 0.5*top length (active area)
        m_offset        = -0.5*(ylFrame - ysFrame);         // radial dist. of active area center w.r.t. chamber center
        std::replace(stereoAngle.begin(),stereoAngle.end(), ';',' ');
        std::istringstream stereoStream(stereoAngle);

	// Initialize the "m_readoutSide" vector
	{
	  std::replace(readoutSide.begin(),readoutSide.end(),';',' ');
	  std::istringstream stream(readoutSide);
	  int iSide;
	  while (stream>>iSide) m_readoutSide.push_back(iSide);
	}
	
        for (int il = 0; il < m_nlayers; il++) {
          double stereoAngleIL{0};
          stereoStream >> stereoAngleIL;

          // identifier of the first channel to retrieve max number of strips
          Identifier id = manager()->mmIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, il + 1, 1);
          int chMax = manager()->mmIdHelper()->channelMax(id);
          if (chMax < 0) {
        chMax = 2500;
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
        log << MSG::WARNING << "MMReadoutElement -- Max number of strips not a valid value" << endmsg;
          }
          MuonChannelDesign& design = m_etaDesign[il];
          
          design.type                = MuonChannelDesign::ChannelType::etaStrip;
          design.detType             = MuonChannelDesign::DetType::MM;
          design.inputPitch          = stripPitch;
          design.thickness           = gasTck; 
          design.nMissedTopEta       = nMissedTopEta;  // #of eta strips that are not connected to any FE board
          design.nMissedBottomEta    = nMissedBottomEta;
          design.nMissedTopStereo    = nMissedTopStereo;  // #of stereo strips that are not connected to any FE board
          design.nMissedBottomStereo = nMissedBottomStereo;
          design.totalStrips         = totalStrips;   
          /// The stereo angle is defined clock-wise from the y-axis
          design.defineTrapezoid(m_minHalfY, m_maxHalfY,m_halfX, - stereoAngleIL);
          /// Input width is defined as the distance between two channels
          design.inputWidth          = stripPitch * std::cos(design.stereoAngle());
          
          m_nStrips.push_back(design.totalStrips);
          if (!design.hasStereoAngle()) {  // eta layers
        design.nch      = design.totalStrips - design.nMissedBottomEta - design.nMissedTopEta;
        design.setFirstPos(-0.5 * design.xSize() + stripPitch);
          } else {  // stereo layers
        design.nch      = design.totalStrips - design.nMissedBottomStereo - design.nMissedTopStereo;
        design.setFirstPos( -0.5 * design.xSize() + (1 + design.nMissedBottomStereo - design.nMissedBottomEta) * stripPitch);
          }
          
          MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
          if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG << "initDesign:" << getStationName() << " layer " << il << ", strip pitch " << design.inputPitch
            << ", nstrips " << design.nch << " stereo " << design.stereoAngle() / Gaudi::Units::degree << endmsg;
        }
      }
      
    }
    else
    {
      MMDetectorHelper aHelper;
      MMDetectorDescription* mm   = aHelper.Get_MMDetector(sector_l, std::abs(getStationEta()), getStationPhi(), m_ml, side);
      MMReadoutParameters roParam = mm->GetReadoutParameters();
      
      double ylFrame  = mm->ylFrame();
      double ysFrame  = mm->ysFrame();
      double pitch    = roParam.stripPitch;
      m_sWidthChamber = mm->sWidth();                   // bottom base length (full chamber)
      m_lWidthChamber = mm->lWidth();                   // top base length (full chamber)
      m_lengthChamber = mm->Length();                   // height of the trapezoid (full chamber)
      m_tckChamber    = mm->Tck();                      // thickness (full chamber)
      m_halfX         = roParam.activeH / 2;            // 0.5*radial_size (active area)
      m_minHalfY      = roParam.activeBottomLength / 2; // 0.5*bottom length (active area)
      m_maxHalfY      = roParam.activeTopLength / 2;    // 0.5*top length (active area)
      m_offset        = -0.5*(ylFrame - ysFrame);       // radial dist. of active area center w.r.t. chamber center
      m_readoutSide   = roParam.readoutSide;
      
      for (int il = 0; il < m_nlayers; il++) {
            // identifier of the first channel to retrieve max number of strips
            Identifier id = manager()->mmIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, il + 1, 1);
            int chMax = manager()->mmIdHelper()->channelMax(id);
            if (chMax < 0) {
          chMax = 2500;
          MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
          log << MSG::WARNING << "MMReadoutElement -- Max number of strips not a valid value" << endmsg;
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
        
            m_nStrips.push_back(design.totalStrips);
            if (!design.hasStereoAngle()) {  // eta layers
          design.nch      = design.totalStrips - design.nMissedBottomEta - design.nMissedTopEta;
          design.setFirstPos(-0.5 * design.xSize() + pitch);
            } else {  // stereo layers
          design.nch      = design.totalStrips - design.nMissedBottomStereo - design.nMissedTopStereo;
          design.setFirstPos( -0.5 * design.xSize() + (1 + design.nMissedBottomStereo - design.nMissedBottomEta) * pitch);
            }
        
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            if (log.level() <= MSG::DEBUG)
          log << MSG::DEBUG << "initDesign:" << getStationName() << " layer " << il << ", strip pitch " << design.inputPitch
          << ", nstrips " << design.nch << " stereo " << design.stereoAngle() / Gaudi::Units::degree << endmsg;
      }
    }
    }


    //============================================================================
    void MMReadoutElement::fillCache() {
      
        if (!m_surfaceData)
            m_surfaceData = std::make_unique<SurfaceData>();
        else {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << "calling fillCache on an already filled cache" << endmsg;
            return;
        }


#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
#endif
        for (int layer = 0; layer < m_nlayers; ++layer) {
      // identifier of the first channel
      Identifier id = manager()->mmIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 1);
          const double sAngle = m_etaDesign[layer].stereoAngle();
      m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this,id));
      m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RotatedTrapezoidBounds>(m_halfX, m_minHalfY, m_maxHalfY, sAngle));
      
      m_surfaceData->m_layerTransforms.push_back(
                             absTransform()                         // transformation from chamber to ATLAS frame
                             * m_delta                              // rotations (a-lines) from the alignment group
                             * m_Xlg[layer]                         // x-shift of the gas-gap center w.r.t. quadruplet center
                             * Amg::Translation3D(0., 0., m_offset) // z-shift to volume center
                             * Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D::UnitY()) // x<->z because of GeoTrd definition
                             * Amg::AngleAxis3D(sAngle, Amg::Vector3D::UnitZ())); 
      
      // surface info (center, normal)
      m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
      m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * (-Amg::Vector3D::UnitZ()));
      
#ifndef NDEBUG
          
      if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG << "MMReadoutElement layer " << layer << " sAngle " << sAngle << " phi direction MM eta strip "
                    << (m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D::UnitY()).phi() << endmsg;
#endif
        }
    }


    //============================================================================
    bool MMReadoutElement::containsId(const Identifier& id) const {
        if (manager()->mmIdHelper()->stationEta(id) != getStationEta()) return false;
        if (manager()->mmIdHelper()->stationPhi(id) != getStationPhi()) return false;

        if (manager()->mmIdHelper()->multilayerID(id) != m_ml) return false;

        int gasgap = manager()->mmIdHelper()->gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        int strip = manager()->mmIdHelper()->channel(id);
        return strip >= 1 && strip <= m_nStrips[gasgap - 1];
    }


    //============================================================================
    Amg::Vector3D MMReadoutElement::localToGlobalCoords(const Amg::Vector3D& locPos, const Identifier& id) const {
        int gg = manager()->mmIdHelper()->gasGap(id);
        //const MuonChannelDesign* design = getDesign(id);
        Amg::Vector3D locPos_ML = (m_Xlg[gg - 1]) * Amg::Translation3D(0., 0., m_offset) * 
                                 //   (design->hasStereoAngle() ?  
                                 //    Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D::UnitY()) * Amg::AngleAxis3D(design->stereoAngle(), Amg::Vector3D::UnitZ())  *
                                 //    Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D::UnitY())  : AmgSymMatrix(3)::Identity())*
        locPos;
       
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
        if (log.level() <= MSG::DEBUG) {
            log << MSG::DEBUG << "position coordinates in the gas-gap r.f.:    "  << locPos << endmsg;
            log << MSG::DEBUG << "position coordinates in the multilayer r.f.: " << locPos_ML << endmsg;
        }
#endif
        Amg::Vector3D gVec = absTransform() * m_delta * locPos_ML;
        return gVec;
    }


    //============================================================================
    void MMReadoutElement::setDelta(const ALinePar& aline) {

        // amdb frame (s, z, t) = chamber frame (y, z, x)
        if (aline) {
            m_delta = aline.delta();
            // The origin of the rotation axes is at the center of the active area 
            // in the z (radial) direction. Account for this shift in the definition 
            // of m_delta so that it can be applied on chamber frame coordinates.
            Amg::Translation3D t(0., 0., m_offset);
            m_ALinePar  = &aline;
            m_delta     = t*m_delta*t.inverse();
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
        double width = m_sWidthChamber + (m_lWidthChamber - m_sWidthChamber)*(z0/m_lengthChamber + 0.5); // because z0 is in [-length/2, length/2]

        double s_rel = s0/(width/2.);           // in [-1, 1]
        double z_rel = z0/(m_lengthChamber/2.); // in [-1, 1]
        double t_rel = t0/(m_tckChamber/2.);    // in [-1, 1]

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
            dz += tw*s_rel*t_rel*m_tckChamber/m_lengthChamber;
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

    void MMReadoutElement::refreshCache() {
        clearCache();
        fillCache();
    }
    

    //============================================================================
    bool MMReadoutElement::spacePointPosition(const Identifier& layerId, const Amg::Vector2D& lpos, Amg::Vector3D& pos) const {

        pos = Amg::Vector3D(lpos.x(), lpos.y(), 0.);

        const MuonChannelDesign* design = getDesign(layerId);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << "Unable to get MuonChannelDesign, therefore cannot provide position corrections. Returning." << endmsg;
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
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                log << MSG::WARNING << "As-built corrections are provided only within the active area. Returning." << endmsg;                
                return false;
            } 

            // setup strip calculator
            NswAsBuilt::stripIdentifier_t strip_id;
            strip_id.quadruplet = { (largeSector() ? NswAsBuilt::quadrupletIdentifier_t::MML : NswAsBuilt::quadrupletIdentifier_t::MMS), getStationEta(), getStationPhi(), m_ml };
            strip_id.ilayer     = manager()->mmIdHelper()->gasGap(layerId);
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
            } 
#ifndef NDEBUG
            else {
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                if (log.level() <= MSG::DEBUG) {    
                    log << MSG::DEBUG << "No as-built corrections provided for stEta: "<<getStationEta() << " stPhi: "<<getStationPhi()<<" ml: "<<m_ml<<" layer: "<<strip_id.ilayer<< endmsg;
                }
            }
#endif
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
