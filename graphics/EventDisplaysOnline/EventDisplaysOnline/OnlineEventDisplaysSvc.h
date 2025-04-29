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



class OnlineEventDisplaysSvc : public extends<AthService,
                                              IOnlineEventDisplaysSvc,
                                              IIncidentListener> {
public:

  OnlineEventDisplaysSvc( const std::string& name, ISvcLocator* pSvcLocator );

  StatusCode initialize() override;
  StatusCode finalize() override;
  void beginEvent();
  void endEvent();
  void handle(const Incident& incident ) override;
  void createWriteableDir(const std::string& directory, gid_t zpgid);
  gid_t setOwnershipToZpGrpOrDefault();
  std::string getFileNamePrefix() override;
  std::string getEntireOutputStr() override;
  std::string getStreamName() override;

private:
  OnlineEventDisplaysSvc();
  SG::ReadHandleKey<xAOD::EventInfo> m_evt{this, "EventInfo", "EventInfo", "Input event information"};
  Gaudi::Property<std::string> m_outputDirectory {this, "OutputDirectory", "/atlas/EventDisplayEvents", "Output Directory"};
  Gaudi::Property<std::vector<std::string>> m_streamsWanted {this, "StreamsWanted", {}, "Desired trigger streams"};
  Gaudi::Property<std::vector<std::string>> m_publicStreams {this, "PublicStreams", {}, "Streams that can be seen by the public"};
  Gaudi::Property<std::string> m_projectTag {this, "ProjectTag", "", "Is needed to add streams to the Public trigger streams"};
  Gaudi::Property<bool> m_BeamSplash {this, "BeamSplash", false, "Is a beam splash event"};
  Gaudi::Property<bool> m_CheckPair {this, "CheckPair", false, "Check for matching ESD and JiveXML files"};
  Gaudi::Property<int> m_maxEvents {this, "MaxEvents", 200, "Number of events to keep per stream"};
  std::string m_FileNamePrefix = "JiveXML";
  std::string m_outputStreamDir = ".Unknown";
  std::string m_entireOutputStr = ".";
  int m_runNumber{};
  long m_eventNumber{};

};

#endif
