/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimREADRAWRANDOMHITSTOOL_H
#define FPGATrackSimREADRAWRANDOMHITSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"


class FPGATrackSimReadRawRandomHitsTool : public extends<AthAlgTool, IFPGATrackSimEventInputHeaderTool>
{
  public:

  FPGATrackSimReadRawRandomHitsTool(const std::string&, const std::string&, const IInterface*);
  virtual ~FPGATrackSimReadRawRandomHitsTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last) const override;
  virtual StatusCode writeData(FPGATrackSimEventInputHeader* header) const override; 
  virtual StatusCode finalize() override;
  
  StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last, bool doReset) const;

  private:
  // JO configuration    
  StringProperty m_inpath {this, "InFileName", "httsim_smartwrapper.root", "input path"};

  // Internal pointers
  mutable std::atomic<unsigned> m_entry = 0;
  mutable std::mutex m_readMutex; // Protect ROOT read operations in const methods
};

#endif // FPGATrackSimREADRAWRANDOMHINPUTTOOL_H
