/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSamplesMon/LArShapeCompleteMaker.h"

#include "CaloIdentifier/CaloGain.h"
#include "LArCafJobs/DataStore.h"
#include "LArSamplesMon/TreeShapeErrorGetter.h"

#include "LArIdentifier/LArOnlineID.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "LArRawConditions/LArShapeComplete.h"
#include "LArSamplesMon/ShapeErrorData.h"
#include "LArCafJobs/DataContainer.h"
#include "LArSamplesMon/History.h"
#include "LArCafJobs/EventData.h"
#include <memory>

using namespace LArSamples;

LArShapeCompleteMaker::LArShapeCompleteMaker(const std::string & name, ISvcLocator * pSvcLocator) : 
  AthAlgorithm(name, pSvcLocator),
  m_template(nullptr),
  m_dumperTool("LArShapeDumperTool")
{
  declareProperty("NSamples", m_nSamples = 5);
  declareProperty("NPhases",  m_nPhases = 17);
  declareProperty("IPhase",   m_iPhase = 8);
  declareProperty("ShapeErrorFileName", m_shapeErrorFileName);
  declareProperty("OutputFileName", m_outputFileName);
  declareProperty("OutputTemplateFileName", m_outputTemplateFileName);
  declareProperty("StoreGateKey", m_sgKey = "LArResiduals");
  declareProperty("MinNPulses", m_minNPulses = 0);
  declareProperty("GroupingType", m_groupingType = "ExtendedSubDetector");
}


LArShapeCompleteMaker::~LArShapeCompleteMaker() 
{
}


StatusCode LArShapeCompleteMaker::initialize()
{
  
  ATH_CHECK(detStore()->retrieve(m_onlineHelper, "LArOnlineID"));

  ATH_CHECK( m_onOffMapKey.initialize() );
  
  return StatusCode::SUCCESS; 
}


StatusCode LArShapeCompleteMaker::execute()
{
  if (m_template) return StatusCode::SUCCESS;
  m_template = new DataStore();

  auto lsc = std::make_unique<LArShapeComplete>();

  if (lsc->setGroupingType(m_groupingType, msg()).isFailure() || lsc->initialize().isFailure()) {
    ATH_MSG_ERROR("Unable to initialize LArShapeComplete");
    return StatusCode::FAILURE;    
  };

  SG::ReadCondHandle<LArOnOffIdMapping> onOffMap (m_onOffMapKey);

  auto errorGetter = std::make_unique<TreeShapeErrorGetter>(m_shapeErrorFileName);
  
  for (unsigned int k = 0; k < LArSamples::Definitions::nChannels; k++) {   
    
    HWIdentifier channelID = m_onlineHelper->channel_Id((IdentifierHash)k);
    const Identifier id = onOffMap->cnvToIdentifier(channelID);
    if (id.get_compact() == 0xffffffff) {
      ATH_MSG_WARNING( "Skipping invalid hash = " << k << "." );
      continue;
    }

    CellInfo* info = m_dumperTool->makeCellInfo(channelID, id);
    if (!info) {
      ATH_MSG_ERROR("Problem making cell info for hash = " << k << ", terminating...");
      return StatusCode::FAILURE;
    }
    
    HistoryContainer* histCont = m_template->makeNewHistory(k, info);
    std::vector<const EventData*> events;
    for (unsigned int g = 0; g < 3; g++) {
      DataContainer* data = new DataContainer((CaloGain::CaloGain)g, std::vector<short>(), 0, 0, 0, -1, std::vector<float>());
      events.push_back(new EventData(-1,-1,-1,-1));
      histCont->add(data);
    }
    History* history = new History(*histCont, events, k, errorGetter.get());
    
    for (unsigned int g = 0; g < 3; g++) {
      CaloGain::CaloGain gain = (CaloGain::CaloGain)g;
      const LArSamples::ShapeErrorData* sed = nullptr;
      if (history) sed = history->shapeErrorData(gain, LArSamples::BestShapeError);
      
      // The containers
      
      // Fill them with 0's to start with
      std::vector<float> phaseVect(m_nSamples,0);
      std::vector<std::vector<float> > deltaVal(m_nPhases, phaseVect);
      std::vector<std::vector<float> > deltaDer(m_nPhases, phaseVect);
      

      if (sed && sed->n() > m_minNPulses) {
       ATH_MSG_DEBUG("--> Setting channel " << k << ", id = " << channelID << ", gain = " << gain << ", size = " 
                << deltaVal.size() << " " << deltaDer.size());
        for (unsigned int i = 0; i < m_nSamples; i++) {
          deltaVal[m_iPhase][i] = sed->xi()(i);
          deltaDer[m_iPhase][i] = sed->xip()(i);
          ATH_MSG_DEBUG("       xi[" << i << " ] = " << deltaVal[m_iPhase][i]);
        }
      }
      else {
       ATH_MSG_DEBUG("--> No information for channel " << k<< ", id = " << channelID << ", gain = " << gain); 
      }
      lsc->set(channelID, gain, deltaVal, deltaDer);
    }
    delete history; 
  }

  ATH_MSG_INFO("Writing LArShapeComplete output to " << m_outputFileName );
  //TFile* out = TFile::Open(m_outputFileName.c_str(), "RECREATE");
  //out->WriteObjectAny(lsc, "LArShapeComplete", "LArShape");
  //delete out;
  if (detStore()->record(std::move(lsc), m_sgKey).isFailure()) {
    ATH_MSG_ERROR("Unable to write LArShapeComplete");
    return StatusCode::FAILURE;    
  };

  ATH_MSG_INFO( "Done!");

  return StatusCode::SUCCESS;
}


StatusCode LArShapeCompleteMaker::finalize()
{
  ATH_MSG_INFO( "Writing template..." );
  m_template->writeTrees(m_outputTemplateFileName.c_str());
  ATH_MSG_INFO("done..." );
  return StatusCode::SUCCESS;
}
