// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1INTERFACES_ZDC_CTP_H
#define TRIGT1INTERFACES_ZDC_CTP_H

// std include(s):
// STL includes
#include <iosfwd>

#include <bitset>
#include <map>
#include <string>

// Gaudi includes
#include "GaudiKernel/DataObject.h"

namespace LVL1 {

   /** @class ZdcCTP
    *  @short ZDC input class to the CTP simulation
    *
    *         A StoreGate class to contain the output status of the
    *         level 1 ZDC trigger simulation for input into the CTP
    *         simulation. This class contains the trigger multiplicities
    *         and single inputs above threshold for one side.
    *
    * @author Matthew Hoppesch <mch6@illinois.edu> Based on MbtsCTP William H. Bell <w.bell@physics.gla.ac.uk>
    */
   class ZdcCTP: public DataObject {
   public:
      // default constructor
      ZdcCTP() = delete;

      // Constructor with parameters:
      ZdcCTP( unsigned int cableword0 = 0 );

      /// Destructor:
      virtual ~ZdcCTP() = default;
      /** @brief set the data that is sent on cable 0 */
      void setCableWord0( unsigned int data);

      /** @brief return the data that is sent on cable 0 */


      /**
         Returns an unsigned integer trigger word containing two 3bit
         trigger multiplicities: backward and forward triggers above
         threshold.
      */

      uint32_t cableWord0() const;

      //! dump raw object content to string
      const std::string dump() const;
      //! print object content in a human readable form to string
      const std::string print() const;

   private:
      /** A data member to contain two 3bit trigger multiplicities. */
      uint32_t m_cableWord0;

   }; // class ZdcCTP

} // namespace LVL1

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( LVL1::ZdcCTP , 232437259 , 1 )

#endif // TRIGT1INTERFACES_ZDC_CTP_H
