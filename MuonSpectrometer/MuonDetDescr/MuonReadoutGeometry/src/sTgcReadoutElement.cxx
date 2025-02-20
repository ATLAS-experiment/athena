/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The sTgc detector = an assembly module = STGC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>

#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelHelpers/StringUtils.h>
#include <GeoModelHelpers/TransformToStringConverter.h>

#include <cmath>
#include <ext/alloc_traits.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Identifier/IdentifierHash.h"
#include "MuonAGDDDescription/sTGCDetectorDescription.h"
#include "MuonAGDDDescription/sTGCDetectorHelper.h"
#include "MuonAGDDDescription/sTGC_Technology.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedDiamondBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

#include "GaudiKernel/ISvcLocator.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#define THROW_EXCEPTION_RE(MSG)                                                                            \
     {                                                                                                  \
        std::stringstream sstr{};                                                                       \
        sstr<<"sTgcReadoutElement - "<<idHelperSvc()->toStringDetEl(identify())<<" "<<__LINE__<<": ";   \
        sstr<<MSG;                                                                                      \
        throw std::runtime_error(sstr.str());                                                           \
     }                                                                                                  \

using namespace GeoStrUtils;
namespace MuonGM {

    //============================================================================
    sTgcReadoutElement::sTgcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, MuonDetectorManager* mgr) 
    : MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::sTgc)
    , m_ml(mL) {

        std::string fixName = (stName[1] == 'L') ? "STL" : "STS";
        Identifier id = mgr->stgcIdHelper()->channelID(fixName, zi, fi, mL, 1, 
                                                       sTgcIdHelper::sTgcChannelTypes::Strip, 1);

        setStationName(fixName);       
        setChamberLayer(mL);
        setIdentifier(id); // representative identifier, with stName, stEta, stPhi, mL 

    }


    //============================================================================
    sTgcReadoutElement::~sTgcReadoutElement() = default;

    //============================================================================
    void sTgcReadoutElement::initDesignFromSQLite(double thickness) {
      SmartIF<IGeoDbTagSvc> geoDbTag{Gaudi::svcLocator()->service("GeoDbTagSvc")};
      SmartIF<IRDBAccessSvc> accessSvc{Gaudi::svcLocator()->service(geoDbTag->getParamSvcName())};

      IRDBRecordset_ptr nswdimRec = accessSvc->getRecordsetPtr("NSWDIM","","");
      IRDBRecordset_ptr wstgcRec  = accessSvc->getRecordsetPtr("WSTGC","","");
      IRDBRecordset_ptr nswPars   = accessSvc->getRecordsetPtr("NSWPARS","","");

      PVConstLink parent = getMaterialGeom()->getParent();
      unsigned int index=parent->indexOf(getMaterialGeom());
      std::string pVName=parent->getNameOfChildVol(index);
      float yCutoutCathode(0);
      if (nswPars->size()==0) {
        THROW_EXCEPTION_RE("Error, cannot access NSWPARS record!");
      } else {
        yCutoutCathode=(*nswPars)[0]->getFloat("NSW_sTGC_yCutoutCathode");
      }

      for (unsigned int ind = 0; ind < wstgcRec->size(); ind++) {
            std::string WSTGC_TYPE       = (*wstgcRec)[ind]->getString("WSTGC_TYPE");               
    
            if (getStationName()[2] != WSTGC_TYPE[6])            continue;
            if (std::abs(getStationEta())!=(int) (WSTGC_TYPE[7]-'0')) continue;
            if (m_ml != (int) (pVName[7]-'0'))                   continue;
            const IRDBRecord *nswdim{nullptr};
            std::string logVolSubName=getMaterialGeom()->getLogVol()->getName().substr(7,4);
            
            size_t w{0};
            for (w=0;w<nswdimRec->size();w++) {
                nswdim = (*nswdimRec)[w];
                const std::string type = nswdim->getString("NSW_TYPE").substr(5,4);
                if (type==logVolSubName) {
                    break;
                }
            }
    
            setSsize(nswdim->getDouble("BASE_WIDTH"));        // bottom base length (full chamber)
            setLongSsize(nswdim->getDouble("TOP_WIDTH"));     // top base length (full chamber)
            setRsize(nswdim->getDouble("LENGTH"));            // height of the trapezoid (full chamber)
    
        
            const double gasTck = (*wstgcRec)[ind]->getDouble("gasTck");
            const double Tck = (*wstgcRec)[ind]->getDouble("Tck");
            const double xFrame = (*wstgcRec)[ind]->getDouble("xFrame");
            const double ylFrame = (*wstgcRec)[ind]->getDouble("ylFrame");
            const double ysFrame = (*wstgcRec)[ind]->getDouble("ysFrame");
            const double wirePitch = (*wstgcRec)[ind]->getDouble("wirePitch");
            const double stripPitch = (*wstgcRec)[ind]->getDouble("stripPitch");
            const double stripWidth = (*wstgcRec)[ind]->getDouble("stripWidth");
            const double sPadWidth = (*wstgcRec)[ind]->getDouble("sPadWidth");
            const double lPadWidth = (*wstgcRec)[ind]->getDouble("lPadWidth");
            const double anglePadPhi = (*wstgcRec)[ind]->getDouble("anglePadPhi");
            const double sStripWidth = (*wstgcRec)[ind]->getDouble("sStripWidth");
            const double lStripWidth = (*wstgcRec)[ind]->getDouble("lStripWidth");
            const int wireGroupWidth = (*wstgcRec)[ind]->getInt("wireGroupWidth");
            const int nStrips = (*wstgcRec)[ind]->getInt("nStrips");
            const std::vector<double> padH = tokenizeDouble((*wstgcRec)[ind]->getString("padH"),";");                   
            const std::vector<double> rankPadPhi = tokenizeDouble((*wstgcRec)[ind]->getString("rankPadPhi"),";");             
            const std::vector<int> nPadPhi = tokenizeInt((*wstgcRec)[ind]->getString("nPadPhi"),";");                
            const std::vector<double> firstPadPhiDivision_C = tokenizeDouble((*wstgcRec)[ind]->getString("firstPadPhiDivision_C"),";");  
            const std::vector<double> PadPhiShift_C = tokenizeDouble((*wstgcRec)[ind]->getString("PadPhiShift_C"),";");          
            const std::vector<double> firstPadPhiDivision_A  = tokenizeDouble((*wstgcRec)[ind]->getString("firstPadPhiDivision_A"),";");  
            const std::vector<double> PadPhiShift_A  = tokenizeDouble((*wstgcRec)[ind]->getString("PadPhiShift_A"),";");          
            const std::vector<double> rankPadH = tokenizeDouble((*wstgcRec)[ind]->getString("rankPadH"),";");               
            const std::vector<int> nPadH = tokenizeInt((*wstgcRec)[ind]->getString("nPadH"),";");                  
            const std::vector<double> firstPadH = tokenizeDouble((*wstgcRec)[ind]->getString("firstPadH"),";");              
            const std::vector<double> firstPadRow = tokenizeDouble((*wstgcRec)[ind]->getString("firstPadRow"),";");            
            const std::vector<double> wireCutout = tokenizeDouble((*wstgcRec)[ind]->getString("wireCutout"),";");             
            const std::vector<int> nWires = tokenizeInt((*wstgcRec)[ind]->getString("nWires"),";");                 
            const std::vector<int> firstWire = tokenizeInt((*wstgcRec)[ind]->getString("firstWire"),";");              
            const std::vector<double> firstTriggerBand = tokenizeDouble((*wstgcRec)[ind]->getString("firstTriggerBand"),";");       
            const std::vector<int> nTriggerBands = tokenizeInt((*wstgcRec)[ind]->getString("nTriggerBands"),";");          
            const std::vector<double> firstStripInTrigger = tokenizeDouble((*wstgcRec)[ind]->getString("firstStripInTrigger"),";");    
            const std::vector<double> firstStripWidth = tokenizeDouble((*wstgcRec)[ind]->getString("firstStripWidth"),";");        
            const std::vector<double> StripsInBandsLayer1 = tokenizeDouble((*wstgcRec)[ind]->getString("StripsInBandsLayer1"),";");    
            const std::vector<double> StripsInBandsLayer2 = tokenizeDouble((*wstgcRec)[ind]->getString("StripsInBandsLayer2"),";");    
            const std::vector<double> StripsInBandsLayer3 = tokenizeDouble((*wstgcRec)[ind]->getString("StripsInBandsLayer3"),";");    
            const std::vector<double> StripsInBandsLayer4 = tokenizeDouble((*wstgcRec)[ind]->getString("StripsInBandsLayer4"),";");    
            const std::vector<int> nWireGroups = tokenizeInt((*wstgcRec)[ind]->getString("nWireGroups"),";");            
            const std::vector<double> firstWireGroup = tokenizeDouble((*wstgcRec)[ind]->getString("firstWireGroup"),";");         
    
            char sector_l  = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
            int  stEta     = std::abs(getStationEta());
            int  Etasign   = getStationEta() / stEta;
            std::string side = (Etasign > 0) ? "A" : "C";
            m_diamondShape = sector_l == 'L' && stEta == 3;
    
    
            // Get frame widths
            setZsize(Tck);             // thickness (full chamber)

            double yCutout  = m_diamondShape ? yCutoutCathode: 0.0; // y of cutout of trapezoid (only in outermost detectors)

    
            // Radial shift of the local frame origin w.r.t. the center of the quadruplet.
            // For diamond shape (QL3) the origin is on the cutout base. For the rest, the it is at the center 
            // of the active area, therefore the shift is half the difference of the top and bottom frame widths.
            m_offset = (m_diamondShape) ? 0.5*getRsize() - (yCutout + ylFrame) : -0.5*(ylFrame - ysFrame); 

            //-------------------
            // Strips
            //-------------------
            for (int il = 0; il < m_nlayers; il++) {      
                m_etaDesign[il].type        = MuonChannelDesign::ChannelType::etaStrip;
                m_etaDesign[il].detType     = MuonChannelDesign::DetType::STGC;
                if (yCutout == 0.) {
                  m_etaDesign[il].defineTrapezoid(0.5 * sStripWidth, 
                                                  0.5 * lStripWidth, 
                                                  0.5 * (getRsize() - ysFrame - ylFrame));
                } else { 
                  m_etaDesign[il].defineDiamond(0.5 * sStripWidth, 
                                                0.5 * lStripWidth, 
                                                0.5 * (getRsize() - ysFrame - ylFrame), yCutout);
                }
                m_etaDesign[il].inputPitch  = stripPitch;
                m_etaDesign[il].inputWidth  = stripWidth;
                m_etaDesign[il].thickness   = gasTck;
                m_etaDesign[il].firstPitch = firstStripWidth[il];
                m_etaDesign[il].setFirstPos(m_diamondShape ? -(m_etaDesign[il].xSize()- yCutout) + m_etaDesign[il].firstPitch
                                                           : -0.5 * m_etaDesign[il].xSize()+ m_etaDesign[il].firstPitch);
                m_etaDesign[il].nch = nStrips;
            }
    
            //-------------------
            // Wires
            //-------------------    
            for (int il = 0; il < m_nlayers; il++) {
              m_phiDesign[il].type        = MuonChannelDesign::ChannelType::phiStrip;
              m_phiDesign[il].detType     = MuonChannelDesign::DetType::STGC;
              if (yCutout == 0.) {
                m_phiDesign[il].defineTrapezoid(0.5 * sPadWidth, 
                                                0.5 * lPadWidth, 
                                                0.5 * (getRsize() - ysFrame - ylFrame) );
              } else { 
                m_phiDesign[il].defineDiamond(0.5 * sPadWidth, 
                                              0.5 * lPadWidth, 
                                              0.5 * (getRsize() - ysFrame - ylFrame), yCutout);
              }
              m_phiDesign[il].inputPitch  = wirePitch;
              m_phiDesign[il].inputWidth  = 0.015;
              m_phiDesign[il].thickness   = getZsize();
              m_phiDesign[il].setFirstPos(firstWire[il]);      // Position of 1st wire, accounts for staggering
              m_phiDesign[il].firstPitch = firstWireGroup[il];                     // Number of Wires in 1st group, group staggering
              m_phiDesign[il].groupWidth  = wireGroupWidth;                           // Number of Wires normal group
              m_phiDesign[il].nGroups = nWireGroups[il];                           // Number of Wire Groups
              m_phiDesign[il].wireCutout = wireCutout[il];                         // Size of "active" wire region for digits
              m_phiDesign[il].nch = nWires[il];

            }

            //-------------------
            // Pads
            //-------------------
            double radius = absTransform().translation().perp() + m_offset;
            for (int il = 0; il < m_nlayers; il++) {
                m_padDesign[il].Length  = getRsize();
                m_padDesign[il].sWidth  = getSsize();
                m_padDesign[il].lWidth  = getLongSsize();
                m_padDesign[il].Size    = getRsize() - ylFrame - ysFrame;
                m_padDesign[il].xFrame  = xFrame;
                m_padDesign[il].ysFrame = ysFrame;
                m_padDesign[il].ylFrame = ylFrame;
                m_padDesign[il].yCutout = yCutout;
                m_padDesign[il].etasign = Etasign;
                m_padDesign[il].setR(radius);
                m_padDesign[il].sPadWidth = sPadWidth;
                m_padDesign[il].lPadWidth = lPadWidth;

                m_padDesign[il].nPadColumns = nPadPhi[il];

                // The C side of the NSW is mirrored instead of rotated
                // We should be using the same values for the pads for both A and C
                // It is easier for us to simply read the same correct value once
                // whereas changing the XML and the reading functions will make this incompatible with past versions
                // Alexandre Laurier 12 Sept 2018
                m_padDesign[il].firstPhiPos= firstPadPhiDivision_A[il];
                m_padDesign[il].inputPhiPitch = anglePadPhi;                                       // stEta<2 ?  PAD_PHI_DIVISION/PAD_PHI_SUBDIVISION : PAD_PHI_DIVISION ;
                m_padDesign[il].PadPhiShift= PadPhiShift_A[il];
                m_padDesign[il].padEtaMin = firstPadRow[il];                                       // FIRST_PAD_ROW_DIVISION[2*sector+(m_ml-1)][stEta-1][il];
                m_padDesign[il].nPadH = nPadH[il];
                m_padDesign[il].padEtaMax = m_padDesign[il].padEtaMin + m_padDesign[il].nPadH;  // PAD_ROWS[2*sector+(m_ml-1)][stEta-1][il];
                m_padDesign[il].firstRowPos = firstPadH[il];                                    // H_PAD_ROW_0[2*sector+(m_ml-1)][il];
                m_padDesign[il].inputRowPitch = padH[il];                                       // PAD_HEIGHT[2*sector+(m_ml-1)][il];
      
                if (sector_l == 'L') {
                  m_padDesign[il].isLargeSector = 1;
                  m_padDesign[il].sectorOpeningAngle = m_padDesign[il].largeSectorOpeningAngle;
                } else {
                  m_padDesign[il].isLargeSector = 0;
                  m_padDesign[il].sectorOpeningAngle = m_padDesign[il].smallSectorOpeningAngle;
                }
                m_padDesign[il].thickness = thickness;
            }      
        }
    }
    void sTgcReadoutElement::initDesignFromAGDD(double thickness) {
        
        char sector_l  = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
        int  stEta     = std::abs(getStationEta());
        int  Etasign   = getStationEta() / stEta;
        std::string side = (Etasign > 0) ? "A" : "C";
        m_diamondShape = sector_l == 'L' && stEta == 3;
     
        ATH_MSG_DEBUG("station name" << getStationName());
     
        sTGCDetectorHelper aHelper;
        sTGCDetectorDescription* stgc = aHelper.Get_sTGCDetector(sector_l, stEta, getStationPhi(), m_ml, side.back());
     
        ATH_MSG_DEBUG( "Found sTGC Detector " << stgc->GetName() );
     
        MuonGM::sTGC_Technology *tech = stgc->GetTechnology();
        if (!tech) THROW_EXCEPTION_RE(" Failed To get Technology for stgc element:"<< stgc->GetName());
     
             
        // Get Chamber length, width and frame widths
        setSsize(stgc->sWidth());                 // bottom base length (full chamber)
        setLongSsize(stgc->lWidth());             // top base length (full chamber)
        setRsize(stgc->Length());                 // height of the trapezoid (full chamber)
        setZsize(stgc->Tck());                    // thickness (full chamber)
        double ysFrame  = stgc->ysFrame();        // Frame thickness on short parallel edge
        double ylFrame  = stgc->ylFrame();        // Frame thickness on long parallel edge
        double xFrame   = stgc->xFrame();         // Frame thickness of non parallel edges
        double yCutout  = stgc->yCutoutCathode(); // y of cutout of trapezoid (only in outermost detectors)
        sTGCReadoutParameters roParam = stgc->GetReadoutParameters();
     
     
        // Radial shift of the local frame origin w.r.t. the center of the quadruplet.
        // For diamond shape (QL3) the origin is on the cutout base. For the rest, the it is at the center 
        // of the active area, therefore the shift is half the difference of the top and bottom frame widths.
        m_offset = (m_diamondShape) ? 0.5*getRsize() - (yCutout + ylFrame) : -0.5*(ylFrame - ysFrame); 
     
        //-------------------
        // Strips
        //-------------------     
        for (int il = 0; il < m_nlayers; il++) {
            // identifier of the first channel - strip plane - to retrieve max number of strips
            m_etaDesign[il].type        = MuonChannelDesign::ChannelType::etaStrip;
            m_etaDesign[il].detType     = MuonChannelDesign::DetType::STGC;
            if (yCutout == 0.) {
                m_etaDesign[il].defineTrapezoid(0.5 * roParam.sStripWidth, 
                                                0.5 * roParam.lStripWidth, 
                                                0.5 * (getRsize() - ysFrame - ylFrame));
            } else { 
                m_etaDesign[il].defineDiamond(0.5 * roParam.sStripWidth, 
                                            0.5 * roParam.lStripWidth, 
                                            0.5 * (getRsize() - ysFrame - ylFrame), yCutout);
            }
            m_etaDesign[il].inputPitch  = stgc->stripPitch();
            m_etaDesign[il].inputWidth  = stgc->stripWidth();
            m_etaDesign[il].thickness   = tech->gasThickness;
            m_etaDesign[il].firstPitch  = roParam.firstStripWidth[il];
            m_etaDesign[il].setFirstPos((m_diamondShape) ? -(m_etaDesign[il].xSize()- yCutout) + m_etaDesign[il].firstPitch
                                                         : -0.5 * m_etaDesign[il].xSize()+ m_etaDesign[il].firstPitch);
            m_etaDesign[il].nch         = roParam.nStrips;

            ATH_MSG_DEBUG("initDesign:" << getStationName() << " layer " << il 
                       << ", strip pitch " << m_etaDesign[il].inputPitch
                       << ", nstrips " << m_etaDesign[il].nch 
                       << ", firstPos: " << m_etaDesign[il].firstPos() );
        }
        
        //-------------------
        // Wires
        //-------------------
        for (int il = 0; il < m_nlayers; il++) {
            m_phiDesign[il].type = MuonChannelDesign::ChannelType::phiStrip;
            m_phiDesign[il].detType = MuonChannelDesign::DetType::STGC;
            if (yCutout == 0.) {
                m_phiDesign[il].defineTrapezoid(0.5 * roParam.sPadWidth, 
                                                0.5 * roParam.lPadWidth, 
                                                0.5 * (getRsize() - ysFrame - ylFrame) );
            } else { 
                m_phiDesign[il].defineDiamond(0.5 * roParam.sPadWidth, 
                                            0.5 * roParam.lPadWidth, 
                                            0.5 * (getRsize() - ysFrame - ylFrame), yCutout);
            }
            m_phiDesign[il].inputPitch  = stgc->wirePitch();
            m_phiDesign[il].inputWidth  = 0.015;
            m_phiDesign[il].thickness   = getZsize();
            m_phiDesign[il].setFirstPos(roParam.firstWire[il]);      // Position of 1st wire, accounts for staggering
            m_phiDesign[il].firstPitch  = roParam.firstWireGroup[il]; // Number of Wires in 1st group, group staggering
            m_phiDesign[il].groupWidth  = roParam.wireGroupWidth;     // Number of Wires normal group
            m_phiDesign[il].nGroups     = roParam.nWireGroups[il];    // Number of Wire Groups
            m_phiDesign[il].wireCutout  = roParam.wireCutout[il];     // Size of "active" wire region for digits
            m_phiDesign[il].nch         = roParam.nWires[il];
                
            ATH_MSG_DEBUG( "initDesign:" << getStationName() << " layer " << il << ", wireGang pitch "
                            << m_phiDesign[il].inputPitch << ", nWireGangs " << m_phiDesign[il].nch );
        }

        //-------------------
        // Pads
        //-------------------
        double radius = absTransform().translation().perp() + m_offset;
        for (int il = 0; il < m_nlayers; il++) {
            m_padDesign[il].Length  = getRsize();
            m_padDesign[il].sWidth  = getSsize();
            m_padDesign[il].lWidth  = getLongSsize();
            m_padDesign[il].Size    = getRsize() - ylFrame - ysFrame;
            m_padDesign[il].xFrame  = xFrame;
            m_padDesign[il].ysFrame = ysFrame;
            m_padDesign[il].ylFrame = ylFrame;
            m_padDesign[il].yCutout = yCutout;
            m_padDesign[il].etasign = Etasign;
            m_padDesign[il].setR(radius);
            m_padDesign[il].sPadWidth = roParam.sPadWidth;
            m_padDesign[il].lPadWidth = roParam.lPadWidth;
            m_padDesign[il].nPadColumns = roParam.nPadPhi[il];

            // The C side of the NSW is mirrored instead of rotated
            // We should be using the same values for the pads for both A and C
            // It is easier for us to simply read the same correct value once
            // whereas changing the XML and the reading functions will make this incompatible with past versions
            // Alexandre Laurier 12 Sept 2018
            m_padDesign[il].firstPhiPos   = roParam.firstPadPhiDivision_A[il];
            m_padDesign[il].inputPhiPitch = roParam.anglePadPhi;       // stEta<2 ?  PAD_PHI_DIVISION/PAD_PHI_SUBDIVISION : PAD_PHI_DIVISION ;
            m_padDesign[il].PadPhiShift   = roParam.PadPhiShift_A[il];
            m_padDesign[il].padEtaMin     = roParam.firstPadRow[il];   // FIRST_PAD_ROW_DIVISION[2*sector+(m_ml-1)][stEta-1][il];
            m_padDesign[il].nPadH         = roParam.nPadH[il];
            m_padDesign[il].padEtaMax     = m_padDesign[il].padEtaMin + roParam.nPadH[il];  // PAD_ROWS[2*sector+(m_ml-1)][stEta-1][il];
            m_padDesign[il].firstRowPos   = roParam.firstPadH[il];     // H_PAD_ROW_0[2*sector+(m_ml-1)][il];
            m_padDesign[il].inputRowPitch = roParam.padH[il];          // PAD_HEIGHT[2*sector+(m_ml-1)][il];

            if (sector_l == 'L') {
                m_padDesign[il].isLargeSector = 1;
                m_padDesign[il].sectorOpeningAngle = m_padDesign[il].largeSectorOpeningAngle;
            } else {
                m_padDesign[il].isLargeSector = 0;
                m_padDesign[il].sectorOpeningAngle = m_padDesign[il].smallSectorOpeningAngle;
            }

            m_padDesign[il].thickness = thickness;

            ATH_MSG_DEBUG( "initDesign stationname " << getStationName() << " layer " << il << ",pad phi angular width "
                    << m_padDesign[il].inputPhiPitch << ", eta pad size " << m_padDesign[il].inputRowPitch
                    << "  Length: " << m_padDesign[il].Length << " sWidth: " << m_padDesign[il].sWidth
                    << " lWidth: " << m_padDesign[il].lWidth << " firstPhiPos:" << m_padDesign[il].firstPhiPos
                    << " padEtaMin:" << m_padDesign[il].padEtaMin << " padEtaMax:" << m_padDesign[il].padEtaMax
                    << " firstRowPos:" << m_padDesign[il].firstRowPos << " inputRowPitch:" << m_padDesign[il].inputRowPitch
                    << " thickness:" << m_padDesign[il].thickness << " sPadWidth: " << m_padDesign[il].sPadWidth
                    << " lPadWidth: " << m_padDesign[il].lPadWidth << " xFrame: " << m_padDesign[il].xFrame
                    << " ysFrame: " << m_padDesign[il].ysFrame << " ylFrame: " << m_padDesign[il].ylFrame
                    << " yCutout: " << m_padDesign[il].yCutout );       
        }    
    }
    
    void sTgcReadoutElement::initDesign(double thickness) {
        
        if (manager()->MinimalGeoFlag() == 0) {
            PVConstLink pvc {getMaterialGeom()};
            unsigned int nchildvol = pvc->getNChildVols();
            int llay = 0;
            std::string::size_type npos;
            for (unsigned ich = 0; ich < nchildvol; ++ich) {
                PVConstLink pc = pvc->getChildVol(ich);
                std::string childname = (pc->getLogVol())->getName();

                ATH_MSG_DEBUG("Volume Type: " << pc->getLogVol()->getShape()->type());
                if ((npos = childname.find("Sensitive")) != std::string::npos) {
                    ++llay;
                    if (llay > 4) {
                        ATH_MSG_DEBUG("number of sTGC layers > 4: increase transform array size");
                        continue;
                    }
                    m_Xlg[llay - 1] = pvc->getXToChildVol(ich);
                }
            }
            assert(m_nlayers ==  llay);             
        }

        SmartIF<IGeoDbTagSvc> geoDbTag{Gaudi::svcLocator()->service("GeoDbTagSvc")};
        if (!geoDbTag) THROW_EXCEPTION_RE( "Could not locate GeoDbTagSvc" );
        if (geoDbTag->getSqliteReader()) initDesignFromSQLite(thickness);
        else initDesignFromAGDD(thickness);

    }

    //============================================================================
    void sTgcReadoutElement::fillCache() {

        if (m_surfaceData) {
            ATH_MSG_WARNING("calling fillCache on an already filled cache" );
            return;
        }

        m_surfaceData = std::make_unique<SurfaceData>();

        for (int layer{0}; layer < m_nlayers; ++layer) {
        
            // Define the geometry for the strips, pads and wires of this readout element. 
            // For QL3 (cutoff trapezoid), diamondBounds are used, while trapezoid bounds are used for the rest. 
            // The assigned coordinate along the layer normal is at the center of the gas gap; 
            // wires are considered at x=0, while strips and pads are shifted by +10/-10 microns.

            //-------------------
            // Layer boundaries
            //-------------------

            if (m_diamondShape) {
                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::RotatedDiamondBounds>(m_etaDesign[layer].minYSize() / 2., 
                                                                                                  m_etaDesign[layer].maxYSize() / 2., 
                                                                                                  m_etaDesign[layer].maxYSize() / 2., 
                                                                                                  m_etaDesign[layer].xSize() / 2. - m_etaDesign[layer].yCutout() / 2, 
                                                                                                  m_etaDesign[layer].yCutout() / 2));  // strips

                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::DiamondBounds>(m_padDesign[layer].sPadWidth / 2., 
                                                                                           m_padDesign[layer].lPadWidth / 2., 
                                                                                           m_padDesign[layer].lPadWidth / 2., 
                                                                                           m_padDesign[layer].Size / 2. - m_padDesign[layer].yCutout / 2, m_padDesign[layer].yCutout / 2));  // pad and wires

            } else {
                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::RotatedTrapezoidBounds>(m_etaDesign[layer].xSize() / 2., 
                                                                                                    m_etaDesign[layer].minYSize() / 2., 
                                                                                                    m_etaDesign[layer].maxYSize() / 2.));  // strips

                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::TrapezoidBounds>(m_padDesign[layer].sPadWidth /2., 
                                                                                             m_padDesign[layer].lPadWidth / 2., 
                                                                                             m_padDesign[layer].Size / 2.));


            }

            //-------------------
            // Wires
            //-------------------

            // identifier of the first channel - wire plane - locX along phi, locY max->min R
            Identifier id = m_idHelper.channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 2, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));
            m_surfaceData->m_layerTransforms.push_back(
               absTransform()                        // transformation from chamber to ATLAS frame
               * m_delta                             // transformations from the alignment group
               * m_Xlg[layer]                        // x-shift of the gas-gap center w.r.t. quadruplet center
               * Amg::getTranslateZ3D(m_offset)      // z-shift to volume center (after m_delta!)
               * Amg::getRotateY3D(-90 * CLHEP::deg)   // x<->z because of GeoTrd definition
               * Amg::getRotateZ3D(-90 * CLHEP::deg)); // x<->y for wires

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));

            //-------------------
            // Strips
            //-------------------

            const double shift{layer%2 == 0 ? 0.01 : -0.01}; // 1st layer gets +0.01; layer numbering starts from 0 here!

            // identifier of the first channel - strip plane
            id = m_idHelper.channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 1, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));

            m_surfaceData->m_layerTransforms.push_back(absTransform() * m_delta * m_Xlg[layer] *Amg::Translation3D(shift, 0., m_offset)
                                                      *Amg::getRotateY3D(-90 * CLHEP::deg)); // x<->z because of GeoTrd definition

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));

            //-------------------
            // Trigger Pads
            //-------------------
            
            // identifier of the first channel - pad plane
            id = m_idHelper.channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 0, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));

            m_surfaceData->m_layerTransforms.push_back(absTransform() * m_delta * m_Xlg[layer] * 
                                                         Amg::getTranslate3D(-shift, 0., m_offset)
                                                       * Amg::getRotateY3D(-90 * CLHEP::deg)   // x<->z because of GeoTrd definition
                                                       * Amg::getRotateZ3D(-90 * CLHEP::deg)); // x<->y for pads

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));
        }
    }


    //============================================================================
    bool sTgcReadoutElement::containsId(const Identifier& id) const {
        if (m_idHelper.stationEta(id) != getStationEta()) return false;
        if (m_idHelper.stationPhi(id) != getStationPhi()) return false;

        if (m_idHelper.multilayerID(id) != m_ml) return false;

        int gasgap = m_idHelper.gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        int strip = m_idHelper.channel(id);
        if (strip < 1) return false;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Strip && strip > m_etaDesign[gasgap - 1].nch) return false;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire &&  strip > m_phiDesign[gasgap -1].nGroups) return false;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) {
            const auto [etaId, phiId] = m_padDesign[gasgap -1].etaPhiId(strip);
            if (etaId < 0 || phiId < 0) return false;
        }
        return true;
    }


    //============================================================================
    double sTgcReadoutElement::channelPitch(const Identifier& id) const {
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) {
            const MuonPadDesign* design = getPadDesign(id);
            if (!design) {                
                ATH_MSG_WARNING( "no pad Design" );
                return -1;
            }
            return design->channelWidth(Amg::Vector2D::Zero(), 0);
        }

        const MuonChannelDesign* design = getDesign(id);
        if (!design) return -1;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Strip)  // sTGC strips
            return design->inputPitch;
        else if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire)  // sTGC wires
            return design->inputPitch * design->groupWidth;        // wire Pitch * number of wires in a group
        else
            return -1;
    }


    //============================================================================
    int sTgcReadoutElement::padNumber(const Amg::Vector2D& pos, const Identifier& id) const {
        const MuonPadDesign* design = getPadDesign(id);
        if (!design) {            
            ATH_MSG_WARNING( "no pad Design" );
            return -1;
        }
        std::pair<int, int> pad(design->channelNumber(pos));
        const sTgcIdHelper& id_helper{*manager()->stgcIdHelper()};
        if (pad.first > 0 && pad.second > 0) {
#ifndef NDEBUG
            bool is_valid {true};
#endif
            const Identifier padID = id_helper.padID(id, id_helper.multilayer(id),
                                                        id_helper.gasGap(id), sTgcIdHelper::Pad, pad.first, pad.second
#ifndef NDEBUG
                                                        , is_valid
#endif
                                                        
                                                        );
            int channel = id_helper.channel(padID);
            int padEta = id_helper.padEta(padID);
            int padPhi = id_helper.padPhi(padID);
            if (
#ifndef NDEBUG                
                !is_valid ||
#endif                
                 padEta != pad.first || padPhi != pad.second) {
                
                ATH_MSG_WARNING( " bad pad indices: input " << pad.first << " " << pad.second << " from ID " << padEta << " "
                    << padPhi );
                return -1;
            }
            return channel;
        }
        
        ATH_MSG_WARNING(__LINE__<< " bad channelNumber" <<pad.first<<" "<<pad.second );

        return -1;
    }


    //============================================================================
    int sTgcReadoutElement::wireNumber(const Amg::Vector2D& pos, const Identifier& id) const {
        const MuonChannelDesign* design = getDesign(id);
        if (!design) {            
            ATH_MSG_WARNING( "no wire design when trying to get the wire number" );
            return -1;
        }
        return design->wireNumber(pos);
    }


    //============================================================================
    double sTgcReadoutElement::wirePitch(int gas_gap) const {
        if (m_phiDesign.empty()) {            
            ATH_MSG_WARNING( "no wire design when trying to get the wire pitch" );
            return -1.0;
        }
        return (m_phiDesign[gas_gap - 1]).inputPitch;
    }


    //============================================================================
    double sTgcReadoutElement::positionFirstWire(const Identifier& id) const {
        double pos_wire = -9999.9;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) {
            const MuonChannelDesign* design = getDesign(id);
            if (!design) {                
                ATH_MSG_WARNING( "no wire design when trying to get the 1st wire position" );
                return pos_wire;
            }
            pos_wire = design->firstPos();
        } else {            
            ATH_MSG_WARNING( "attempt to retrieve the 1st wire position with a wrong identifier" );
        }
        return pos_wire;
    }


    //============================================================================
    int sTgcReadoutElement::numberOfWires(const Identifier& id) const {
        int nWires = -1;
        if (m_idHelper.channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) {
            const MuonChannelDesign* design = getDesign(id);
            if (!design) {                
                ATH_MSG_WARNING( "no wire design when trying to get the total number of wires" );
                return nWires;
            }
            nWires = design->nch;
        } else {            
            ATH_MSG_WARNING( "attempt to retrieve the number of wires with a wrong identifier" );
        }
        return nWires;
    }


    //============================================================================
    Amg::Vector3D sTgcReadoutElement::localToGlobalCoords(const Amg::Vector3D& locPos, Identifier id) const {
        int gg = m_idHelper.gasGap(id);
        int channelType = m_idHelper.channelType(id);

        // The assigned coordinate along the layer normal is at the center of the gas gap; 
        // wires are considered at x=0, while:
        // for layers 1, 3 strips (pads) are shifted by +10 (-10) microns
        // for layers 2, 4 strips (pads) are shifted by -10 (+10) microns 
        double shift{0.};
        if (channelType != 2) shift = ((gg % 2) ^ (channelType==0)) ? 0.01 : -0.01;
        const Amg::Vector3D locPos_ML = m_Xlg[gg - 1] * Amg::getTranslate3D(shift, 0., m_offset) * locPos;
        
        ATH_MSG_DEBUG( "position coordinates in the gas-gap r.f.:    "  << Amg::toString(locPos) );
        ATH_MSG_DEBUG( "position coordinates in the multilayer r.f.: " << Amg::toString(locPos_ML) );
        return absTransform() * m_delta * locPos_ML;
    }


    //============================================================================
    void sTgcReadoutElement::setDelta(const ALinePar& aline) {
        // amdb frame (s, z, t) = chamber frame (y, z, x)        
        if (aline) {
            static const Amg::Transform3D permute{GeoTrf::GeoRotation{90.*Gaudi::Units::deg,90.*Gaudi::Units::deg, 0.}};
            // The origin of the rotation axes is at the center of the active area 
            // in the z (radial) direction. Account for this shift in the definition 
            // of m_delta so that it can be applied on chamber frame coordinates.
            m_ALinePar  = &aline;
            m_delta     = Amg::getTranslateZ3D(m_offset)* permute*aline.delta()*
                          permute.inverse()*Amg::getTranslateZ3D(-m_offset);
            ATH_MSG_DEBUG(idHelperSvc()->toStringDetEl(identify())<<" setup new alignment: "<<GeoTrf::toString(m_delta));
            refreshCache();
        } else {
            clearALinePar();
        }
    }

    //============================================================================
    void sTgcReadoutElement::clearALinePar() {
        if (has_ALines()) {
            m_ALinePar = nullptr; 
            m_delta = Amg::Transform3D::Identity(); 
            refreshCache();
        }
    }

    //============================================================================
    void sTgcReadoutElement::setBLinePar(const BLinePar& bLine) {
        ATH_MSG_DEBUG("Setting B-line for " <<idHelperSvc()->toStringDetEl(identify())<<" "<<bLine);
        m_BLinePar = &bLine;
    }

    //============================================================================
    void sTgcReadoutElement::posOnDefChamber(Amg::Vector3D& locPosML) const {

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
        const double bp = m_BLinePar->getParameter(Parameter::bp);
        const double bn = m_BLinePar->getParameter(Parameter::bn);
        const double sp = m_BLinePar->getParameter(Parameter::sp);
        const double sn = m_BLinePar->getParameter(Parameter::sn);
        const double tw = m_BLinePar->getParameter(Parameter::tw);
        const double eg = m_BLinePar->getParameter(Parameter::eg)*1.e-3;
        const double ep = m_BLinePar->getParameter(Parameter::ep)*1.e-3;
        const double en = m_BLinePar->getParameter(Parameter::en)*1.e-3;

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
    void sTgcReadoutElement::spacePointPosition(const Identifier& layerId, double locXpos, double locYpos, Amg::Vector3D& pos) const {

        pos = Amg::Vector3D(locXpos, locYpos, 0.);

        const MuonChannelDesign* design = getDesign(layerId);
        if (!design) {            
            ATH_MSG_WARNING( "Unable to get MuonChannelDesign, therefore cannot provide position corrections. Returning." );
            return;
        }

        bool conditionsApplied{false};
        Amg::Transform3D trfToML{Amg::Transform3D::Identity()};

#ifndef SIMULATIONBASE
        //*********************
        // As-Built (MuonNswAsBuilt is not included in AthSimulation)
        //*********************
        const NswAsBuilt::StgcStripCalculator* sc = manager()->getStgcAsBuiltCalculator();        
        if(manager()->getsTGCAsBuilt2() && design->type == MuonChannelDesign::ChannelType::etaStrip){
            pos.head<2>() = manager()->getsTGCAsBuilt2()->correctPosition(layerId, pos.head<2>());

        } else if (sc && design->type == MuonChannelDesign::ChannelType::etaStrip) {

            Amg::Vector2D lpos(locXpos, locYpos);
            
            // express the local position w.r.t. the nearest active strip
            Amg::Vector2D rel_pos;
            int istrip = design->positionRelativeToStrip(lpos, rel_pos);
            if (istrip < 0) {                
                ATH_MSG_WARNING( "As-built corrections are provided only for eta strips within the active area. Returning." );
                return;
            }

            // setup strip calculator
            NswAsBuilt::stripIdentifier_t strip_id;
            strip_id.quadruplet = { (largeSector() ? NswAsBuilt::quadrupletIdentifier_t::STL : NswAsBuilt::quadrupletIdentifier_t::STS), getStationEta(), getStationPhi(), m_ml };
            strip_id.ilayer     = m_idHelper.gasGap(layerId);
            strip_id.istrip     = istrip;

            // get the position coordinates, in the chamber frame, from NswAsBuilt.
            // applying the 10um shift along the beam axis for strips (see fillCache()).
            NswAsBuilt::StgcStripCalculator::position_t calcPos = sc->getPositionAlongStgcStrip(NswAsBuilt::Element::ParameterClass::CORRECTION, strip_id, rel_pos.y(), rel_pos.x());
            
            if (calcPos.isvalid == NswAsBuilt::StgcStripCalculator::IsValid::VALID) {
                pos = calcPos.pos;
                pos[0] += (strip_id.ilayer%2) ? 0.01 : -0.01; // 1st layer gets +0.01; layer numbering starts from 1

                // signal that pos is now in the chamber reference frame
                // (don't go back to the layer frame yet, since we may apply b-lines later on)
                trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);   
                conditionsApplied = true;
            } else {                
                ATH_MSG_DEBUG( "No as-built corrections provided for stEta: "<<getStationEta() << " stPhi: "<<getStationPhi()<<" ml: "<<m_ml<<" layer: "<<strip_id.ilayer);
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
    }

}  // namespace MuonGM
