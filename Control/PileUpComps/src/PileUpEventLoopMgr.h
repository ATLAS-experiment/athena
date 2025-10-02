/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPEVENTLOOPMGR_H
#define PILEUPEVENTLOOPMGR_H
/** @file PileUpEventLoopMgr.h
    @brief The ATLAS event loop for pile-up applications.
    @author Paolo Calafiura
*/

// Base class headers
#include "AthenaKernel/IEventSeek.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"

// Athena headers
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/IEvtIdModifierSvc.h"
#include "PileUpTools/PileUpStream.h"
#include "PileUpTools/PileUpMisc.h"

// Gaudi headers
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include <string>

// Forward declarations
class IBeamIntensity;
class IBeamLuminosity;
class IBkgStreamsCache;
class IEvtSelector;
class IIncidentSvc;
class PileUpMergeSvc;
class StoreGateSvc;
class EventContext;
class EventID;


/** @class PileUpEventLoopMgr
    @brief The ATLAS event loop for pile-up applications.
*/

class PileUpEventLoopMgr : public extends<MinimalEventLoopMgr, IEventSeek>,
                           public AthMessaging
{
public:

  /// Standard Constructor
  PileUpEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~PileUpEventLoopMgr();

public:
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode nextEvent(int maxevt) override;
  virtual StatusCode executeEvent( EventContext &&ctx ) override;

  virtual StatusCode seek(int evt) override;
  virtual int curEvent() const override;

  using AthMessaging::msg;
  using AthMessaging::msgLvl;

private:
  /// Reference to the Algorithm Execution State Svc
  SmartIF<IAlgExecStateSvc>  m_aess;

  void modifyEventContext(EventContext& ctx, const EventID& eID, bool consume_modifier_stream);

  /// setup input and overlay selectors and iters
  StatusCode setupStreams();

  /// Run the algorithms for the current event
  virtual StatusCode executeAlgorithms(const EventContext& ctx);

  ///return the 'fake BCID' corresponding to bunchXing
  inline unsigned int getBCID(int bunchXing, unsigned int centralBCID) const {
    //FIXME to be completely safe this should should probably depend on the bunch spacing too. Perhaps that concept should be deprecated though?
    return static_cast<unsigned int>((((bunchXing + static_cast<int>(centralBCID)) % static_cast<int>(m_maxBunchCrossingPerOrbit)) + static_cast<int>(m_maxBunchCrossingPerOrbit) )  % static_cast<int>(m_maxBunchCrossingPerOrbit));
  }

  /// Incident Service
  ServiceHandle<IIncidentSvc> m_incidentSvc;

  /// Input Stream
  PileUpStream m_origStream;

  /// output store
  ServiceHandle<StoreGateSvc> m_evtStore;              // overlaid (output) event store

  ServiceHandle<IEvtSelector> m_origSel{this, "OrigSelector", "EventSelector",
    "EventSelector for original (physics) events stream"};
  ServiceHandle<IEvtSelector> m_signalSel{this, "SignalSelector", "",
    "EventSelector for signal (hard-scatter) events stream"};
  ServiceHandle<IBeamIntensity> m_beamInt{this, "BeamInt", "FlatBM",
    "The service providing the beam intensity distribution"};
  ServiceHandle<IBeamLuminosity> m_beamLumi{this, "BeamLuminosity", "LumiProfileSvc",
    "The service providing the beam luminosity distribution vs. run"};
  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc",
    "PileUp Merge Service"};
  ServiceHandle<IEvtIdModifierSvc> m_evtIdModSvc{this, "EvtIdModifierSvc", "",
    "ServiceHandle for EvtIdModifierSvc"};

  Gaudi::Property<unsigned int> m_maxBunchCrossingPerOrbit{this, "MaxBunchCrossingPerOrbit", 3564,
    "The number of slots in each LHC beam. Default: 3564."};
  Gaudi::Property<float> m_xingFreq{this, "XingFrequency", 25.0,
    "ns"};
  Gaudi::Property<int> m_firstXing{this, "firstXing", -2,
    "time of first xing / XingFrequency (0th xing is 1st after trigger)"};
  Gaudi::Property<int> m_lastXing{this, "lastXing", 1,
    "time of last xing / XingFrequency (0th xing is 1st after trigger)"};
  Gaudi::Property<float> m_maxCollPerXing{this, "MaxMinBiasCollPerXing", 23.0,
    "Set to digitization numberOfCollisions prop. for variable-mu and RunDMC jobs."};
  ToolHandleArray<IBkgStreamsCache> m_caches{this, "bkgCaches", {},
    "list of tools managing bkg events"};
  Gaudi::Property<bool> m_allowSubEvtsEOF{this, "AllowSubEvtsEOF", true,
    "if true(default) an EOF condition in the BkgStreamsCaches is not considered "
    "to be an error IF maxevt=-1 (loop over all available events)"};
  Gaudi::Property<bool> m_xingByXing{this, "XingByXing", false,
    "if set to true we will not cache bkg events from one xing to then next. "
    "This greatly increases the amount of I/O and greatly reduces the memory required to run a job"};
  Gaudi::Property<int> m_failureMode{this, "FailureMode", 1,
    "Controls behaviour of event loop depending on return code of"
    " Algorithms. 0: all non-SUCCESSes terminate job. "
    "1: RECOVERABLE skips to next event, FAILURE terminates job "
    "(DEFAULT). 2: RECOVERABLE and FAILURE skip to next events"};
  Gaudi::Property<bool> m_allowSerialAndMPToDiffer{this, "AllowSerialAndMPToDiffer", true,
    "When set to False, this will allow the code to reproduce serial output in an "
    "AthenaMP job, albeit with a significant performance penalty."};
  Gaudi::Property<std::string> m_evinfName{this, "EventInfoName", c_pileUpEventInfoObjName,
    "SG key for the EventInfo object"};
  Gaudi::Property<std::string> m_evinfContName{this, "EventInfoContName", c_pileUpEventInfoContName,
    "SG key for the EventInfoContainer object"};
  Gaudi::Property<uint32_t> m_mcChannelNumber{ this, "MCChannelNumber", 0,
    "sample MC channel number" };

  /// current run number
  uint32_t m_currentRun{0};
  bool m_firstRun{true};

  int m_nevt{0};
  int m_ncurevt{0};
  bool m_skipExecAlgs{false};
  bool m_loadProxies{true};

};
#endif // PILEUPTOOLS_PILEUPEVENTLOOPMGR_H
