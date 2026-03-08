/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GoodRunsLists_GoodRunsListSelectorTool_H
#define GoodRunsLists_GoodRunsListSelectorTool_H

/** @file GoodRunsListSelectorTool.h
 *  @brief This file contains the class definition for the GoodRunsListSelectorTool class.
 *  @author Max Baak <mbaak@cern.ch>
 **/

#include "GoodRunsLists/IGoodRunsListSelectorTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthenaEvtLoopPreSelectTool.h"
#include "GoodRunsLists/RegularFormula.h"
#include "GoodRunsLists/TGoodRunsListReader.h"
#include "GoodRunsLists/TGRLCollection.h"
#include <vector>
#include <string>
#include <map>
#include <memory>


/** @class GoodRunsListSelectorTool
 *  @brief This file contains the class definition for the GoodRunsListSelectorTool class.
 **/


class GoodRunsListSelectorTool : public extends<AthAlgTool, IGoodRunsListSelectorTool, IAthenaEvtLoopPreSelectTool>
{
 public:    
  //Delegate constructor
  using base_class::base_class;
  
  virtual ~GoodRunsListSelectorTool() {};

  /// Initialize AlgTool
  StatusCode initialize();
  /// called for each event by EventSelector to decide if the event should be passed
  bool passEvent(const EventIDBase& pEvent) ;
  /// Finalize AlgTool
  StatusCode finalize();

  /// called for each event by GoodRunsListSelectorAlg to decide if the event should be passed
  bool passRunLB( int runNumber, int lumiBlockNr,
                  const std::vector<std::string>& grlnameVec=std::vector<std::string>(),
                  const std::vector<std::string>& brlnameVec=std::vector<std::string>() ) ;
  /// called for each event by GoodRunsListSelectorAlg to decide if the event should be passed
  bool passThisRunLB( const std::vector<std::string>& grlnameVec=std::vector<std::string>(), 
                      const std::vector<std::string>& brlnameVec=std::vector<std::string>() ) ;
  /// register grl/brl combination 
  bool registerGRLSelector(const std::string& name, const std::vector<std::string>& grlnameVec, const std::vector<std::string>& brlnameVec);
  /// get GRL selector registry
  const std::map< std::string, vvPair >& getGRLSelectorRegistry() { return m_registry; }

  /// get grl/brl collection
  const Root::TGRLCollection* getGRLCollection() const { return m_grlcollection.get(); } 
  const Root::TGRLCollection* getBRLCollection() const { return m_brlcollection.get(); }

 protected:

  bool fileExists(const char* fileName);

  Gaudi::Property<std::vector<std::string> > m_goodrunslistVec{this, "GoodRunsListVec", {}, "list of input xml files"};
  Gaudi::Property<std::vector<std::string> > m_blackrunslistVec{this,"BlackRunsListVec", {}, "list of input xml files"}; 


  std::unique_ptr<Root::TGRLCollection> m_grlcollection{new Root::TGRLCollection()};
  std::unique_ptr<Root::TGRLCollection> m_brlcollection{new Root::TGRLCollection()};

  std::unique_ptr<Root::TGoodRunsListReader> m_reader{new Root::TGoodRunsListReader()};

  Gaudi::Property<int>  m_boolop{this,"BoolOperation",0};
  Gaudi::Property<bool> m_passthrough{this,"PassThrough",true};
  Gaudi::Property<bool> m_rejectanybrl{this,"RejectBlackRunsInEventSelector",false};
  Gaudi::Property<bool> m_eventselectormode{this,"EventSelectorMode",false};

  std::map< std::string, vvPair > m_registry;

};

#endif

