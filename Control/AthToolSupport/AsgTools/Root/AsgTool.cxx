/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolStore.h"

#ifndef XAOD_STANDALONE
namespace {

// Helpers to pack/unpack the PTR#TYPE/NAME string set up by the macros
// in AsgToolMacros.h.

std::string getType (const std::string& s)
{
  std::string::size_type pos1 = s.find('#');
  if (pos1 == std::string::npos)
    pos1 = 0;
  else
    ++pos1;
  std::string::size_type pos2 = s.find('/', pos1);
  if (pos2 != std::string::npos)
    return s.substr (pos1, pos2-pos1);
  return "";
}

std::string getName (const std::string& s)
{
  std::string::size_type pos = s.find('/');
  if (pos == std::string::npos)
    return s;
  return s.substr (pos+1, std::string::npos);
}

const IInterface* getParent (const std::string& s)
{
  std::string::size_type pos = s.find('#');
  if (pos == std::string::npos)
    return Gaudi::svcLocator()->service( "ToolSvc" );
  void* p;
  std::string ss = s.substr (0, pos);
  sscanf (ss.c_str(), "%p", &p);
  if (p != nullptr)
    return reinterpret_cast<IInterface*>(p);
  return Gaudi::svcLocator()->service( "ToolSvc" );
}

} // anonymous namespace
#endif // not XAOD_STANDALONE


namespace asg {

   AsgTool::AsgTool( const std::string& name )
      : AsgToolBase(
#ifndef XAOD_STANDALONE
                    ::getType(name), ::getName(name), ::getParent(name)
#else // not XAOD_STANDALONE
                    name
#endif // not XAOD_STANDALONE
                    )
#ifdef XAOD_STANDALONE
      , m_event()
#endif // XAOD_STANDALONE
   {
      ToolStore::put( this ).ignore(); // Register the tool in the ToolStore
   }

   AsgTool::~AsgTool() {

      ToolStore::remove( this ).ignore(); // Remove the tool from the ToolStore
   }

#ifdef XAOD_STANDALONE

   SgEvent* AsgTool::evtStore() const {

      return &m_event;
   }

#endif // XAOD_STANDALONE

   /// Instead of using this, weirdly named function, user code should get
   /// the string name of the current minimum message level (in case they
   /// really need it...), with:
   ///
   /// <code>
   ///   MSG::name( msg().level() )
   /// </code>
   ///
   /// This function's name doesn't follow the ATLAS coding rules, and as such
   /// will be removed in the not too distant future.
   ///
   /// @returns The string name of the current minimum message level that's
   ///          printed
   ///
   const std::string& AsgTool::msg_level_name() const {

      return MSG::name( msg().level() );
   }

   const std::string& AsgTool::getName( const void* ptr ) const {

#ifdef XAOD_STANDALONE
      // In case we use @c xAOD::Event, we have a direct function call
      // for this.
      return evtStore()->event()->getName( ptr );
#else
      const SG::DataProxy* proxy = evtStore()->proxy( ptr );
      static const std::string dummy = "";
      return ( proxy == nullptr ? dummy : proxy->name() );
#endif // XAOD_STANDALONE
   }

   SG::sgkey_t AsgTool::getKey( const void* ptr ) const {

#ifdef XAOD_STANDALONE
      // In case we use @c xAOD::Event, we have a direct function call
      // for this.
      return evtStore()->event()->getKey( ptr );
#else
      const SG::DataProxy* proxy = evtStore()->proxy( ptr );
      return ( proxy == nullptr ? 0 : proxy->sgkey() );
#endif // XAOD_STANDALONE
   }

   void AsgTool::print() const {

      ATH_MSG_INFO( "AsgTool " << name() << " @ " << this );
      return;
   }

} // namespace asg
