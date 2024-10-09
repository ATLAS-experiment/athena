///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// CallGraphBuilderSvc.h 
// Header file for class CallGraphBuilderSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONCOMPS_CALLGRAPHBUILDERSVC_H 
#define PERFMONCOMPS_CALLGRAPHBUILDERSVC_H 

// STL includes
#include <mutex>
#include <string>
#include <stack>

// boost includes
//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/graph/adjacency_list.hpp>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/HashMap.h"

// PerfMonKernel includes
#include "PerfMonKernel/ICallGraphBuilderSvc.h"


namespace PerfMon {

class CallGraphBuilderSvc : public extends<AthService, ICallGraphBuilderSvc>
{
  ///////////////////////////////////////////////////////////////////
  // Public typedefs: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  typedef unsigned long NodeId_t;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  CallGraphBuilderSvc( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~CallGraphBuilderSvc(); 

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}


  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// open a new node in the call graph tree
  virtual void openNode( const std::string& nodeName ) override;

  /// close an existing node in the call graph tree
  virtual void closeNode( const std::string& nodeName ) override;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  CallGraphBuilderSvc();

  /// Mutex protecting the following members
  std::recursive_mutex m_mutex;

  /// a "unique identifier" for node names
  static NodeId_t m_uuid ATLAS_THREAD_SAFE;

  /// Method-name-to-UUID
  GaudiUtils::HashMap<std::string, NodeId_t> m_nameToId ATLAS_THREAD_SAFE;
  /// UUID-to-Method-name
  GaudiUtils::HashMap<NodeId_t, std::string> m_idToName ATLAS_THREAD_SAFE;

  /// stack of method names (in fact their uuid)
  std::stack<NodeId_t> m_stack ATLAS_THREAD_SAFE;

  typedef boost::adjacency_list<> CallGraph_t;
  /// the callgraph
  CallGraph_t m_graph ATLAS_THREAD_SAFE;

}; 

} // end namespace PerfMon

#endif //> PERFMONCOMPS_CALLGRAPHBUILDERSVC_H
