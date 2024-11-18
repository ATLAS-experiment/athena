/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "JetToolHelpers/HistoInput1D.h"

namespace JetHelper {

HistoInput1D::HistoInput1D(const std::string& name)
     : HistoInputBase{name}
 { 
 }

StatusCode HistoInput1D::initialize()
{
    // First deal with the input variable
    // Make sure we haven't already configured the input variable

    ATH_CHECK( m_vartool.retrieve() );

    if (m_vartool.empty())
    {
        ATH_MSG_ERROR("Failed to create the input variable");         
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
        ATH_MSG_ERROR("Failed while reading histogram from file");     
        return StatusCode::FAILURE;
    }

    if (!m_hist)
    {
        ATH_MSG_ERROR("Histogram pointer is empty after reading from file");
        return StatusCode::FAILURE;
    }

    if (m_hist->GetDimension() != 1)
    {
        ATH_MSG_ERROR("Read the specified histogram, but it has a dimension of " << m_hist->GetDimension() << " instead of the expected 1");
        return StatusCode::FAILURE;
    }

    // Determine the histogram interpolation strategy
    if (m_interpStr == "")
    {
        ATH_MSG_FATAL("No histogram interpolation type was specified. Aborting.");
        return StatusCode::FAILURE;
    }
    else if (m_interpStr == "Full")
        m_interpNum = InterpType::Full;
    else if (m_interpStr == "None")
        m_interpNum = InterpType::None;
    else if (m_interpStr == "OnlyX")
        m_interpNum = InterpType::OnlyX;
    else if (m_interpStr == "OnlyY")
    {
        ATH_MSG_FATAL("Interpolation type " << m_interpStr << " not valid for 1D histogram.");
        return StatusCode::FAILURE;
    }
    else
    {
        ATH_MSG_FATAL("Unrecognized interpolation type: " << m_interpStr << " --> options are None/Full/OnlyY/OnlyX");
        return StatusCode::FAILURE;
    }

    // TODO
    // We have both, set the dynamic range of the input variable according to histogram range
    // Low edge of first bin (index 1, as index 0 is underflow)
    // High edge of last bin (index N, as index N+1 is overflow)
    //m_inVar.SetDynamicRange(m_hist->GetXaxis()->GetBinLowEdge(1),m_hist->GetXaxis()->GetBinUpEdge(m_hist->GetNbinsX()));

    return StatusCode::SUCCESS;
}


float HistoInput1D::getValue(const xAOD::Jet& jet, const JetContext& event) const
{

    float varValue {m_vartool->getValue(jet,event)};
    
    // TODO make this configurable as error vs frozen to edges
    varValue = enforceAxisRange(*m_hist->GetXaxis(),varValue);
    
    // TODO Handle interpolation vs projected+cached interpolate vs bin content
    switch (m_interpNum)
    {
        // "Full" and "OnlyX" interpolation mean the same thing for 1D histogram
        // Use the default HistoInputBase reading function
        case InterpType::Full:
        case InterpType::OnlyX:
            return readFromHisto(varValue);
        case InterpType::None:
            // No interpolation at all
            return m_hist->GetBinContent(m_hist->GetXaxis()->FindBin(varValue));
        default:
            // Should never get here due to previous checks
            ATH_MSG_ERROR("Unsupported interpolation type for a 1D histogram");
            return 0;
    }
}


bool HistoInput1D::runUnitTests() const
{
    // TODO
    return false;
}
} // namespace JetHelper
