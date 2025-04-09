/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// SGAudSvc.h 
// Header file for class SGAudSvc
// Author: Ilija Vukotic <ivukotic@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef SGCOMPS_SGAUDSVC_H
#define SGCOMPS_SGAUDSVC_H

// STL includes
#include <string>
#include <iosfwd>
#include <map>
#include <set>
#include <fstream>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IClassIDSvc.h"

#include "AthenaKernel/ISGAudSvc.h"

// Forward declaration
class ISvcLocator;
class IChronoStatSvc;
class IAlgContextSvc;
class AlgContextSvc;


/**
 * This service gives a graphical representation of algorithms accessing StoreGate.
 * SGAudSvc instruments retrieve and record functions of StoreGate and from there gets
 * names of objects accessed. Upon getting an object name it asks AlgContexSvc for the
 * name of the current algorithm. At the end of run, an ASCII file is produced (SGAudSvc.out).
 *
 * By default data are not collected for the first three events.
 */
class SGAudSvc : public extends<AthService,
                                ISGAudSvc, IIncidentListener>
{
public:

  /// Constructor with parameters: 
  SGAudSvc( const std::string& name, ISvcLocator* pSvcLocator );

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}

/** incident service handle for EndEvent. Calls monitor. There should be more elegant way to get number of events passed.
*/
  virtual void handle( const Incident& incident ) override;

  // do the auditing, called from DataStore.cxx 
  virtual void SGAudit(const std::string& key, const CLID& id, 
		       const int& fnc, const int& store_id) override;

  
/** 
* @brief Gets name of curently running algorithm from AlgContextSvc.
*/
  bool SGGetCurrentAlg();
/** 
* @brief For implementing custom increased granularity auditing of for instance tools.
*/
  virtual void setFakeCurrentAlg(const std::string&) override;
/** 
* @brief For implementing custom increased granularity auditing of for instance tools.
*/
  virtual void clearFakeCurrentAlg() override;
 
  private: 

  /// Default constructor: 
  SGAudSvc();

  void SGAudRETRIEVE(std::string SGobject);
  void SGAudRECORD(std::string SGobject);

  void getNobj(const std::string& name);
  void addRead();
  void addWrite();

  /// just counts events. called at EndEvent incident
  void monitor();

  void writeJSON();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  Gaudi::Property<std::string> m_outFileName{this, "OutFileName", "SGAudSvc.out",
    "Name of the output file to hold SGAudSvc data"};

  Gaudi::Property<std::string> m_allFileName{ "FullFileName", "",
    "Name of the output file to hold the full SG aud data"};

  Gaudi::Property<std::string> m_sumFileName{this, "SummaryFileName", "",
    "Name of the output file to hold the summary output in json format"};

  Gaudi::Property<bool> m_ignoreFakeAlgs{this, "IgnoreFakeAlgs", false,
    "Set to ignore any attempts to override current-alg"};

  Gaudi::Property<int> m_startEvent{this, "StartEvent", m_startEvent = 3,
    "Event number to start recording data"};

  Gaudi::Property<bool> m_useCLID{this, "UseCLID", true,
    "Use CLID or DataObj name in Summary File"};

  /// Pointer to the @c AlgContextScv
  ServiceHandle<IAlgContextSvc> p_algCtxSvc;
  ServiceHandle<IClassIDSvc> m_pCID;
  
  /// Vector of accessed SG objects names
  std::vector<std::string> m_vObj;
  /// Vector of names of algorithms accessing SG
  std::vector<std::string> m_vAlg;
  /// map counting Reads of each object by each algorithm.
  std::map<int,int> m_timesRead;
  /// map counting Writes of each object by each algorithm.
  std::map<int,int> m_timesWritten;
  std::string m_currAlg;
  std::string m_currObj;
  std::string m_fakeCurrAlg;
  int m_nCurrAlg{0};
  int m_nCurrObj{0};
  int m_nEvents{0};

  // map<"alg_name", set<"cid/key"> >
  typedef std::map<std::string, std::set<std::string> > DataMap;
  DataMap m_read;
  DataMap m_write;

  std::ofstream m_ofa, m_ofs;
  bool m_inExec{false};
}; 

#endif
