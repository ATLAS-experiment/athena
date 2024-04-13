/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Header file for class VP1Alg                           //
//                                                         //
//  update: Riccardo-Maria BIANCHI <rbianchi@cern.ch>      //
//          23 May 2014                                    //
//                                                         //
//  This is the Athena algorithm starting the production   //
//  of event files for VP1 Live, the online 3D event       //
//  display at P1.
//
//  Major updates:
//  - 2022 Apr, Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
//              Fixed the bug with large event numbers, moving to 'unsigned long long'
//
/////////////////////////////////////////////////////////////

#ifndef VP1ALGS_VP1EVENTPROD
#define VP1ALGS_VP1EVENTPROD

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ServiceHandle.h"
#include "EventDisplaysOnline/IOnlineEventDisplaysSvc.h"

#include <string>

class VP1EventProd: public AthAlgorithm,
		    public IIncidentListener
{
 public:
  VP1EventProd(const std::string& name, ISvcLocator* pSvcLocator);
  ~VP1EventProd();

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  void handle(const Incident& inc);

 private:

  Gaudi::Property<bool> m_isOnline {this, "IsOnline", false, "If running at point 1"};
  ServiceHandle<IOnlineEventDisplaysSvc> m_onlineEDsvc{this, "OnlineEventDisplaysSvc", "Online Event Displays Service"};

  // run/event number to be used in the vp1 event file name
  unsigned long m_runNumber;
  unsigned long long  m_eventNumber;

  unsigned long m_timeStamp;
  std::string m_humanTimestamp;
  std::string m_outputFileType;
  bool m_removeTempInputFiles;

  // properties
  std::string m_inputPoolFile;
  std::string m_destinationDir;
  bool m_createDestinationDir;
  int m_maxProducedFiles;
  int m_nEvent; // Internal counter for the number of processed events
};

#endif
