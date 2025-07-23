/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// This class builds one Si Crystal to be placed in one module
//
// The information on which layer to build is retrieved by the Geometry Manager
//
// Contained by: PixelModule_log
//
// Contains: nothing
//
#include "GeoPixelSiCrystal.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCommonItems.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "ReadoutGeometryBase/PixelDiodeTreeBuilder.h"

#include <algorithm> //for std::min, std::max
#include <utility>
#include <vector>

#include "PixelGeoUtils.h"
namespace {
   InDetDD::PixelReadoutTechnology getPixelReadoutTechnology(int rowsPerCircuit, int columnsPerCircuit) {
      if (rowsPerCircuit*columnsPerCircuit>26000) { return InDetDD::PixelReadoutTechnology::FEI4; }
      else                                        { return InDetDD::PixelReadoutTechnology::FEI3; }
   }
}
using namespace InDetDD;

GeoPixelSiCrystal::GeoPixelSiCrystal(InDetDD::PixelDetectorManager* ddmgr,
                                     PixelGeometryManager* mgr,
				                     GeoModelIO::ReadGeoModel* sqliteReader,
                                     std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                                     std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX,
                                     bool isBLayer, bool isModule3D)
  : GeoVPixelFactory (ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX))
{
  // 
  //Builds the design for this crystal
  m_isBLayer = isBLayer;
  m_isModule3D = isModule3D;
 
  // Dimensions
  const double thickness = m_gmt_mgr->PixelBoardThickness(m_isModule3D);
  const double length = m_gmt_mgr->PixelBoardLength(m_isModule3D);
  const double width = m_gmt_mgr->PixelBoardWidth(m_isModule3D);

  int circuitsPerPhi = m_gmt_mgr->DesignCircuitsPhi(m_isModule3D); // Warning col/row naming opposite to chip
  int circuitsPerEta = m_gmt_mgr->DesignCircuitsEta(m_isModule3D); // Warning col/row naming opposite to chip
  int cellRowPerCirc = m_gmt_mgr->DesignCellRowsPerCircuit(m_isModule3D);
  int cellColPerCirc = m_gmt_mgr->DesignCellColumnsPerCircuit(m_isModule3D);
  int rowsPerCircuit = m_gmt_mgr->DesignDiodeRowsPerCircuit(m_isModule3D);
  int columnsPerCircuit = m_gmt_mgr->DesignDiodeColumnsPerCircuit(m_isModule3D);
  int readoutSide = m_gmt_mgr->DesignReadoutSide(m_isModule3D);

  double pitchEtaLongEnd =  m_gmt_mgr->DesignPitchZLongEnd(m_isModule3D);
  double pitchEtaLong =  m_gmt_mgr->DesignPitchZLong(m_isModule3D);
  double pitchPhi = m_gmt_mgr->DesignPitchRP(m_isModule3D);
  double pitchEta = m_gmt_mgr->DesignPitchZ(m_isModule3D);

  auto  readoutTechnology = getPixelReadoutTechnology(rowsPerCircuit,columnsPerCircuit );
  auto circuitsPerPhi_corr = circuitsPerPhi;
  auto rowsPerCircuit_corr = rowsPerCircuit;
  if (readoutTechnology==InDetDD::PixelReadoutTechnology::FEI3 && circuitsPerPhi==1 && rowsPerCircuit==328) {
     // FEI3 has 2x8 circuits and 160 + 4 rows per circuit not 1x8 circuits with 328 rows
     // without this correction it is not possible to use the attribute associated to sub-matrices
     // to assign FE numbers to sub-matrices.
     circuitsPerPhi_corr*=2;
     rowsPerCircuit_corr/=2;
  }
  constexpr auto kNDirections = InDetDD::detail::kNDirections;
  constexpr auto kNPixelLocations = InDetDD::detail::kNPixelLocations;
  PixelDiodeTree diode_tree = InDetDD::detail::makePixelDiodeTree(m_gmt_mgr,
                                                    readoutTechnology,
                                                    std::array<int,kNDirections>{circuitsPerPhi_corr,circuitsPerEta},    // [0]=phi/row, [1]=eta/column
                                                    std::array<int,kNDirections>{rowsPerCircuit_corr,columnsPerCircuit}, // [0]=phi/row, [1]=eta/column
                                                    std::array<std::array<double,kNDirections>,kNPixelLocations>{        // regular/central,longEnd/outer,long/inner
                                                       std::array<double,kNDirections>{pitchPhi,pitchEta},
                                                       std::array<double,kNDirections>{0.,pitchEtaLongEnd},
                                                       std::array<double,kNDirections>{0.,pitchEtaLong}});

  std::unique_ptr<PixelModuleDesign> p_barrelDesign2 = std::make_unique<PixelModuleDesign>(thickness,
							     circuitsPerPhi,
							     circuitsPerEta,
							     cellColPerCirc,
							     cellRowPerCirc,
							     columnsPerCircuit,
							     rowsPerCircuit,
							     std::move(diode_tree),
							     InDetDD::electrons,
							     readoutSide,
							     false,              /* 3D */
							     InDetDD::Undefined, /* detector type */
							     readoutTechnology);

  // Multiple connections (ganged pixels)
  if (m_gmt_mgr->NumberOfEmptyRows() > 0) {
    int minRow = m_gmt_mgr->EmptyRows(0);
    int maxRow = minRow;
    for (int iConnect = 0; iConnect < m_gmt_mgr->NumberOfEmptyRows(); iConnect++){
      minRow = std::min(minRow, m_gmt_mgr->EmptyRows(iConnect));
      minRow = std::min(minRow, m_gmt_mgr->EmptyRowConnections(iConnect));
      maxRow = std::max(maxRow, m_gmt_mgr->EmptyRows(iConnect));
      maxRow = std::max(maxRow, m_gmt_mgr->EmptyRowConnections(iConnect));
    }
  
    std::vector <int> connections(maxRow-minRow+1);
    
    // We fill them all with a one to one correspondence first.
    for (unsigned int iRow = 0; iRow < connections.size(); iRow++){
      connections[iRow] = iRow +  minRow;
    }
    
    // Now make the connections.
    for (int iConnect = 0; iConnect < m_gmt_mgr->NumberOfEmptyRows(); iConnect++){
      connections[m_gmt_mgr->EmptyRows(iConnect)-minRow] = m_gmt_mgr->EmptyRowConnections(iConnect);
    }
   
    p_barrelDesign2->addMultipleRowConnection(minRow, connections);

  } else {
    // No empty rows.
    if (!m_gmt_mgr->ibl()) {
      m_gmt_mgr->msg(MSG::WARNING) << "GeoPixelSiCrystal: No ganged pixels" << endmsg;
    } else {
      if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) << "GeoPixelSiCrystal: No ganged pixels" << endmsg;
    }
  }
  
  // Check that the active area seems reasonable

  if ( (m_gmt_mgr->DesignRPActiveArea(m_isModule3D) > width) ||
       (m_gmt_mgr->DesignZActiveArea(m_isModule3D) >  length) || 
       (width - m_gmt_mgr->DesignRPActiveArea(m_isModule3D) > 4 * Gaudi::Units::mm) || 
       (length - m_gmt_mgr->DesignZActiveArea(m_isModule3D) > 4 * Gaudi::Units::mm) ) { 
    m_gmt_mgr->msg(MSG::WARNING) << "GeoPixelSiCrystal: Active area not consistent with sensor size. Sensor: " 
			       << width/Gaudi::Units::mm << " x " << length/Gaudi::Units::mm << ", Active: " 
			       << m_gmt_mgr->DesignRPActiveArea(m_isModule3D)/Gaudi::Units::mm << " x " << m_gmt_mgr->DesignZActiveArea(m_isModule3D)/Gaudi::Units::mm 
			       << endmsg;
  } else {
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) 
      << "GeoPixelSiCrystal: Sensor: "  
      << width/Gaudi::Units::mm << " x " << length/Gaudi::Units::mm << ", Active: " 
      << m_gmt_mgr->DesignRPActiveArea(m_isModule3D)/Gaudi::Units::mm << " x " << m_gmt_mgr->DesignZActiveArea(m_isModule3D)/Gaudi::Units::mm 
      << endmsg;		       
  }


  m_design = m_DDmgr->addDesign(std::move(p_barrelDesign2));
  
  
}
GeoVPhysVol* GeoPixelSiCrystal::Build() {

  GeoFullPhysVol* siPhys{nullptr};
  int brl_ec= m_gmt_mgr->isEndcap() ? 2*m_gmt_mgr->GetSide() : 0;

  if(m_sqliteReader) {
    std::string siName ="SiPhys_" + std::to_string(brl_ec) +"_"+ std::to_string(m_gmt_mgr->GetLD()) +"_"+ std::to_string(m_gmt_mgr->Phi()) +"_"+ std::to_string(m_gmt_mgr->Eta());
    siPhys = (*m_mapFPV)[siName];
  }
  else {
    //(sar) code moved from c'tor..
    const double thickness = m_gmt_mgr->PixelBoardThickness(m_isModule3D);
    const double length = m_gmt_mgr->PixelBoardLength(m_isModule3D);
    const double width = m_gmt_mgr->PixelBoardWidth(m_isModule3D);
    //
    std::string matName = m_gmt_mgr->getMaterialName("Sensor");
    const GeoMaterial* siMat = m_mat_mgr->getMaterial(matName);
    const GeoBox* siBox = new GeoBox(thickness*0.5,width*0.5,length*0.5);
    std::string logname{"siLog"};
    // There is not a strong need to give the blayer a different name but leave it for now. 
    if(m_isBLayer) logname = "siBLayLog";
    auto *logVolume = new GeoLogVol(logname,siBox,siMat);
    //(sar) ...to here


    siPhys = new GeoFullPhysVol(logVolume);
  }

  // Build the Identifier for the silicon:
  //
  const PixelID * idHelper = m_gmt_mgr->getIdHelper();
  m_id = idHelper->wafer_id(brl_ec,m_gmt_mgr->GetLD(),m_gmt_mgr->Phi(),m_gmt_mgr->Eta());
  SiDetectorElement * element = new SiDetectorElement(m_id, m_design, siPhys, m_gmt_mgr->commonItems());
  
  // add the element to the manager
  m_DDmgr->addDetectorElement(element);
  return siPhys;
}
