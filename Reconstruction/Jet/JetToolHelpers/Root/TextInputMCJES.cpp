/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "JetToolHelpers/TextInputMCJES.h"

namespace JetHelper {
TextInputMCJES::TextInputMCJES(const std::string& name)
     : MCJESInputBase{name}
{
}

StatusCode TextInputMCJES::initialize()
{

    ATH_CHECK( m_vartool1.retrieve() );
    ATH_CHECK( m_vartool2.retrieve() );

    if (m_vartool1.empty() || m_vartool2.empty())
    {
        ATH_MSG_ERROR("Failed to create the input variable");
        return StatusCode::FAILURE;
    }

    // Now deal with the TEnv, config file
    // Make sure we haven't already retrieved the file
    if (m_config != nullptr)
    {
        ATH_MSG_ERROR("The config file already exists");
        return StatusCode::FAILURE;
    }

    if (!readMCJESFromText())
    {
        ATH_MSG_ERROR("Failed while reading histogram from file"); 
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

float TextInputMCJES::getValue(const xAOD::Jet& jet, const JetContext& event) const
{
    float varValue1 {m_vartool1->getValue(jet,event)};
    float varValue2 {m_vartool2->getValue(jet,event)};


    if(m_corrName == "JES" ) return getJES(varValue1, varValue2);
    if(m_corrName == "EtaCorr" ) return getEtaCorr(varValue1, varValue2);
    if(m_corrName == "EmaxJES") return getEmaxJES(varValue2);
    
    return 0;
}
} // namespace JetHelper
