/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAU_PANTAUPROCESSOR_H
#define PANTAU_PANTAUPROCESSOR_H

// Gaudi includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "tauRecTools/TauRecToolBase.h"
#include "xAODPFlow/PFOContainer.h"

// C++ includes
#include <string>
#include <map>
#include <vector>

// PanTau includes
#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/ITool_TauConstituentGetter.h"
#include "PanTauAlgs/ITool_TauConstituentSelector.h"
#include "PanTauAlgs/ITool_DetailsArranger.h"
#include "PanTauAlgs/ITool_PanTauTools.h"

namespace PanTau 
{

  /** @class PanTauProcessor

      @author  Peter Wienemann <peter.wienemann@cern.ch>
      @author  Sebastian Fleischmann <Sebastian.Fleischmann@cern.ch>
      @author  Robindra Prabhu <robindra.prabhu@cern.ch>
      @author  Christian Limbach <limbach@physik.uni-bonn.de>
      @author  Peter Wagner <peter.wagner@cern.ch>
      @author  Lara Schildgen <schildgen@physik.uni-bonn.de>
  */  

  class PanTauProcessor : virtual public TauRecToolBase
    {
    public:

       ASG_TOOL_CLASS2( PanTauProcessor, TauRecToolBase, ITauToolBase )

       PanTauProcessor(const std::string& name);
       ~PanTauProcessor();

       virtual StatusCode initialize();
       virtual StatusCode finalize();
       virtual StatusCode executePanTau(xAOD::TauJet& pTau, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const;
       
    private:
        
        //Tools used in seed building
        ToolHandle<PanTau::ITool_InformationStore>          m_Tool_InformationStore{this, "Tool_InformationStore", "PanTau::Tool_InformationStore/Tool_InformationStore", "Tool handle to Tool_InformationStore"};
        ToolHandle<PanTau::ITool_TauConstituentGetter>      m_Tool_TauConstituentGetter{this, "Tool_TauConstituentGetter", "PanTau::Tool_TauConstituentGetter/Tool_TauConstituentGetter", "Tool handle to Tool_TauConstituentGetter"};
        ToolHandle<PanTau::ITool_TauConstituentSelector>    m_Tool_TauConstituentSelector{this, "Tool_TauConstituentSelector", "PanTau::Tool_TauConstituentSelector/Tool_TauConstituentSelector", "Tool handle to Tool_TauConstituentSelector"};
        ToolHandle<PanTau::ITool_PanTauTools>               m_Tool_FeatureExtractor{this, "Tool_FeatureExtractor", "PanTau::Tool_FeatureExtractor/Tool_FeatureExtractor", "Tool handle to Tool_FeatureExtractor"};
        
        //Tools used in seed finalizing
        ToolHandle<PanTau::ITool_PanTauTools>               m_Tool_DecayModeDeterminator{this, "Tool_DecayModeDeterminator", "PanTau::Tool_DecayModeDeterminator/Tool_DecayModeDeterminator", "Tool handle to Tool_DecayModeDeterminator"};
        ToolHandle<PanTau::ITool_DetailsArranger>           m_Tool_DetailsArranger{this, "Tool_DetailsArranger", "PanTau::Tool_DetailsArranger/Tool_DetailsArranger", "Tool handle to Tool_DetailsArranger"};

        //Tools used in seed building
	Gaudi::Property<std::string> m_Tool_InformationStoreName{this, "Tool_InformationStoreName", "", "Tool handle to Tool_InformationStore"};
	Gaudi::Property<std::string> m_Tool_TauConstituentGetterName{this, "Tool_TauConstituentGetterName", "", "Tool handle to Tool_TauConstituentGetter"};
	Gaudi::Property<std::string> m_Tool_TauConstituentSelectorName{this, "Tool_TauConstituentSelectorName", "", "Tool handle to Tool_TauConstituentSelector"};
	Gaudi::Property<std::string> m_Tool_FeatureExtractorName{this, "Tool_FeatureExtractorName", "", "Tool handle to Tool_FeatureExtractor"};
        
        //Tools used in seed finalizing
	Gaudi::Property<std::string> m_Tool_DecayModeDeterminatorName{this, "Tool_DecayModeDeterminatorName", "", "Tool handle to Tool_DecayModeDeterminator"};
	Gaudi::Property<std::string> m_Tool_DetailsArrangerName{this, "Tool_DetailsArrangerName", "", "Tool handle to Tool_DetailsArranger"}; 
        
        std::vector<double>                                 m_Config_PtBins;
        double                                              m_Config_MinPt = 0.0;
        double                                              m_Config_MaxPt = 0.0;
        
        static void                                                fillDefaultValuesToTau(xAOD::TauJet* tauJet) ;
        
        
    }; //end class
} // end of namespace

#endif 
