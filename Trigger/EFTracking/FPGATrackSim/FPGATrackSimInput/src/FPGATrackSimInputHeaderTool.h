/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimInputHeaderTool_H
#define FPGATrackSimInputHeaderTool_H

/**
 * @file FPGATrackSimInputHeaderTool.h
 *
 * This class reads FPGATrackSim input data from a ROOT file (wrapper file)
 * Designed to be not thread-safe
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"

#include <atomic>

class FPGATrackSimEventInputHeader;

class FPGATrackSimInputHeaderTool : public extends<AthAlgTool, IFPGATrackSimEventInputHeaderTool>  {
 public:
 
  FPGATrackSimInputHeaderTool(const std::string&, const std::string&, const IInterface*);
  virtual ~FPGATrackSimInputHeaderTool() = default;    
  virtual StatusCode initialize() override; 
  virtual StatusCode finalize()   override;
  virtual StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last) const override;
  virtual StatusCode writeData(FPGATrackSimEventInputHeader* header) const override; 
  
  
 private:
  StringArrayProperty   m_inpath          {this, "InFileName", {"."}, "input file paths"};
  StringProperty        m_rwoption        {this, "RWstatus", std::string("READ"), "define read or write file option: READ, RECREATE, HEADER"};


  //internal counters  
  mutable std::atomic<unsigned> m_event = 0;
  mutable std::atomic<unsigned> m_totevent = 0;
  mutable std::atomic<unsigned> m_file = 0;
  mutable std::mutex m_fileMutex; // Protect file operations in const methods

  std::string m_branchName;
  StatusCode openFile(std::string const & path) const;

};

#endif // FPGATrackSimInputHeaderTool_H
