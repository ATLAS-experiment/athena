/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSim_READOUTPUTHEADERTOOL_H
#define FPGATrackSim_READOUTPUTHEADERTOOL_H

/**
 * @file FPGATrackSimOutputHeaderTool.h
 *
 * This class reads/write FPGATrackSim output data from/to a ROOT file 
 * Designed to be not thread-safe
 */

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ITHistSvc.h"

#include "TFile.h"
#include "TTree.h"

#include <numeric>
#include <atomic>

class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;

class FPGATrackSimOutputHeaderTool : public AthAlgTool
{

public:

  FPGATrackSimOutputHeaderTool(const std::string&, const std::string&, const IInterface*);
  virtual ~FPGATrackSimOutputHeaderTool()  = default;
  virtual StatusCode initialize() override; 
  virtual StatusCode finalize()   override;

  // Helpers to add branches for reading or writing.. Make this completely generic.
  FPGATrackSimLogicalEventInputHeader* addInputBranch(const std::string& branchName, bool write = true);
  FPGATrackSimLogicalEventOutputHeader* addOutputBranch(const std::string& branchName, bool write = true);

  // Helper function; part of initialize that actually sets up the branches for reading.
  StatusCode configureReadBranches();

  // Actually read or write the corresponding objects.
  StatusCode readData(bool &last);
  StatusCode writeData();

  // Not sure I understand why this is done like this.
  std::string fileName() { return std::accumulate(m_inpath.value().begin(), m_inpath.value().end(), std::string{}); }

  // Not sure this is needed, but it was in the interface.
  TTree* getEventTree() { return m_EventTree; };

private:
  // JO configuration (Converted to Gaudi::Property).

  // This is a vector because it allows us to read multiple files.
  // Only relevant for reading, writing multiple files isn't supported.
  Gaudi::Property<std::vector<std::string>>   m_inpath {this, "InFileName", {"."}, "input file paths"};

  // RECREATE and HEADER are both "write" options. RECREATE (re)creates the output ROOT file.
  // HEADER does not, it assumes some other tool or service has opened the file.
  Gaudi::Property<std::string> m_rwoption {this, "RWstatus", std::string("READ"), "define read or write file option: READ, RECREATE, HEADER"};

  // Name of the output tree to create, can be overridden.
  Gaudi::Property<std::string> m_treeName {this, "OutputTreeName", "FPGATrackSimLogicalEventTree", "Name of the output TTree to create."};

  // Service handle for the histogram service.
  ServiceHandle<ITHistSvc> m_tHistSvc {this, "THistSvc", "THistSvc"};

  // Max output events
  Gaudi::Property<unsigned> m_eventLimit {this, "EventLimit", 10000 , "Maximum Number of Events to Output"};

  // internal counters  
  std::atomic<unsigned> m_event = 0;
  std::atomic<unsigned> m_totevent = 0;
  std::atomic<unsigned> m_file = 0;
 
  // These were protected in the interface but I don't think they have to be.
  std::vector<FPGATrackSimLogicalEventInputHeader*>  m_eventInputHeaders;
  std::vector<FPGATrackSimLogicalEventOutputHeader*> m_eventOutputHeaders;

  std::vector<std::string> m_branchNameIns;
  std::vector<std::string> m_branchNameOuts;

  TFile *m_infile = nullptr;
  TTree *m_EventTree = nullptr;
  
  StatusCode openFile(std::string const & path);

};

#endif // FPGATrackSim_READOUTPUTHEADERTOOL_H
