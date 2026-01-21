/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFINTERFACESVC_H
#define EFINTERFACESVC_H

#include "GaudiKernel/IIncidentListener.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "eformat/eformat.h"
#include "df_ef_interface/df_ef_interface.h"
#include <boost/dll.hpp>
#include <boost/property_tree/ptree.hpp>
#include <queue>
#include <mutex>
#include <memory>

/** @class EFInterfaceSvc
 *  @brief A service managing all the communication with the df_ef interface, for online use
 *
 *  This service replaces the use of hltinterface::DataCollector in the online HLT
 *  dynamically loads proper implementation library
 *  calls interface methods for event retrieval or accept/reject decision
 *  manages the pTree for configuring the interface
 **/
class EFInterfaceSvc: public extends <AthService, IIncidentListener>
{ 
public:

  enum class Status {
    OK = 0,      ///< event returned
    NO_EVENT,    ///< no event available
    STOP         ///< stop transition (no more events)
  };

  EFInterfaceSvc(const std::string& name, ISvcLocator *svc );
  virtual ~EFInterfaceSvc() noexcept override {}

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode stop() override;
  virtual StatusCode finalize() override;

  virtual void handle(const Incident& incident) override;
  virtual Status getNext(std::unique_ptr<uint32_t[]>& rawEventPtr);
  void eventDone(std::unique_ptr<uint32_t[]> rawEventPtr);
  boost::property_tree::ptree prepInterfacePTree();

  // Event Counters
  uint32_t m_acceptedEvents{0};
  uint32_t m_rejectedEvents{0};
  uint32_t m_processedEvents{0};

private:
  boost::dll::shared_library m_efdfinterface_library; ///< Library with the df_ef_interface implementation
  std::unique_ptr<daq::df_ef_interface::EventHandler> m_eventHandler;
  std::queue<std::future<std::unique_ptr<uint32_t []>>> m_getNextFuture; 
  std::mutex m_queueMutex; ///< Mutex for future queue

  // ------------------------- Properties --------------------------------------
  Gaudi::Property<std::string> m_interface_library_name {this, "EFDFInterfaceLibraryName", "TrigDFEmulator",
    "Name of the EFDF interface shared library to load"};
  Gaudi::Property<int> m_getNextTimeout {this, "GetNextTimeout", 1000,
    "Timeout for getting the next event (in milliseconds)"};
  Gaudi::Property<int> m_stride {this, "Stride", 1,
    "Stride for the event retrieval"};
  Gaudi::Property<int> m_fileOffset {this, "FileOffset", 0,
    "File offset for the event retrieval"};
  Gaudi::Property<int> m_numEvents {this, "NumEvents", 100,
    "Number of events to process"};
  Gaudi::Property<int> m_skipEvents {this, "SkipEvents", 0,
    "Number of events to skip"};
  Gaudi::Property<bool> m_loopOverFiles {this, "LoopOverFiles", true,
    "Flag to enable looping over files"};
  Gaudi::Property<std::string> m_outputFileName {this, "OutputFileName", "test_output.data",
    "Name of the output file"};
  Gaudi::Property<std::vector<std::string>> m_files {this, "Files", {""},
    "List of input files"};
  Gaudi::Property<int> m_runNumber {this, "RunNumber", 0,
    "Run number for the events processing"};
  Gaudi::Property<int> m_triggerType {this, "TriggerType", 0,
    "Trigger type for the events processing"};
  Gaudi::Property<int> m_beamType {this, "BeamType", 0,
    "Beam type"};
  Gaudi::Property<int> m_beamEnergy {this, "BeamEnergy", 0,
    "Beam energy"};
  Gaudi::Property<std::string> m_detMask {this, "DetMask", "00000000000000000000000000000000",
    "Detector mask"};
  Gaudi::Property<std::string> m_T0_project_tag {this, "T0ProjectTag", "T0_project_tag",
    "T0 project tag"};
  Gaudi::Property<std::string> m_stream {this, "Stream", "stream",
    "Stream name"};
  Gaudi::Property<int> m_lumiblock {this, "Lumiblock", 0,
    "Lumiblock"};
};

#endif // EFINTERFACESVC_H 

