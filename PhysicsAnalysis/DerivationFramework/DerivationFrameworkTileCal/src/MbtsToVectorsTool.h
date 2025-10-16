///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// MbtsToVectorsTool.h 
// Header file for class MBTSToVectors
/////////////////////////////////////////////////////////////////// 
#ifndef DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_MBTSTOVECTORSTOOL_H
#define DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_MBTSTOVECTORSTOOL_H 1

#include "TileEvent/TileContainer.h"

// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// PhysicsAnalysis/DerivationFramework/DerivationFrameworkInterfaces includes
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// STL includes
#include <string>

// Forward declaration
class TileTBID;

namespace DerivationFramework {

  class MbtsToVectorsTool: public extends<AthAlgTool, IAugmentationTool>
  { 
    
    public: 
    
      /// Constructor with parameters: 
      MbtsToVectorsTool( const std::string& type, const std::string& name, const IInterface* parent );
    
      virtual StatusCode addBranches(const EventContext& ctx) const override final;
      virtual StatusCode initialize() override final;

    private:

      Gaudi::Property<std::string> m_prefix{this, "Prefix", "mbts_"};
      Gaudi::Property<bool> m_saveEtaPhi{this, "SaveEtaPhiInfo", true};

      SG::ReadHandleKey<TileCellContainer> m_cellContainerKey{this, "CellContainer", "MBTSContainer"};

      SG::WriteHandleKey<std::vector<float> > m_energyKey{this, "Energy", "energy"};
      SG::WriteHandleKey<std::vector<float> > m_timeKey{this, "Time", "time"};
      SG::WriteHandleKey<std::vector<float> > m_etaKey{this, "Eta", "eta"};
      SG::WriteHandleKey<std::vector<float> > m_phiKey{this, "Phi", "phi"};
      SG::WriteHandleKey<std::vector<int> > m_qualityKey{this, "Quality", "quality"};
      SG::WriteHandleKey<std::vector<int> > m_typeKey{this, "Type", "type"};
      SG::WriteHandleKey<std::vector<int> > m_moduleKey{this, "Module", "module"};
      SG::WriteHandleKey<std::vector<int> > m_channelKey{this, "Channel", "channel"};

      const TileTBID* m_tileTBID = nullptr;

      static const unsigned int MAX_MBTS_COUNTER{32};
  }; 
  
}


#endif //> !DERIVATIONFRAMEWORK_DERIVATIONFRAMEWORKTILECAL_MBTSTOVECTORSTOOL_H
