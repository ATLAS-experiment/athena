/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHSIMALGS_SIMPLEMERGEMCEVENTCOLLTOOL_H
#define MCTRUTHSIMALGS_SIMPLEMERGEMCEVENTCOLLTOOL_H

#include "PileUpTools/PileUpToolBase.h"

#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include <utility> /* pair */

class McEventCollection;

#include "AtlasHepMC/GenParticle_fwd.h"
#include "AtlasHepMC/GenVertex_fwd.h"
/** @class SimpleMergeMcEventCollTool
 *  @brief a PileUpTool to merge MC truth collection in the overlay store
 *
 *  $Id:
 *  @author jchapman@cern.ch
 *
 */
class SimpleMergeMcEventCollTool : public PileUpToolBase {
public:
  SimpleMergeMcEventCollTool(const std::string& type,
                 const std::string& name,
                 const IInterface* parent);
  virtual StatusCode initialize() override final;
  ///called before the subevts loop. Not (necessarily) able to access
  ///SubEvents
  virtual StatusCode prepareEvent(const EventContext& ctx, unsigned int nInputEvents) override final;
  ///called at the end of the subevts loop. Not (necessarily) able to access
  ///SubEvents
  virtual StatusCode mergeEvent(const EventContext& ctx) override final;
  ///called for each active bunch-crossing to process current SubEvents
  /// bunchXing is in ns
  virtual StatusCode
    processBunchXing(int /*bunchXing*/,
                     SubEventIterator bSubEvents,
                     SubEventIterator eSubEvents) override final;
  /// return false if not interested in  certain xing times (in ns)
  /// implemented by default in PileUpToolBase as FirstXing<=bunchXing<=LastXing
  //  virtual bool toProcess(int bunchXing) const;

  virtual StatusCode processAllSubEvents(const EventContext& ctx) override final;

private:
  //** Ensure that any GenEvent::HeavyIon info is stored in the signal GenEvent.
  StatusCode saveHeavyIonInfo(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection);
  //** Add the required information from the current GenEvent to the output McEventCollection
  StatusCode processEvent(const McEventCollection *pMcEvtColl, McEventCollection *outputMcEventCollection, const int currentBkgEventIndex, int bunchCrossingTime, int pileupType);
  //** Print out detailed debug info if required.
  void printDetailsOfMergedMcEventCollection(McEventCollection *outputMcEventCollection) const;
  //** Handle for the PileUpMergeSvc (provides input McEventCollections)
  ServiceHandle<PileUpMergeSvc> m_pMergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc", ""};
  StringProperty m_truthCollOutputKey{this, "TruthCollOutputKey", "TruthEvent", "Name of output McEventCollection"};
  McEventCollection* m_outputMcEventCollection{};
  //** Name of input McEventCollection
  StringProperty m_truthCollInputKey{this, "TruthCollInputKey", "TruthEvent", ""};
  //** Override the event numbers to be the current background event index
  BooleanProperty m_overrideEventNumbers{this, "OverrideEventNumbers", false, ""};
  //** Just save the Signal GenEvent
  BooleanProperty m_onlySaveSignalTruth{this, "OnlySaveSignalTruth", false, "Just save the Signal GenEvent"};
  //** Bool to indicate that the next GenEvent is a new signal event
  bool m_newevent{true};
  //** The total number of GenEvents that will be passed for the current signal event
  unsigned int m_nInputMcEventColls{0};
  //** How many background events have been read so far for this signal event
  unsigned int m_nBkgEventsReadSoFar{0};
};
#endif //MCTRUTHSIMALGS_SIMPLEMERGEMCEVENTCOLLTOOL_H
