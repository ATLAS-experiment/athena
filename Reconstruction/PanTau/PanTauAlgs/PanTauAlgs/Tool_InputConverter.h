/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAUALGS_TOOL_INPUTCONVERTER_H
#define PANTAUALGS_TOOL_INPUTCONVERTER_H

#include <map>
#include <vector>
#include <string>

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/ITool_InputConverter.h"

#include "xAODPFlow/PFO.h"
#include "xAODTau/TauJet.h"
#include "xAODPFlow/PFODefs.h"

namespace Rec {
    class TrackParticle;
}

namespace PanTau {
    class TauConstituent;
}


namespace PanTau {

  class Tool_InputConverter : public asg::AsgTool, virtual public PanTau::ITool_InputConverter  {

    ASG_TOOL_CLASS1(Tool_InputConverter, PanTau::ITool_InputConverter)
        
    public:
        
        Tool_InputConverter(const std::string &name);
        virtual ~Tool_InputConverter ();
        
        virtual StatusCode initialize();
        
        virtual StatusCode ConvertToTauConstituent(const xAOD::PFO* pfo,
                                                   PanTau::TauConstituent* &tauConstituent,
                                                   const xAOD::TauJet* tauJet) const;
        
    protected:
        
        //member variables 
        ToolHandle<PanTau::ITool_InformationStore> m_Tool_InformationStore{this, "Tool_InformationStore", "PanTau::Tool_InformationStore/Tool_InformationStore","Link to tool with all information"};
        Gaudi::Property<std::string> m_Tool_InformationStoreName{this, "Tool_InformationStoreName", "", "Optional Name for InformationStore instance in ABR"};
        
        virtual bool passesPreselectionEnergy(double energy) const;
        
        int     m_Config_UsePionMass = 0;
        
        double  m_Config_TauConstituents_Types_DeltaRCore = 0.0;
        double  m_Config_TauConstituents_PreselectionMinEnergy = 0.0;
       
	bool m_init=false;
  public:
	inline bool isInitialized(){return m_init;}

    }; //end class Tool_InputConverter


} //end namespace PanTau


#endif // PANTAUALGS_TOOL_INPUTCONVERTER_H
