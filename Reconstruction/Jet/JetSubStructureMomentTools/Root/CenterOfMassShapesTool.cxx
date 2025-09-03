/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/CenterOfMassShapesTool.h"
#include "JetSubStructureUtils/Thrust.h"
#include "JetSubStructureUtils/FoxWolfram.h"
#include "JetSubStructureUtils/SphericityTensor.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include <string>


CenterOfMassShapesTool::CenterOfMassShapesTool(const std::string& name) : 
  JetSubStructureMomentToolsBase(name)
{
}

StatusCode CenterOfMassShapesTool::initialize() {
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("NSubjettinessTool needs to have its input jet container name configured!");
    return StatusCode::FAILURE;
  }

  m_ThrustMin_Key = m_jetContainerName + "." + m_prefix + m_ThrustMin_Key.key();
  m_ThrustMaj_Key = m_jetContainerName + "." + m_prefix + m_ThrustMaj_Key.key();

  for(unsigned int i=0; i<5; i++)
    m_FoxWolfram_Keys.emplace_back(m_jetContainerName + "." + m_prefix +
				   "FoxWolfram" + std::to_string(i));

  m_Sphericity_Key = m_jetContainerName + "." + m_prefix + m_Sphericity_Key.key();
  m_Aplanarity_Key = m_jetContainerName + "." + m_prefix + m_Aplanarity_Key.key();

  ATH_CHECK(m_ThrustMin_Key.initialize());
  ATH_CHECK(m_ThrustMaj_Key.initialize());
  ATH_CHECK(m_FoxWolfram_Keys.initialize());
  ATH_CHECK(m_Sphericity_Key.initialize());
  ATH_CHECK(m_Aplanarity_Key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CenterOfMassShapesTool::modify(xAOD::JetContainer& jets) const {
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_ThrustMin(m_ThrustMin_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_ThrustMaj(m_ThrustMaj_Key);

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_FoxWolfram;
  for(const auto& key : m_FoxWolfram_Keys)
    wdhs_FoxWolfram.emplace_back(key);

  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Sphericity(m_Sphericity_Key);
  SG::WriteDecorHandle<xAOD::JetContainer, float> wdh_Aplanarity(m_Aplanarity_Key);

  for(const xAOD::Jet* injet : jets){

    fastjet::PseudoJet jet;
    bool decorate = SetupDecoration(jet, *injet);

    std::map<std::string, double> res_t, res_fox, res_s;
  
    res_t["ThrustMin"]     = -999;
    res_t["ThrustMaj"]     = -999;
    res_fox["FoxWolfram0"] = -999;
    res_fox["FoxWolfram1"] = -999;
    res_fox["FoxWolfram2"] = -999;
    res_fox["FoxWolfram3"] = -999;
    res_fox["FoxWolfram4"] = -999;
    res_s["Sphericity"]    = -999;
    res_s["Aplanarity"]    = -999;

    if (decorate) {
      JetSubStructureUtils::Thrust t;
      JetSubStructureUtils::FoxWolfram foxwolfram;
      JetSubStructureUtils::SphericityTensor sphericity;
      res_t   = t.result(jet);
      res_fox = foxwolfram.result(jet);
      res_s   = sphericity.result(jet);
    }

    wdh_ThrustMin(*injet) = res_t["ThrustMin"];
    wdh_ThrustMaj(*injet) = res_t["ThrustMaj"];

    for(unsigned int i=0; i<5; i++)
      wdhs_FoxWolfram[i](*injet) = res_fox["FoxWolfram"+std::to_string(i)];

    wdh_Sphericity(*injet) = res_s["Sphericity"];
    wdh_Aplanarity(*injet) = res_s["Aplanarity"];
  }

  return StatusCode::SUCCESS;
}

