/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PANTAUALGS_TOOL_TAUCONSTITUENTGETTER
#define PANTAUALGS_TOOL_TAUCONSTITUENTGETTER

#include <map>
#include <vector>
#include <string>

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "PanTauAlgs/HelperFunctions.h"
#include "PanTauAlgs/ITool_InformationStore.h"
#include "PanTauAlgs/ITool_TauConstituentGetter.h"
#include "PanTauAlgs/ITool_InputConverter.h"

#include "xAODTau/TauJet.h"

namespace PanTau {
  class TauConstituent;
}


namespace PanTau {

  class Tool_TauConstituentGetter : public asg::AsgTool, virtual public PanTau::ITool_TauConstituentGetter  {
        
    ASG_TOOL_CLASS1(Tool_TauConstituentGetter, PanTau::ITool_TauConstituentGetter)

      public:
        
    Tool_TauConstituentGetter(const std::string &name);
    virtual ~Tool_TauConstituentGetter ();
        
    virtual StatusCode initialize();
        
    virtual StatusCode GetTauConstituents(const xAOD::TauJet* tauJet,
					  std::vector<TauConstituent*>& outputList) const;
        
        
  protected:
        
    //member variables 
    ToolHandle<PanTau::ITool_InputConverter>    m_Tool_InputConverter{this, "Tool_InputConverter", "PanTau::Tool_InputConverter/Tool_InputConverter", "Link to tool to convert into TauConstituents"};

    Gaudi::Property<std::string> m_Tool_InputConverterName{this, "Tool_InputConverterName", "", "Link to tool to convert into TauConstituents"};

    bool m_init=false;

  public:
    inline bool isInitialized(){return m_init;}
        
  }; //end class ConstituentGetter

}//end namespace PanTau


#endif // PANTAUALGS_TOOL_TAUCONSTITUENTGETTER
