/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ONLINEEVENTDISPLAYSSVC_H
#define ONLINEEVENTDISPLAYSSVC_H

#include "AthenaBaseComps/AthService.h"
#include "EventDisplaysOnline/IOnlineEventDisplaysSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "StoreGate/ReadHandle.h"
#include "xAODEventInfo/EventInfo.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>

template <class TYPE> class SvcFactory;

class OnlineEventDisplaysSvc : public AthService, virtual public IOnlineEventDisplaysSvc, virtual public IIncidentListener {

protected:
  friend class SvcFactory<OnlineEventDisplaysSvc>;
  
public:

  OnlineEventDisplaysSvc( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~OnlineEventDisplaysSvc();
   
  static const InterfaceID& interfaceID();

  //To allow access to the IOnlineEventDisplaysSvc interface
  StatusCode queryInterface( const InterfaceID& riid, void** ppvIf );

  StatusCode initialize();
  StatusCode finalize();
  void beginEvent();
  void endEvent();
  void handle(const Incident& incident );       
  void createWriteableDir(std::string directory, gid_t zpgid);
  gid_t setOwnershipToZpGrpOrDefault();
  std::string getFileNamePrefix() override;
  std::string getEntireOutputStr() override;
  std::string getStreamName() override;
  
private:
  OnlineEventDisplaysSvc();
  SG::ReadHandleKey<xAOD::EventInfo> m_evt{this, "EventInfo", "EventInfo", "Input event information"};
  Gaudi::Property<std::string> m_outputDirectory {this, "OutputDirectory", "/atlas/EventDisplayEvents", "Output Directory"};
  Gaudi::Property<std::vector<std::string>> m_streamsWanted {this, "StreamsWanted", {}, "Desired trigger streams"};
  Gaudi::Property<std::vector<std::string>> m_publicStreams {this, "PublicStreams", {}, "Desired public streams"};
  Gaudi::Property<bool> m_sendToPublicStream {this, "SendToPublicStream", false, "Allowed to be seen by the public on atlas live"};
  Gaudi::Property<int> m_maxEvents {this, "MaxEvents", 200, "Number of events to keep per stream"};
  std::string m_FileNamePrefix = "JiveXML";
  std::string m_outputStreamDir = ".Unknown";
  std::string m_entireOutputStr = ".";
};

inline const InterfaceID& OnlineEventDisplaysSvc::interfaceID()
{ 
  return IOnlineEventDisplaysSvc::interfaceID();
}

#endif

