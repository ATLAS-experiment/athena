/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**   
 *   @file ITkPixelCablingAlg.cxx
 *
 *   @brief Fills an ITkPixel cabling object from a plain text source
 *
 *   @author Shaun Roe, Ondra Kovanda
 *   @date June 2024
 */

//package includes
#include "ITkPixelCablingAlg.h"

//indet includes
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"


//Athena includes
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "PathResolver/PathResolver.h"
#include "AthenaKernel/IOVInfiniteRange.h"



// Gaudi include
#include "GaudiKernel/EventIDRange.h"


//STL
#include <iostream>
#include <fstream>



// Constructor
ITkPixelCablingAlg::ITkPixelCablingAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

//
StatusCode
ITkPixelCablingAlg::initialize() {
  m_source = PathResolver::find_file(m_source.value(), "DATAPATH");
  if (m_source.empty()) {
    ATH_MSG_FATAL("The ITkPixel data file for cabling, " << m_source.value() << ", was not found.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Reading cabling from " << m_source.value());
  // ITkPixelID
  ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));
  // det manager
  if (m_useTestCabling) ATH_CHECK(detStore()->retrieve(m_detManager, "ITkPixel"));
  // Write Cond Handle
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}


//
StatusCode
ITkPixelCablingAlg::execute(const EventContext& ctx) const {
  // Write Cond Handle
  SG::WriteCondHandle<ITkPixelCablingData> writeHandle = SG::makeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                  << ". In theory this should not be called, but may happen"
                  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<ITkPixelCablingData> pCabling = std::make_unique<ITkPixelCablingData>();
  
  //For development purposes, generate a dummy cabling structure, using the existing
  //offline IDs and dummy online ones. It's problematic to deal with quads easily, as
  //they have one offline ID, but up to 4 online IDs. If there will be virtual elinks
  //for merged quads or if there's no data merging, the situation is not mappable for
  //the encoding offline -> online map. Once could at most map to certain bits that
  //would be shared between the 4 online IDs. In the decoding direction, the
  //online->offline will be always mappable, although it would be degenerate mapping.
  if (m_useTestCabling){
    ATH_MSG_DEBUG("Using test cabling generated with ITkPixelCablingAlg::generateTestCabling(...)");
    ATH_CHECK(generateTestCabling(pCabling));
    pCabling->print();
    if (writeHandle.record(EventIDRange(IOVInfiniteRange::infiniteRunLB()), std::move(pCabling)).isFailure()) {
        return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
  }

  
  
  auto inputFile = std::ifstream(m_source.value());
  if (not inputFile.good()){
    ATH_MSG_ERROR("The itk cabling file "<<m_source.value()<<" could not be opened.");
    return StatusCode::FAILURE;
  }


  inputFile>>*pCabling;
  const int numEntries = pCabling->size();
  ATH_MSG_DEBUG(numEntries << " entries were made to the identifier map.");

  // Define validity of the output cond object and record it
  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0, 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDRange rangeW{start, stop};
  if (writeHandle.record(rangeW, std::move(pCabling)).isFailure()) {
    ATH_MSG_FATAL("Could not record ITkPixelCablingData " << writeHandle.key() 
                  << " with EventRange " << rangeW
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("recorded new conditions data object " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
  return (numEntries==0) ? (StatusCode::FAILURE) : (StatusCode::SUCCESS);
}

StatusCode ITkPixelCablingAlg::generateTestCabling(std::unique_ptr<ITkPixelCablingData>& cabling) const {
    //We'll loop over all known hashes, and create a sensible cabling situation
    //for testing purposes, create online ID by shifting the wafer ID by two bits, allowing
    //to accommodate the non-merged quads with 4 online links vs. one offline entity.
    //What's used as the online "base" (32 bit) is just the offline ID (64bit) >> 32.
    //Lowest two bits are always 0 in the base - in fact, at least 2 bytes are. Fine for testing.
    
    for (size_t hash = 0; hash < m_idHelper->wafer_hash_max(); hash++){
        Identifier id = m_idHelper->wafer_id(hash);
        uint32_t onID = (id.get_compact() >> 32) & 0xFFFFFFFF;
        
        const InDetDD::SiDetectorElement *element = m_detManager->getDetectorElement(id);
        const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign *>(&element->design());

        ITkPixelCabling::ModuleType moduleType;
        ITkPixelCabling::TransformType moduleTransform;
        if (p_design->numberOfCircuits() == 4){
            //it's a quad
            
            //Let's make the entire layer 1 and 2 non-merged.
            //We can experiment with this, but we're also after
            //a proof-of-principle here.
            if (m_idHelper->barrel_ec(id) == 0 && (m_idHelper->layer_disk(id) == 1 || m_idHelper->layer_disk(id) == 2)){
                //Non-merged quad has 4 online IDs mappend onto 1 offline,
                //differentiated by 2-bit chip ID
                moduleType = ITkPixelCabling::ModuleType::SimpleQuad;
                moduleTransform = ITkPixelCabling::TransformType::NominalQuad;
                cabling->addEntryOnOff(onID | 0b00, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
                cabling->addEntryOnOff(onID | 0b01, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
                cabling->addEntryOnOff(onID | 0b10, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
                cabling->addEntryOnOff(onID | 0b11, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
            
            }
            else{
                //Merged quads have 1:1 online:offline correspondence.
                //Don't need to invent any substructure, but keep it
                //consistent with the quads
                moduleType = ITkPixelCabling::ModuleType::MergedQuad;
                moduleTransform = ITkPixelCabling::TransformType::NominalQuad;
                cabling->addEntryOnOff(onID, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
            }
            
            //We can also fill in the offline->online map
            //Were are creating the "base" online ID, i. e. without the chip ID
            cabling->addEntryOffOn(id, ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>(onID, moduleType, moduleTransform));


        }
        else {
            
            //All triplets map 1:1 to chips. Keeping the online
            //"base" ID consistently shifted - we're dealing with chip 00
            ITkPixelCabling::ModuleType moduleType = m_idHelper->barrel_ec(id) == 0 ? ITkPixelCabling::ModuleType::IBTriplet : ITkPixelCabling::ModuleType::IECTriplet;
            ITkPixelCabling::TransformType moduleTransform = m_idHelper->barrel_ec(id) == 0 ? ITkPixelCabling::TransformType::NominalIBTriplet : ITkPixelCabling::TransformType::NominalIECTriplet;
            cabling->addEntryOnOff(onID, ITkPixelCabling::ModuleInfo(id, moduleType, moduleTransform));
            
            //We can also fill in the offline->online map
            //Were are creating the "base" online ID, i. e. without the chip ID
            cabling->addEntryOffOn(id, ITkPixelCabling::ModuleInfo<ITkPixelOnlineId>(onID, moduleType, moduleTransform));
        }
 
    }
    
    return StatusCode::SUCCESS;
}

