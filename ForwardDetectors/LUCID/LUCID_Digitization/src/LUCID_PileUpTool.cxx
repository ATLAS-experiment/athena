/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_PileUpTool.h"

#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "GeneratorObjects/HepMcParticleLink.h"


#include <map>
#include <algorithm>

/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::initialize()
{

  ATH_MSG_DEBUG ( "LUCID_PileUpTool::initialize() called " );

  setParameters(m_numTubesProp,
                m_qdcChannelsPerPEProp,
                m_qdcPedestalProp,
                m_qdcFedNoiseFactorProp,
                m_tdcPmtNoiseFactorProp,
                m_tdcFedNoiseFactorProp,
                m_TotalPMTgainProp,
                m_AmpFactorProp,
                m_Q1binProp,
                m_NoiseChargeProp,
                m_numDyinodesProp,
                m_dynodeGammaFactorProp,
                m_pmtSmearingProp,
                m_pmtScalingProp,
                m_gasScalingProp,
                m_npeThresholdProp,
                m_fillRootTreeProp);

  ATH_MSG_DEBUG ( " numTubes         : " << m_numTubes          << endmsg
                  << " qdcChannelsPerPE : " << m_qdcChannelsPerPE  << endmsg
                  << " qdcPedestal      : " << m_qdcPedestal       << endmsg
                  << " qdcFedNoiseFactor: " << m_qdcFedNoiseFactor << endmsg
                  << " tdcPmtNoiseFactor: " << m_tdcPmtNoiseFactor << endmsg
                  << " tdcFedNoiseFactor: " << m_tdcFedNoiseFactor << endmsg
                  << " fillRootTree     : " << m_fillRootTree      );

  ATH_CHECK(m_randomSvc.retrieve());
  ATH_MSG_DEBUG ( "Retrieved RandomNumber Service"           );

  ATH_CHECK(m_mergeSvc.retrieve());
  ATH_MSG_DEBUG ( "Retrieved PileUpMergeSvc" );

  m_mergedhitList = new LUCID_SimHitCollection();

  if (m_fillRootTree)
    {
      ATH_CHECK(setDebugTree());
      ATH_MSG_DEBUG ( "Retrieved ROOT tree" );
    }

  return StatusCode::SUCCESS;
}

/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::prepareEvent(const EventContext& /*ctx*/, const unsigned int nInputEvents)
{
  ATH_MSG_DEBUG ( "prepareEvent() called for " << nInputEvents << " input events" );

  CHECK(recordContainers(this->evtStore(), m_key_digitCnt));
  ATH_MSG_DEBUG ( " Digit container is recorded in StoreGate " );

  m_mergedhitList->clear();

  return StatusCode::SUCCESS;
}

/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::processBunchXing(int bunchXing,
                                              SubEventIterator bSubEvents,
                                              SubEventIterator eSubEvents)
{
  ATH_MSG_DEBUG ( "LUCID_PileUpTool::processBunchXing() " << bunchXing );

  SubEventIterator iEvt = bSubEvents;

  for (; iEvt!=eSubEvents; ++iEvt)
    {

      StoreGateSvc& seStore = *iEvt->ptr()->evtStore();
      ATH_MSG_VERBOSE ( "SubEvt EventInfo from StoreGate " << seStore.name() << " :"
                        << " bunch crossing : " << bunchXing
                        << " time offset : "    << iEvt->time()
                        << " event number : "   << iEvt->ptr()->eventNumber()
                        << " run number : "     << iEvt->ptr()->runNumber()
                        );
      const LUCID_SimHitCollection* tmpColl = nullptr;

      if (!seStore.retrieve(tmpColl,m_SimHitCollectionName.value()).isSuccess())
        {

          ATH_MSG_ERROR ( "SubEvent BCM SiHitCollection not found in StoreGate " << seStore.name() );

          return StatusCode::FAILURE;
        }

      ATH_MSG_DEBUG ( "LUCID_SimHitCollection found with " << tmpColl->size() << " hits" );

      LUCID_SimHitCollection::const_iterator i = tmpColl->begin();
      LUCID_SimHitCollection::const_iterator e = tmpColl->end();

      for (; i!=e; ++i) m_mergedhitList->push_back((*i));

    }

  return StatusCode::SUCCESS;
}

/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::mergeEvent(const EventContext& ctx)
{
  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
  ATH_CHECK(fillDigitContainer(m_mergedhitList, rngEngine));
  ATH_MSG_DEBUG ( " LUCID_DigitContainer successfully registered in StoreGate " );

  return StatusCode::SUCCESS;
}

/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::processAllSubEvents(const EventContext& ctx)
{
  ATH_MSG_VERBOSE ( "processAllSubEvents()" );

  typedef PileUpMergeSvc::TimedList<LUCID_SimHitCollection>::type TimedHitCollList;

  TimedHitCollList hitCollList;

  ATH_CHECK(m_mergeSvc->retrieveSubEvtsData(m_SimHitCollectionName, hitCollList));
  ATH_MSG_DEBUG ( "Retrieved TimedHitCollList" );

  ATH_MSG_DEBUG ( " PileUp: Merge " << hitCollList.size() << " LUCID_SimHitCollection with key " << m_SimHitCollectionName.value() );

  TimedHitCollection<LUCID_SimHit> thpclucid;

  TimedHitCollList::iterator iColl  (hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());

  while (iColl != endColl)
    {
      const LUCID_SimHitCollection* tmpColl(iColl->second);

      thpclucid.insert(iColl->first, tmpColl);

      ATH_MSG_DEBUG ( " LUCID_SimHitCollection found with " << tmpColl->size() << " hits " << iColl->first );

      ++iColl;
    }


  ATH_CHECK(recordContainers(this->evtStore(), m_key_digitCnt));
  ATH_MSG_DEBUG ( " Digit container is recorded in StoreGate " );

  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
  ATH_CHECK(fillDigitContainer(thpclucid, rngEngine));
  ATH_MSG_DEBUG ( " Digit container was filled successfully " );

  return StatusCode::SUCCESS;
}
/// ----------------------------------------------------------------------------------------------------
StatusCode LUCID_PileUpTool::finalize()
{
  return StatusCode::SUCCESS;
}
