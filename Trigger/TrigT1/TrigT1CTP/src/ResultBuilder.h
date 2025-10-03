/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRIGT1CTP_RESULTBUILDER_H
#define TRIGT1CTP_RESULTBUILDER_H

// local includes
//#include "./SpecialTrigger.h"
#include "./ThresholdMap.h"
#include "./ItemMap.h"

#include <stdint.h>

// For handling different CTP versions:
#include "CTPfragment/CTPdataformatVersion.h"
#include "GaudiKernel/EventIDBase.h"

#include "AthenaBaseComps/AthAlgTool.h"

// L1 configuration data
#include "TrigConfData/L1Menu.h"

// Output to the RoIB:
#include "TrigT1Interfaces/CTPSLink.h"

// Output to the DAQ:
#include "TrigT1Result/CTP_RDO.h" // TODO: obsolete it in favour of xAOD::CTPResult --> see line below
#include "xAODTrigger/CTPResult.h"
#include "xAODTrigger/CTPResultAuxInfo.h"
#include "TrigT1Interfaces/CTPResultUtils.h"

#include "CLHEP/Random/RandomEngine.h"

namespace LVL1CTP {

   /**
    *
    *   @short Class used to create the CTP output (RDO, SLink and TriggerInfo)
    *
    *   This class constructs and uses vectors of the following words to form the CTP output:
    *   - tbp: Trigger before prescale words
    *   - tap: Trigger after prescale words 
    *   - tav: Trigger after veto words 
    *   - tip: Trigger input pattern words
    *   - extra: Extra data words
    *
    */
   
   class ResultBuilder : public AthAlgTool {

   public:
      
      ResultBuilder( const std::string& type, const std::string& name, const IInterface* parent );
      ~ResultBuilder();

      void setRandomEngine( CLHEP::HepRandomEngine* rndmEngine );

      StatusCode setConfiguration( const TrigConf::L1Menu& l1menu );

      StatusCode buildItemDecision( const std::map<std::string, unsigned int> & thrMultiMap,
                                    std::map<std::string, unsigned int> & itemDecisionMap,
                                    CLHEP::HepRandomEngine* rndmEngine ) const;
      

      StatusCode constructResultVectors( const std::map<std::string, unsigned int> & itemDecisionMap,
                                         std::vector<uint32_t> & tbp, std::vector<uint32_t> & tap, std::vector<uint32_t> & tav,
                                         unsigned char & triggerType ) const;


      StatusCode constructTIPVector( const std::map<std::string, unsigned int> & thrMultiMap,
                                     std::vector<uint32_t> & tip ) const;


      //! build RoI result (LVL1CTP::CTPSLink)
      std::unique_ptr<CTPSLink> constructRoIResult( const EventIDBase & eventID, 
                                                    const std::vector<uint32_t> & tbp, const std::vector<uint32_t> & tap, 
                                                    const std::vector<uint32_t> & tav, const std::vector<uint32_t> & tip, 
                                                    const std::vector<uint32_t> & extra,
                                                    const unsigned char triggerType ) const;

      //! build RDO result (CTP_RDO) - TODO obsolete it in favor of xAOD::CTPResult
      std::unique_ptr<CTP_RDO> constructRDOResult( const EventIDBase & eventID, 
                                                   const std::vector<uint32_t> & tbp, const std::vector<uint32_t> & tap, 
                                                   const std::vector<uint32_t> & tav, const std::vector<uint32_t> & tip,
                                                   const std::vector<uint32_t> & extra ) const;

     //! build RDO result (CTPResult)
     std::pair< std::unique_ptr<xAOD::CTPResult>, std::unique_ptr<xAOD::CTPResultAuxInfo> > constructCTPResult( const EventIDBase & eventID, 
							  const std::vector<uint32_t> & tbp, const std::vector<uint32_t> & tap, 
							  const std::vector<uint32_t> & tav, const std::vector<uint32_t> & tip,
							  const std::vector<uint32_t> & extra, const unsigned char triggerType ) const;

     
   private:

      enum WrdType { TBP = 0x01, TAP = 0x02, TAV = 0x04 };

      StatusCode createTriggerConfigMaps(const TrigConf::L1Menu& l1menu);

      //! build list of fired items and dump to string
      std::vector<std::string> firedItems(const std::vector<uint32_t>& triggerWords) const;

      // configuration information
      std::unique_ptr<ThresholdMap>         m_thrConfigMap{ nullptr };    //!< Map between threshold objects and their CTP-internal description
      std::unique_ptr<ItemMap>              m_itemConfigMap{ nullptr };   //!< Map between item objects and their CTP-internal description
      InternalTriggerMap            m_internalTrigger;             //!< internal triggers BGRP and RNDM
      unsigned int                  m_ctpVersionNumber { 4 };      //!< CTP data format version (4 in most of Run 2 and in Run 3) 
      CTPdataformatVersion*         m_ctpDataFormat { nullptr };   //!< CTP data format details
   };


   // typedefs

   // generic typedef for easy access to TIP/TBP/TAP/TAV information
   typedef std::vector<uint32_t> (ResultBuilder::*Result)() const;

} // namespace LVL1CTP

#endif // TRIGT1CTP_RESULTBUILDER_H
