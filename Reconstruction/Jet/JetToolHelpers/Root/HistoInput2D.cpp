/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "JetToolHelpers/HistoInput2D.h"

namespace JetHelper {
HistoInput2D::HistoInput2D(const std::string& name)
    : HistoInputBase{name}
{ }

StatusCode HistoInput2D::initialize()
{
    // First deal with the input variables
    // Make sure we haven't already configured the input variables

    ATH_CHECK( m_varTool1.retrieve() );
    ATH_CHECK( m_varTool2.retrieve() );

    if (m_varTool1.empty() || m_varTool2.empty())
    {
        ATH_MSG_ERROR("Failed to create input variable(s)");        
        return StatusCode::FAILURE;
    }

    // Now deal with the histogram
    // Make sure we haven't already retrieved the histogram
    if (m_hist != nullptr)
    {
        ATH_MSG_ERROR("The histogram already exists");       
        return StatusCode::FAILURE;
    }

    if (!readHistoFromFile())
    {
        ATH_MSG_ERROR("Failed while reading histogram from root file");	
	return StatusCode::FAILURE;
    }

    if (!m_hist)
    {
        ATH_MSG_ERROR("Histogram pointer is empty after reading from file");
        return StatusCode::FAILURE;
    }

    if (m_hist->GetDimension() != 2)
    {
        ATH_MSG_ERROR("Read the specified histogram, but it has a dimension of " << m_hist->GetDimension() << " instead of the expected 2");        
        return StatusCode::FAILURE;
    }

    // TODO
    // We have both, set the dynamic range of the input variable according to histogram range
    // Low edge of first bin (index 1, as index 0 is underflow)
    // High edge of last bin (index N, as index N+1 is overflow)
    //m_inVar1.SetDynamicRange(m_hist->GetXaxis()->GetBinLowEdge(1),m_hist->GetXaxis()->GetBinUpEdge(m_hist->GetNbinsX()));
    //m_inVar2.SetDynamicRange(m_hist->GetYaxis()->GetBinLowEdge(1),m_hist->GetYaxis()->GetBinUpEdge(m_hist->GetNbinsY()));

    return StatusCode::SUCCESS;
}

float HistoInput2D::getValue(const xAOD::Jet& jet, const JetContext& event) const
{
    float varValue1 {m_varTool1->getValue(jet,event)};

    varValue1 = enforceAxisRange(*m_hist->GetXaxis(),varValue1);
    
    float varValue2 {m_varTool2->getValue(jet,event)};

    varValue2 = enforceAxisRange(*m_hist->GetYaxis(),varValue2);
    
    return readFromHisto(varValue1,varValue2);
}


bool HistoInput2D::runUnitTests() const
{
    // TODO
    return false;
}

} // namespace JetHelper
