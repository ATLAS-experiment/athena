/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelEncodingAlg.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "StoreGate/ReadHandle.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

ITkPixelEncodingAlg::ITkPixelEncodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_pixelManager(nullptr),
  m_pixIdHelper(nullptr)
{

}


StatusCode ITkPixelEncodingAlg::initialize()
{
  
  ATH_CHECK(m_pixelRDOKey.initialize());

  // retrieve PixelID helper
  if (!detStore()->retrieve(m_pixIdHelper, "PixelID").isSuccess()) {
    ATH_MSG_FATAL("Unable to retrieve PixelID helper");
    return StatusCode::FAILURE;
  }

  // retrieve PixelDetectorManager
  if (!detStore()->retrieve(m_pixelManager,"ITkPixel").isSuccess()) {
    ATH_MSG_FATAL("Unable to retrieve PixelDetectorManager");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode ITkPixelEncodingAlg::execute(const EventContext& ctx) const
{

  // const PixelRDO_Container p_pixelRDO_cont = nullptr;
  SG::ReadHandle<PixelRDO_Container> rdoContainer(m_pixelRDOKey, ctx);

  PixelRDO_Container::const_iterator rdoCollections      = rdoContainer->begin();
  PixelRDO_Container::const_iterator rdoCollectionsEnd   = rdoContainer->end();

  for(; rdoCollections!=rdoCollectionsEnd; ++rdoCollections){
    const COLLECTION* RDO_Collection(*rdoCollections);

    for(const auto *const rdo : *RDO_Collection) {
      // const Identifier rdoID = rdo->identify();

      const Identifier rdoID = rdo->identify();
      const Identifier wafferID = m_pixIdHelper->wafer_id(rdoID);

      int pixPhiIx = m_pixIdHelper->phi_index(rdoID);
      int pixEtaIx = m_pixIdHelper->eta_index(rdoID);

      const int tot = rdo->getToT();
      uint32_t chip = getFE(wafferID, rdoID);
      uint32_t col = getColumn(wafferID, rdoID);
      uint32_t row = getRow(wafferID, rdoID);

      ATH_MSG_INFO("Chip: "+ std::to_string(chip) + "  ToT: " + std::to_string(tot) + " pixEtaIx: " +  std::to_string(pixEtaIx) + " col: " +  std::to_string(col)+ "  pixPhiIx: " + std::to_string(pixPhiIx) + "  row: " + std::to_string(row));
    };
  };

  return StatusCode::SUCCESS;
}



// From ITkPixelReadoutManager
uint32_t ITkPixelEncodingAlg::getColumn(const Identifier wafferID, const Identifier rdoID) const {


  const InDetDD::SiDetectorElement *element = m_pixelManager->getDetectorElement(wafferID);
  const InDetDD::PixelModuleDesign *design = static_cast<const InDetDD::PixelModuleDesign *>(&element->design());

  int eta_index = m_pixIdHelper->eta_index(rdoID);
  int columnsPerFE = design->columnsPerCircuit();


  // ---------------------
  // Convert eta index to column number
  // ---------------------
  unsigned int column{};
  if (eta_index >= columnsPerFE) {
    column = 2 * columnsPerFE - eta_index - 1;
  } else {
    column = eta_index;
  }


  return column;
}

// From ITkPixelReadoutManager
uint32_t ITkPixelEncodingAlg::getRow(const Identifier wafferID, const Identifier rdoID) const {
  const InDetDD::SiDetectorElement *element = m_pixelManager->getDetectorElement(wafferID);
  const InDetDD::PixelModuleDesign *design = static_cast<const InDetDD::PixelModuleDesign *>(&element->design());

  unsigned int FEsPerRow = design->numberOfCircuitsPerRow();
  unsigned int rowsPerFE =  design->rowsPerCircuit();
  unsigned int phi_index = m_pixIdHelper->phi_index(rdoID);

  // Identify the module type
  Region region = element->isBarrel() ? BARREL : ENDCAP;
  if (region == ENDCAP) {
    // Swap phi_index for even endcap modules
    int module_phi = m_pixIdHelper->phi_module(wafferID);
    if (module_phi % 2 == 0) {
      phi_index = FEsPerRow * rowsPerFE - phi_index - 1;
    }
  }

  // ---------------------
  // Convert phi index to row number
  // ---------------------
  unsigned int row{};
  if (phi_index >= rowsPerFE) {
    row = 2 * rowsPerFE - phi_index - 1;
  } else {
    row = phi_index;
  }

  return row;
}


uint32_t ITkPixelEncodingAlg::getFE(const Identifier wafferID, const Identifier rdoID) const {
  const InDetDD::SiDetectorElement *element = m_pixelManager->getDetectorElement(wafferID);
  const InDetDD::PixelModuleDesign *design = static_cast<const InDetDD::PixelModuleDesign *>(&element->design());

  unsigned int FEsPerRow = design->numberOfCircuitsPerRow();
  unsigned int rowsPerFE =  design->rowsPerCircuit();
  unsigned int columnsPerFE = design->columnsPerCircuit();

  // ---------------------
  // Set module properties
  // ---------------------
  unsigned int phi_index = m_pixIdHelper->phi_index(rdoID);
  unsigned int eta_index = m_pixIdHelper->eta_index(rdoID);

  // Identify the module type
  Region region = element->isBarrel() ? BARREL : ENDCAP;
  if (region == ENDCAP) {
    // Swap phi_index for even endcap modules
    int module_phi = m_pixIdHelper->phi_module(wafferID);
    if (module_phi % 2 == 0) {
      phi_index = FEsPerRow * rowsPerFE - phi_index - 1;
    }
  }

  // ---------------------
  // Compute FE number
  // ---------------------
  // ITk has up to 4 FEs
  unsigned int FErow = static_cast<unsigned int>(std::floor(phi_index / rowsPerFE));
  unsigned int FEcol = static_cast<unsigned int>(std::floor(eta_index / columnsPerFE));
  if (FErow > 0) {
    return 2 + FEcol;
  } else {
    return FEcol;
  }
}
