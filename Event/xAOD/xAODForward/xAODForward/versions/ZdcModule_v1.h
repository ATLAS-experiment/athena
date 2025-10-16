// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODFORWARD_VERSIONS_ZDCMODULE_V1_H
#define XAODFORWARD_VERSIONS_ZDCMODULE_V1_H

// EDM include(s):
#include "AthContainers/AuxElement.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include <stdint.h>

namespace xAOD {
  
  /// Class containing ZDC Module information
  ///
  /// For information how to retrieve, and to some level use this object,
  /// see the detailed package description.
  ///
  /// @author Peter Steinberg <peter.steinberg@bnl.gov>
  ///
  
  class ZdcModule_v1 : public SG::AuxElement {
    
  public:
        
    /// Default constructor
    ZdcModule_v1();
    
    uint32_t zdcId() const;
    void setZdcId(uint32_t );
    int zdcSide() const;
    void setZdcSide(int );
    int zdcModule() const;
    void setZdcModule(int );
    int zdcType() const;
    void setZdcType(int );
    int zdcChannel() const;
    void setZdcChannel(int );

    
    void setWaveform(const std::string&, const std::vector<uint16_t>&);
    const std::vector<uint16_t>& getWaveform(const std::string&) const;
    

   }; // class ZdcModule_v1

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::ZdcModule_v1, SG::AuxElement);

#endif // XAODFORWARD_VERSIONS_ZDCMODULE_V1_H
