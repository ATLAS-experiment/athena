/* 
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   BeeKstMetaData.h
 * @author Dvij Chaitanya Mankad <dvij.mankad@cern.ch>
 *
 * @brief  Store JO metadata specific to the BPHY18 derivation
 *         designed for the K*0 (K+ pi-) e+ e- vertexing.
 */

#ifndef DERIVATIONFRAMEWORK_BeeKst_metadata_H
#define DERIVATIONFRAMEWORK_BeeKst_metadata_H

#include <string>
#include <map>
#include <vector>

#include "DerivationFrameworkBPhys/BPhysMetadataBase.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace DerivationFramework {
  ///
  /// @class  BeeKstMetaData
  /// @author Dvij Chaitanya Mankad <dvij.mankad@cern.ch.>
  ///
  /// @brief  Store JO metadata specific to the BPHY18 derivation
  ///         designed for the K*0 (K+ pi-) e+ e- vertexing.
  ///
  /// Store JO metadata specific to the BPHY18 derivation in the output 
  /// file.
  /// This class inherits from BPhysMetadataBase.
  ///
  class BeeKstMetaData : virtual public BPhysMetadataBase {
    public: 
    /// @brief Main constructor
    BeeKstMetaData(const std::string& t, const std::string& n,
		     const IInterface* p);
  }; // class
} // namespace

#endif // DERIVATIONFRAMEWORK_BeeKstMetaData_H
