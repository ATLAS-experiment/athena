/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_DCS_CONDITIONSSVC_H
#define TRT_DCS_CONDITIONSSVC_H

/** @file TRT_DCS_ConditionsSvc.h
 *  @brief Service to provide a simple Athena interface to read
 *  DCS conditions data from COOL
 * author Denver Whittington <Denver.Whittington@cern.ch>
 **/
#include <vector>
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "TRT_ConditionsServices/ITRT_DCS_ConditionsSvc.h"
#include "TRT_ConditionsServices/ITRT_ConditionsSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ICondSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TRT_ConditionsServices/ITRT_HWMappingSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

class StoreGateSvc;
class TRT_ID;
class Identifier;
class TH1D;

/// Service for accessing DCS conditions information
class TRT_DCS_ConditionsSvc : public extends<AthService,
                                             ITRT_ConditionsSvc,
                                             ITRT_DCS_ConditionsSvc>
  {

 public:

  using base_class::base_class;
  virtual ~TRT_DCS_ConditionsSvc();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  /// Returns the DCS conditions StatusFlag for a given identifier.
  InDet::TRT_DCS_StatusFlag getFlag( const Identifier );

  /// Returns the value for a given identifier and data type.
  /** Get the value for a given identifier and DCS conditions data type.
   */
  StatusCode getValue( const Identifier, const InDet::TRT_DCS_DataType,
		       InDet::TRT_DCS_ValueType& );

  /// Returns the value for a given folder/channel name.
  /** Get the value for a given folder/channel name.
   */
  StatusCode getValue( const std::string & fname, const std::string & chanName, InDet::TRT_DCS_ValueType& value);

  /// Returns the value for a given folder and COOL channel number.
  /** Get the value for a given folder and COOL channel number.
   */
  StatusCode getValue( const std::string & fname, const int chanNum, InDet::TRT_DCS_ValueType& value);

  const CondAttrListCollection* getCollection( const std::string & collName);

  void monitorBarrel();
  void monitorEndcapA();
  void monitorEndcapC();


  /// @name Functions inherited from ITRT_ConditionsSvc
  //@{

  /// Evaluation for TRT_ConditionsSummarySvc
  /** Function inherited from ITRT_ConditionsSvc.
   *  Returns a InDet::TRT_CondFlag for the Identifier, as determined
   *  by the TRT_DCS_ConditionsSvc.
   */
  InDet::TRT_CondFlag condSummaryStatus( const Identifier& );

  //@}

 private:

  // jobOptions properties
  BooleanProperty m_VeryVerbose{this, "VeryVerbose", false};
  StringProperty m_Barrel_HV_COOLFolderName{this, "Barrel_HV_COOLFolderName", "/TRT/DCS/HV/BARREL"};
  StringProperty m_EndcapA_HV_COOLFolderName{this, "EndcapA_HV_COOLFolderName", "/TRT/DCS/HV/ENDCAPA"};
  StringProperty m_EndcapC_HV_COOLFolderName{this, "EndcapC_HV_COOLFolderName", "/TRT/DCS/HV/ENDCAPC"};
  FloatProperty m_HVWarnValLo{this, "HV_WarningValueHigh", 1000.};
  FloatProperty m_HVWarnValHi{this, "HV_WarningValueLow", 2000.};
  ServiceHandle<StoreGateSvc> m_evtStore{this, "EventStore", "StoreGateSvc"};
  ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "DetectorStore"};
  ServiceHandle<ITRT_HWMappingSvc> m_mapSvc{this, "HWMapSvc", "TRT_HWMappingSvc"};
  SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKey{this,"EventInfoKey","EventInfo","RHK for EventInfo"};
  // Conditions access
  ServiceHandle<ICondSvc> m_condSvc{"CondSvc", name()};
  SG::ReadCondHandleKey<CondAttrListCollection> m_barrelReadKey{this,"BarrelKeyName","/TRT/DCS/HV/BARREL","HV Barrel in-key"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_EAReadKey{this,"EAKeyName","/TRT/DCS/HV/ENDCAPA","HV EA in-key"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_ECReadKey{this,"ECKeyName","/TRT/DCS/HV/ENDCAPC","HV EC in-key"};
  mutable std::vector<const CondAttrListCollection*> m_Barrel_HV_COOLCont ATLAS_THREAD_SAFE {nullptr}; // Guarded by m_cacheMutex
  mutable std::vector<const CondAttrListCollection*> m_EndcapA_HV_COOLCont ATLAS_THREAD_SAFE {nullptr}; // Guarded by m_cacheMutex
  mutable std::vector<const CondAttrListCollection*> m_EndcapC_HV_COOLCont ATLAS_THREAD_SAFE {nullptr}; // Guarded by m_cacheMutex
  mutable std::mutex m_cacheMutex;
  mutable std::vector<EventContext::ContextEvt_t> m_evtBA ATLAS_THREAD_SAFE {EventContext::INVALID_CONTEXT_EVT}; // Guarded by m_cacheMutex
  mutable std::vector<EventContext::ContextEvt_t> m_evtEA ATLAS_THREAD_SAFE {EventContext::INVALID_CONTEXT_EVT}; // Guarded by m_cacheMutex
  mutable std::vector<EventContext::ContextEvt_t> m_evtEC ATLAS_THREAD_SAFE {EventContext::INVALID_CONTEXT_EVT}; // Guarded by m_cacheMutex

  IntegerProperty m_IOVmaxLength{this, "IOVmaxLength", 86400 /* 24 hours */};
  BooleanProperty m_doIOVchecking{this, "DoIOVChecking", false};
  BooleanProperty m_FallBackOnCOOLChanNames{this, "FallBackOnCOOLChanNames", false};


  // Straw Helpers
  const TRT_ID* m_TRT_ID_Helper{};

  /// Counters
  /** Counts the number of times getFlag returns each flag */
  int m_numFlagRED{};
  int m_numFlagNOINFO{};

  

  // Other private variables
  int m_currentTimestamp{};

  // Monitoring histograms
  BooleanProperty m_doMonitoring{this, "DoMonitoring", false};
  TH1D* m_h_Barrel_nRED{};
  TH1D* m_h_EndcapA_nRED{};
  TH1D* m_h_EndcapC_nRED{};
  TH1D* m_h_Barrel_nNOINFO{};
  TH1D* m_h_EndcapA_nNOINFO{};
  TH1D* m_h_EndcapC_nNOINFO{};
  TH1D* m_h_Barrel_HVvalAvg{}; // don't forget to divide by nEvts at end!
  TH1D* m_h_EndcapA_HVvalAvg{}; // don't forget to divide by nEvts at end!
  TH1D* m_h_EndcapC_HVvalAvg{}; // don't forget to divide by nEvts at end!
  int m_nBAEvts{};
  int m_nEAEvts{};
  int m_nECEvts{};
};

#endif // TRT_DCS_CONDITIONSSVC_H
