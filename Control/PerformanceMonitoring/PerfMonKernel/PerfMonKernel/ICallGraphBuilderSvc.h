///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ICallGraphBuilderSvc.h 
// Header file for class ICallGraphBuilderSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONKERNEL_ICALLGRAPHBUILDERSVC_H 
#define PERFMONKERNEL_ICALLGRAPHBUILDERSVC_H 

// STL includes


// FrameWork includes
#include "GaudiKernel/IInterface.h"

// Forward declaration

class ICallGraphBuilderSvc : virtual public IInterface
{ 
 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  DeclareInterfaceID(ICallGraphBuilderSvc, 1, 0);

  /** Destructor: 
   */
  virtual ~ICallGraphBuilderSvc();

  ///////////////////////////////////////////////////////////////////
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// open a new node in the call graph tree
  virtual void openNode( const std::string& nodeName ) = 0;

  /// close an existing node in the call graph tree
  virtual void closeNode( const std::string& nodeName ) = 0;

  /////////////////////////////////////////////////////////////////// 
  // Private methods: 
  /////////////////////////////////////////////////////////////////// 
 private: 

}; 

#endif //> PERFMONKERNEL_ICALLGRAPHBUILDERSVC_H
