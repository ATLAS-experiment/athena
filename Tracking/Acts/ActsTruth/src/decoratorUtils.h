/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACK_SAFEDECORATOR_H
#define ACTSTRACK_SAFEDECORATOR_H
/**
 * copied from InDetPhysValMonitoring/src/safeDecorator.h
 */



#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/EventContext.h"

#include <iostream>
#include <utility>
#include <vector>

#include <cstdlib>

namespace ActsTrk {

  // convenience method to create several decorator handles from keys
  template <class T_Cont, class T>
  std::vector<SG::WriteDecorHandle<T_Cont,T> >
  createDecorators(  const std::vector<SG::WriteDecorHandleKey<T_Cont> > &keys,
                     const EventContext &ctx) {
    std::vector<SG::WriteDecorHandle<T_Cont,T> > out;
    out.reserve(keys.size());
    for( const SG::WriteDecorHandleKey<T_Cont> &a_key : keys) {
      out.emplace_back(a_key,ctx);
      if (not out.back().isValid()) {
        std::stringstream msg;
        msg << "Failed to create decorator handdle " << a_key.key();
        throw std::runtime_error( msg.str() );
      }
    }
    return out;
  }


  // convenience method to create several decorator handle keys.
  template<class T_Parent, class T_Cont>
  void createDecoratorKeys(T_Parent &parent,
                           const SG::ReadHandleKey<T_Cont> &container_key,
                           const std::string &prefix,
                           const std::vector<std::string> &decor_names,
                           std::vector<SG::WriteDecorHandleKey<T_Cont> > &decor_out) {
    decor_out.clear();
    decor_out.reserve(decor_names.size());
    for (const std::string &a_decor_name : decor_names) {
      assert( !a_decor_name.empty() );
      decor_out.emplace_back(container_key.key()+"."+prefix+a_decor_name);
      // need to declare handles, otherwise the scheduler would not pick up the data dependencies
      // introduced by the decorations
      parent.declare(decor_out.back());
      decor_out.back().setOwner(&parent);
      decor_out.back().initialize().ignore();
    }
  }
}
#endif
