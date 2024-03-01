/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SimpleMergeMcEventCollTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "AtlasHepMC/HeavyIon.h"
#include <fstream>

SimpleMergeMcEventCollTool::SimpleMergeMcEventCollTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface *parent) :
  PileUpToolBase(type, name, parent)
{
}

StatusCode SimpleMergeMcEventCollTool::initialize()
{
  if(!m_pMergeSvc.empty())
    {
       ATH_CHECK(m_pMergeSvc.retrieve());
    }
  return StatusCode::SUCCESS;
}

/// PileUpTools Approach
StatusCode SimpleMergeMcEventCollTool::prepareEvent(const EventContext& /*ctx*/, unsigned int nInputEvents)
{
  ATH_MSG_VERBOSE ( "prepareEvent()" );
  m_nBkgEventsReadSoFar = 0;
  m_newevent = true;

  //Check we are getting at least one event
  m_nInputMcEventColls = nInputEvents;
  if (0 == m_nInputMcEventColls)
     {
      ATH_MSG_ERROR("prepareEvent: TimedTruthList with key "
                    << m_truthCollInputKey << " is empty");
      return StatusCode::RECOVERABLE;
    }
  ATH_MSG_DEBUG( "prepareEvent: there are " << m_nInputMcEventColls << " subevents in this event.");

  m_outputMcEventCollection = nullptr;
  return StatusCode::SUCCESS;
}

StatusCode SimpleMergeMcEventCollTool::processBunchXing(int /*bunchXing*/,
                                                        SubEventIterator bSubEvents,
                                                        SubEventIterator eSubEvents)
{
  SubEventIterator iEvt(bSubEvents);
  //loop over the McEventCollections (each one assumed to containing exactly one GenEvent) of the various input events
  while (iEvt != eSubEvents)
    {
      StoreGateSvc& seStore(*iEvt->ptr()->evtStore());
      const McEventCollection *pMEC(nullptr);
      ATH_CHECK(seStore.retrieve(pMEC, m_truthCollInputKey));
      ATH_MSG_DEBUG ("processBunchXing: SubEvt McEventCollection from StoreGate " << seStore.name() );
      if (!m_outputMcEventCollection) {
        m_outputMcEventCollection = new McEventCollection();
        ATH_CHECK(evtStore()->record(m_outputMcEventCollection, m_truthCollOutputKey));
      }
      ATH_CHECK(this->processEvent(
                                   pMEC,
                                   m_outputMcEventCollection,
                                   iEvt->index(),
                                   static_cast<int>(iEvt->time()),
                                   static_cast<int>(iEvt->type())
                                   ));
      ++iEvt;
    }
  return StatusCode::SUCCESS;
}

StatusCode SimpleMergeMcEventCollTool::mergeEvent(const EventContext& /*ctx*/)
{
  ATH_MSG_DEBUG( "mergeEvent()" );
  if(m_nBkgEventsReadSoFar+1<m_nInputMcEventColls)
    {
      ATH_MSG_WARNING( "mergeEvent: Expected " << m_nInputMcEventColls << " subevents, but only saw " << m_nBkgEventsReadSoFar+1 << "! The job will probably crash now..." );
      return StatusCode::FAILURE;
    }
  if(msgLvl(MSG::VERBOSE)) { this->printDetailsOfMergedMcEventCollection(m_outputMcEventCollection); }
  return StatusCode::SUCCESS;
}

/// Algorithm Approach
StatusCode SimpleMergeMcEventCollTool::processAllSubEvents(const EventContext& /*ctx*/)
{
  ATH_MSG_VERBOSE ( "processAllSubEvents()" );
  SG::WriteHandle<McEventCollection> outputMcEventCollection(m_truthCollOutputKey.value());
  ATH_CHECK(outputMcEventCollection.record(std::make_unique<McEventCollection>()));

  //first get the list of McEventCollections
  typedef PileUpMergeSvc::TimedList<McEventCollection>::type TimedTruthList;
  TimedTruthList truthList;
  ATH_CHECK(m_pMergeSvc->retrieveSubEvtsData(m_truthCollInputKey.value(), truthList));

  m_nBkgEventsReadSoFar=0;
  m_newevent = true;

  //Check we are getting at least one event
  m_nInputMcEventColls=truthList.size();
  if (0 == m_nInputMcEventColls)
    {
      ATH_MSG_ERROR("TimedTruthList with key " << m_truthCollInputKey << " is empty.");
      return StatusCode::RECOVERABLE;
    }

  ATH_MSG_DEBUG( "execute: there are " << m_nInputMcEventColls << " subevents in this event.");
  //TODO can we make this into an auto for loop?
  TimedTruthList::iterator timedTruthListIter(truthList.begin()), endOfTimedTruthList(truthList.end());
  //loop over the McEventCollections (each one assumed to containing exactly one GenEvent) of the various input events
  while (timedTruthListIter != endOfTimedTruthList)
    {
      const PileUpTimeEventIndex& currentPileUpTimeEventIndex(timedTruthListIter->first); //time() , type()
      const McEventCollection *pBackgroundMcEvtColl(&*(timedTruthListIter->second));
      ATH_CHECK(this->processEvent(
                                   pBackgroundMcEvtColl,
                                   outputMcEventCollection.ptr(),
                                   currentPileUpTimeEventIndex.index(),
                                   static_cast<int>(currentPileUpTimeEventIndex.time()),
                                   static_cast<int>(currentPileUpTimeEventIndex.type())
                                   ));
      ++timedTruthListIter;
    } //timed colls

  if(msgLvl(MSG::VERBOSE)) { this->printDetailsOfMergedMcEventCollection(outputMcEventCollection.ptr()); }
  return StatusCode::SUCCESS;
}

/// Common methods

StatusCode SimpleMergeMcEventCollTool::saveHeavyIonInfo(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection)
{
  if (outputMcEventCollection->at(0)->heavy_ion()) return StatusCode::SUCCESS;
  if (pMcEvtColl->at(0)->heavy_ion())
    {
//It should be clarified if we want to get a copy or the content
#ifdef HEPMC3
     HepMC::GenHeavyIonPtr hinew=std::make_shared<HepMC::GenHeavyIon>(*(pMcEvtColl->at(0)->heavy_ion()));
     outputMcEventCollection->at(0)->set_heavy_ion(hinew);
#else
      outputMcEventCollection->at(0)->set_heavy_ion(*(pMcEvtColl->at(0)->heavy_ion()));
#endif
    }
  return StatusCode::SUCCESS;
}

StatusCode SimpleMergeMcEventCollTool::processEvent(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection, const int currentBkgEventIndex, int bunchCrossingTime, int pileupType)
{
  ATH_MSG_VERBOSE ( "processEvent() Event Type: " << pileupType << ", BunchCrossingTime: " << bunchCrossingTime );
  if (!outputMcEventCollection) {
    ATH_MSG_ERROR( this->name()<<"::processEvent() was passed an null output McEventCollection pointer." );
    return StatusCode::FAILURE;
  }
  if (!pMcEvtColl) {
    ATH_MSG_ERROR( this->name()<<"::processEvent() was passed an null input McEventCollection pointer." );
    return StatusCode::FAILURE;
  }

  if ( pMcEvtColl->empty() || (m_onlySaveSignalTruth && !m_newevent) ) {
    ++m_nBkgEventsReadSoFar;
    return StatusCode::SUCCESS;
  }

  //GenEvt is there

  const HepMC::GenEvent& currentBackgroundEvent(**(pMcEvtColl->begin()));
  // FIXME no protection against multiple GenEvents having the same event number
  HepMC::GenEvent* copiedEvent = new HepMC::GenEvent(currentBackgroundEvent);
  if (m_overrideEventNumbers) {
    copiedEvent->set_event_number(currentBkgEventIndex);
  }
  HepMC::fillBarcodesAttribute(copiedEvent);
#ifdef HEPMC3
  copiedEvent->add_attribute("BunchCrossingTime",std::make_shared<HepMC3::IntAttribute>(bunchCrossingTime));
  copiedEvent->add_attribute("PileUpType",std::make_shared<HepMC3::IntAttribute>(pileupType));
#endif
   outputMcEventCollection->push_back(copiedEvent);
  ATH_CHECK(this->saveHeavyIonInfo(pMcEvtColl, outputMcEventCollection));
   m_newevent = false;
  ++m_nBkgEventsReadSoFar;
  return StatusCode::SUCCESS;
}

void SimpleMergeMcEventCollTool::printDetailsOfMergedMcEventCollection(McEventCollection *outputMcEventCollection) const
{
  if (outputMcEventCollection->empty()) { return; }
  DataVector<HepMC::GenEvent>::const_iterator outputEventItr(outputMcEventCollection->begin());
  const DataVector<HepMC::GenEvent>::const_iterator endOfEvents(outputMcEventCollection->end());
  ATH_MSG_INFO ( "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" );
  ATH_MSG_INFO ( "Current OUTPUT GenEvent: " );
  while(outputEventItr!=endOfEvents)
    {
      const int signal_process_id=HepMC::signal_process_id(*outputEventItr);
      const int event_number((*outputEventItr)->event_number());
      ATH_MSG_INFO ( "GenEvent #"<<event_number<<", signal_process_id="<<signal_process_id<</*", category="<<event->second<<*/", number of Vertices="<<(*outputEventItr)->vertices_size() );
      char fname[80];
      sprintf(fname,"%s.event%d.txt",m_truthCollInputKey.value().c_str(),event_number);
      std::ofstream of(fname);
      HepMC::Print::line(of,*(*outputEventItr)); // verbose output
      of.close();
      ++outputEventItr;
    }
  ATH_MSG_INFO ( "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" );
  return;
}

