/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IgFEXSim_H
#define IgFEXSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"

namespace LVL1 {

/*
Interface definition for gFEXSim
*/

  static const InterfaceID IID_IgFEXSim("LVL1::IgFEXSim", 1, 0);
  typedef  std::array<std::array<int, 40>, 32> gTowersIDs;

  class IgFEXSim : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual StatusCode execute(const EventContext& ctx,
                               const gTowersIDs& tmp_gTowersIDs_subset,
                               gFEXOutputCollection* gFEXOutputs,
                               std::vector<uint32_t>& gRhoTobWords,
                               std::vector<uint32_t>& gBlockTobWords,
                               std::vector<uint32_t>& gJetTobWords,
                               std::vector<int32_t>&  gScalarEJwojTobWords,
                               std::vector<uint32_t>& gMETComponentsJwojTobWords,
                               std::vector<uint32_t>& gMHTComponentsJwojTobWords,
                               std::vector<uint32_t>& gMSTComponentsJwojTobWords,
                               std::vector<uint32_t>& gMETComponentsNoiseCutTobWords,
                               std::vector<uint32_t>& gMETComponentsRmsTobWords,
                               std::vector<uint32_t>& gScalarENoiseCutTobWords,
                               std::vector<uint32_t>& gScalarERmsTobWordss) const = 0;

  private:

  };

  inline const InterfaceID& LVL1::IgFEXSim::interfaceID()
  {
    return IID_IgFEXSim;
  }

} // end of namespace

#endif
