/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef eTowerMakerFromEfexTowers_H
#define eTowerMakerFromEfexTowers_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "L1CaloFEXToolInterfaces/IeTowerBuilder.h"

namespace LVL1 {

class eTowerMakerFromEfexTowers : public AthReentrantAlgorithm
{
 public:

  eTowerMakerFromEfexTowers(const std::string& name, ISvcLocator* pSvcLocator);
  ~eTowerMakerFromEfexTowers() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:

    SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainerSGKey {this, "InputTowers", "L1_eFexDataTowers", "efexTowers (use L1_eFexEmulatedTowers for built from SC, or L1_eFexDataTowers for efex readout"};
    // the use-case for this second input tower collection is in monitoring when running in prescaled readout mode
    // we want to use the input data readout when we have it, but otherwise fallback to the calo readout
    SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainer2SGKey {this, "SecondaryInputTowers", "L1_eFexEmulatedTowers", "If specified, will fallback to this collection of towers if the first collection is incomplete"};
    UnsignedIntegerProperty m_minTowersRequired {this,"MinTowersRequired",1,"Will use the primary collection provided there's at least this many towers there"};

    SG::WriteHandleKey<LVL1::eTowerContainer> m_eTowerContainerSGKey {this, "MyETowers", "eTowerContainer", "MyETowers"};
    ToolHandle<IeTowerBuilder> m_eTowerBuilderTool {this, "eTowerBuilderTool", "LVL1::eTowerBuilder", "Tool that builds eTowers for simulation"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_noiseCutsKey{this,"NoiseCutsKey","",
                                                                 "Key to noise cuts (AttrListCollection)"};
    UnsignedIntegerProperty m_noiseCutBeginTimestamp {this,"NoiseCutsBeginTimestamp",1672531200,"Earliest timestamp that noise cuts will be applied from db. Default is start of 2023"};

    mutable std::atomic<bool> m_printedNoiseCuts = false;

    mutable std::mutex m_debugMutex;

};

} // end of LVL1 namespace
#endif
