/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HICentralityDecorationTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "PathResolver/PathResolver.h"
#include "HICentralityDecorationTool.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>
#include <fstream>

namespace DerivationFramework
{
    HICentralityDecorationTool::HICentralityDecorationTool(const std::string& type, const std::string& name, const IInterface* parent)
        : base_class(type, name, parent) {
        }
  
        // Athena initialize and finalize
        StatusCode HICentralityDecorationTool::initialize() {
    
        // Resolve the path to the centrality definition file
        std::string resolvedPath = PathResolver::find_file(m_centralityDefinitionFile, "CALIBPATH");
    
        // Debug message to print the resolved path or indicate failure
        if (resolvedPath.empty()) {
            ATH_MSG_ERROR("Could not find centrality definition file: " << m_centralityDefinitionFile);
            return StatusCode::FAILURE;
        }

        std::ifstream infile(resolvedPath);
        if (!infile.is_open()) {
            ATH_MSG_ERROR("Could not open centrality definition file: " << resolvedPath);
            return StatusCode::FAILURE;
        }
    
        std::string line;
        for (int i = 0; i < 3; ++i) {
            std::getline(infile, line); // Skip header lines
        }
    
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::string centileStr;
            float fCal, centile;
        
            if (iss >> centileStr >> std::skipws >> fCal) {
                centileStr.pop_back(); // Remove '%' character
                centile = std::stof(centileStr);
                m_centralityPercentiles.push_back(centile);
                m_fCalValues.push_back(fCal);
            } else {
                ATH_MSG_WARNING("Could not parse line: " << line);
            }
        }

        infile.close();
        return StatusCode::SUCCESS;
    }

    StatusCode HICentralityDecorationTool::addBranches() const
    {
        // Load event EventInfo
        const xAOD::EventInfo* eventInfo = nullptr;     
        ATH_CHECK(evtStore()->retrieve( eventInfo ));

        // Set up the decorators for centrality
        const static SG::AuxElement::Decorator< float > ecCentralityMin("CentralityMin") ;
        const static SG::AuxElement::Decorator< float > ecCentralityMax("CentralityMax") ;
            
        const static SG::AuxElement::ConstAccessor<float>  acc_FCalEtA("FCalEtA");
        const static SG::AuxElement::ConstAccessor<float>  acc_FCalEtC("FCalEtC"); 

        // Calculate total FCal ET
        float total_fcal_et = (acc_FCalEtA(*eventInfo) + acc_FCalEtC(*eventInfo)) / 1.e6;

        float centralityMin = 0.0; 
        float centralityMax = 100.0;   
        bool foundRange = false;
        for (size_t i = 0; i < m_fCalValues.size(); ++i) {
            if (total_fcal_et < m_fCalValues[i]) { 
                centralityMin = m_centralityPercentiles[i];
                foundRange = true;
                break;
            }
            centralityMax = m_centralityPercentiles[i];
        }
        if (!foundRange) {
            // Top possible range
            centralityMin = 80.;
        }

        // Decorate eventInfo with centrality values    
        ecCentralityMin(*eventInfo) = centralityMin;
        ecCentralityMax(*eventInfo) = centralityMax;
            
        return StatusCode::SUCCESS;
    }
}

