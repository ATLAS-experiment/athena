/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelEncodingAlg.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "InDetIdentifier/PixelID.h"


ITkPixelEncodingAlg::ITkPixelEncodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_pixelManager(nullptr),
  m_pixIdHelper(nullptr)
{

}


StatusCode ITkPixelEncodingAlg::initialize()
{
  return StatusCode::SUCCESS;
}


StatusCode ITkPixelEncodingAlg::execute(const EventContext& ctx) const
{

  fillChipMaps();

  return StatusCode::SUCCESS;
}


void ITkPixelEncodingAlg::fillChipMaps() const 
{
  
  ATH_MSG_DEBUG("In RD53BEncodingAlg::fillChipMaps ... starting!" );

  // retrieve RDO container
  const PixelRDO_Container* p_pixelRDO_cont;
  if (evtStore()->retrieve(p_pixelRDO_cont, "PixelRDOs") == StatusCode::SUCCESS) {
    
    // loop over all pixel elements 
    InDetDD::SiDetectorElementCollection::const_iterator element;
    for (element = m_pixelManager->getDetectorElementBegin(); element != m_pixelManager->getDetectorElementEnd(); ++element) {
      
      if (!(*element)) continue;
      
      // get the IdHash of the detector element
      IdentifierHash IdHash = (*element)->identifyHash();
      
      const InDetDD::PixelModuleDesign *design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&((*element)->design()));
      
      // get the module and chip definitions
      const int chips = design->numberOfCircuits();            
      int rowsPerChip = design->rowsPerCircuit();
      int columnsPerChip = design->columnsPerCircuit();
      const int chipsInPhi = design->rows()/rowsPerChip;
      const int chipsinEta = design->columns()/columnsPerChip;            
      
      const float phiPitch = design->phiPitch();
      bool use50x50 = true;
      if (phiPitch < s_pitch50x50)
        use50x50 = false;        
      
      // get the region and the layer ltype for filling the histograms
      Region region = (*element)->isBarrel() ? BARREL : ENDCAP;
        
      
      bool doSwapCoordinates = false;
      // takle the case where the chips are rotated. You need to swap the phi/eta indices for the pixel
      // since the front-end as well is rotated and the chip map has to get the right coordinates
      // It happens for all the single chip modules in the endcap:
      // - innermost layer
      // - shorties in the next-to-innermost layer          
      if (region==ENDCAP and chips==1) {
        doSwapCoordinates = true;
      }
      
      // swap dimensions if needed
      if (doSwapCoordinates) 
        std::swap(columnsPerChip, rowsPerChip);
      
      
      // The chipmap was initialized here -- sroygara
      // std::vector<ChipMap> chip_maps = std::vector<ChipMap>(chips, ChipMap(columnsPerChip, rowsPerChip, use50x50));
            
      // get the RDO collection associated to the detector element
      PixelRDO_Container::const_iterator rdoCont_itr(p_pixelRDO_cont->indexFind(IdHash));
      
      // if the collection is filled, fill the chip map, otherwise leave it empty       
      if (rdoCont_itr!=p_pixelRDO_cont->end()) {        
        
        // loop though the rdo collection 
        PixelRDO_Collection::const_iterator rdo_itr((*rdoCont_itr)->begin());
        const PixelRDO_Collection::const_iterator rdo_end((*rdoCont_itr)->end());
        for ( ; rdo_itr != rdo_end; ++rdo_itr ) {

          // Get info from RDO          
          const Identifier rdoID((*rdo_itr)->identify());
          int pixPhiIx(m_pixIdHelper->phi_index(rdoID));
          int pixEtaIx(m_pixIdHelper->eta_index(rdoID));
          const int tot((*rdo_itr)->getToT());
          
          // swap coordinates if needed
          if (doSwapCoordinates) {
            std::swap(pixEtaIx,pixPhiIx);
          }
          
          // evaluating the chip number considering the number of rows and columns per chip and
          // the total number of rows and columns on the sensor
          int chip = std::ceil(pixEtaIx/columnsPerChip) + chipsInPhi*std::ceil(pixPhiIx/rowsPerChip);
                    
          // get the eta/phi index wrt to the chip, not the module
          int pixEta = pixEtaIx - std::ceil(pixEtaIx/columnsPerChip)*columnsPerChip;
          int pixPhi = pixPhiIx - std::ceil(pixPhiIx/rowsPerChip)*rowsPerChip;          
                       
          
          
          // The info is then passed to some sort of chip map -- sroygara
          //chip_maps.at(chip).fillChipMap(pixEta, pixPhi, tot);
          
        }        
      }
    }
  }
          
  ATH_MSG_DEBUG("In RD53BEncodingAlg::fillChipMaps ... done!" );
}










