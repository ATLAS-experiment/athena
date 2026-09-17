// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_VERSIONS_TRIGCONFKEYS_V1_H
#define XAODTRIGGER_VERSIONS_TRIGCONFKEYS_V1_H

// System include(s):
extern "C" {
#   include <stdint.h>
}

namespace xAOD {

   /// Class describing the configuration used in a given event
   ///
   /// One object of this type needs to be put into each xAOD event in order
   /// to make it easy to identify which trigger configuration describes that
   /// given event.
   ///
   /// Since it doesn't make any sense to slim this object, it doesn't actually
   /// use the auxiliary store infrastructure.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 630856 $
   /// $Date: 2014-11-25 12:22:49 +0100 (Tue, 25 Nov 2014) $
   ///
   class TrigConfKeys_v1 {

   public:
      /// Default constructor
      TrigConfKeys_v1( uint32_t smk = 0, uint32_t l1psk = 0,
                       uint32_t hltpsk = 0 );

      /// Get the Super Master Key of this configuration
      uint32_t smk() const;
      /// Set the Super Master Key of this configuration
      void setSMK( uint32_t value );

      /// Get the LVL1 prescale key of this configuration
      uint32_t l1psk() const;
      /// Set the LVL1 prescale key of this configuration
      void setL1psk( uint32_t value );

      /// Get the HLT prescale key of this configuration
      uint32_t hltpsk() const;
      /// Set the HLT prescale key of this configuration
      void setHLTpsk( uint32_t value );

   private:
      /// The Super Master Key of this configuration
      uint32_t m_smk;
      /// The LVL1 prescale key of this configuration
      uint32_t m_l1psk;
      /// The HLT prescale key of this configuration
      uint32_t m_hltpsk;

   }; // class TrigConfKeys_v1

} // namespace xAOD

// Need to declare this even if there are no bases in order to put this
// in a TypelessDataBucket during trigger deserialization.
#include "xAODCore/BaseInfo.h"
SG_BASES(xAOD::TrigConfKeys_v1);


#endif // XAODTRIGGER_VERSIONS_TRIGCONFKEYS_V1_H
