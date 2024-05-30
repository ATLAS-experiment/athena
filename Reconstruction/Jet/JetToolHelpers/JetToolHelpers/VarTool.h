/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_VARTOOL_H
#define JETTOOLHELPERS_VARTOOL_H



#include "JetToolHelpers/JetContext.h"
#include "JetToolHelpers/InputVariable.h"
#include "xAODJet/Jet.h"
#include "AsgTools/AsgTool.h" //inheritance
#include "JetAnalysisInterfaces/IVarTool.h" //inheritance
#include "AsgTools/PropertyWrapper.h"

#include <memory> //unique_ptr
#include <string>

namespace JetHelper {

    /// Class VarTool
    /// This class is design to help the user by specializing the InputVariable
    /// InputVariable is fix in the initialization, and can be retrieve with getvar()
 
class VarTool : public asg::AsgTool, virtual public IVarTool
{

  ASG_TOOL_CLASS(VarTool,IVarTool)

    public:
        /// Constructor for standalone usage
        VarTool(const std::string& name);
        /// Function initialising the tool
        virtual StatusCode initialize() override;
        /// return the InputVariable ready to be use 
        const InputVariable * getvar() const {return m_v.get();};
        /// return either xAOD or context variable values
        virtual float getValue(const xAOD::Jet& jet, const JetContext& jc) const override {return m_v->getValue(jet,jc);};


    private:
        /// InputVariable for user
        std::unique_ptr<InputVariable> m_v;
        /// InputVariable properties, set in initialization
        Gaudi::Property<std::string> m_name {this,"Name", "pt"};
        Gaudi::Property<std::string> m_type {this,"Type", "float"};
        Gaudi::Property<float> m_scale {this,"Scale", 1.};
        Gaudi::Property<bool> m_isJetVar {this,"isJetVar", true};
};
} // namespace JetHelper
#endif

