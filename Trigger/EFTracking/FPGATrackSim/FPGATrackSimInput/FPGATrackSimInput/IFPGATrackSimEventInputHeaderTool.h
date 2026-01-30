/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IFPGATrackSimEventInputHeaderTool.h
 *
 * This declares a basic interface for input tools which provide the FPGATrackSimEventInputHeader data
 * to all FPGATrackSim processing.
 */

#ifndef IFPGATrackSimEventInputHeaderTool_H
#define IFPGATrackSimEventInputHeaderTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TFile.h"
#include "TTree.h"


// to do: merge this with FPGATrackSimSGInput/IFPGATrackSimInputTool
// since they are both abstract interfaces

class FPGATrackSimEventInputHeader;
class IFPGATrackSimEventInputHeaderTool : virtual public ::IAlgTool 
{      
 public:
  virtual ~IFPGATrackSimEventInputHeaderTool() = default;
  DeclareInterfaceID( IFPGATrackSimEventInputHeaderTool, 1, 0);

   
  virtual StatusCode readData(FPGATrackSimEventInputHeader* header, bool &last) const = 0;
  virtual StatusCode writeData(FPGATrackSimEventInputHeader* header) const = 0;
  virtual FPGATrackSimEventInputHeader*  getEventInputHeader()   {return m_eventHeader; };
  virtual TTree* getEventTree() {return m_EventTree;};

 protected:
 
  // ROOT I/O objects modified in const readData()/writeData() methods.
  // - Modified in const methods: openFile() and readData()
  // - Thread-safety: Protected by m_fileMutex (or m_readMutex) in derived classes (FPGATrackSimInputHeaderTool, FPGATrackSimReadRawRandomHitsTool)
  mutable FPGATrackSimEventInputHeader* m_eventHeader ATLAS_THREAD_SAFE = nullptr;
  mutable TFile* m_infile ATLAS_THREAD_SAFE = nullptr;
  mutable TTree* m_EventTree ATLAS_THREAD_SAFE = nullptr;
};


#endif // IFPGATrackSimEventInputHeaderTool_H
