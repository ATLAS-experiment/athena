// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCNVINTERFACES_IEVENTINFOCNVTOOL_H
#define XAODCNVINTERFACES_IEVENTINFOCNVTOOL_H

// Gaudi/Athena include(s):
#include "GaudiKernel/IAlgTool.h"

// Forward declaration(s):
namespace xAOD {
    class EventInfo_v1;
    typedef EventInfo_v1 EventInfo;
}
class EventContext;
class EventInfo;

namespace xAODMaker {

   /// The interface provided by IEventInfoCnvTool
   static const InterfaceID
   IID_IEventInfoCnvTool( "xAODMaker::IEventInfoCnvTool", 1, 0 );

   /**
    *  @short Interface for the tool creating xAOD::EventInfo from an AOD
    *
    *         This interface is implemented by the tool that converts the
    *         EventInfo object from an existing POOL/BS file into
    *         an xAOD::EventInfo object.
    *
    * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
    *
    */
   class IEventInfoCnvTool : public virtual IAlgTool {

   public:
      /// Function that fills an existing xAOD::EventInfo object with data
      virtual StatusCode convert( const EventContext& ctx,
                                  const EventInfo* aod,
                                  xAOD::EventInfo* xaod,
                                  bool pileUpInfo = false,
                                  bool copyPileUpLinks = true ) const = 0;

      /// Gaudi interface definition
      static const InterfaceID& interfaceID() {
         return IID_IEventInfoCnvTool;
      }

   }; // class IEventInfoCnvTool

} // namespace xAODMaker

#endif // XAODCNVINTERFACES_IEVENTINFOCNVTOOL_H
