/**
 * @file InputVariable.cpp
 * @date 2022-06-01
 *
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *
 */

#include "JetToolHelpers/InputVariable.h"
#include <cmath> //std::abs
#include <iostream> //std::cerr

namespace JetHelper{

  InputVariable::InputVariable(const std::string& name, std::function<float(const xAOD::Jet& jet, const JetContext& jc)> func):
    InputVariable(name)
  {
    m_customFunction = std::move(func);
  }
  
  

  
  std::unique_ptr<InputVariable> createJetVariable(const std::string& name, const std::string& type,  float scale) {
    
    // Variables stored on the xAOD::Jet
    // First, check for pre-defined attributes (not stored as generic auxdata)

    if (name == "e")
      return std::make_unique<InputVariable>(name,
					     [scale](const xAOD::Jet& jet, const JetContext&) {
					       return jet.e()*scale;
					     });

    if (name == "et")
      return std::make_unique<InputVariable>(name,
					     [scale](const xAOD::Jet& jet, const JetContext&) {
					       return jet.p4().Et()*scale;
					     });

    if (name == "abseta" || name == "|eta|")
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return std::abs(jet.eta());
					     });

    if (name == "rapidity" || name == "y")
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return jet.rapidity();
					     });

    if (name == "absrapidity" || name == "|rapidity|" || name == "absy" || name == "|y|")
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return std::abs(jet.rapidity());
					     });
        
    if (name == "mass" || name == "M" || name == "m")
      return std::make_unique<InputVariable>(name,
					     [scale](const xAOD::Jet& jet, const JetContext&) {
					       return jet.m()*scale;
					     });

    if (name == "absDetEta")
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return std::abs(jet.getAttribute<float>("DetectorEta"));
					     });

    if (name == "absConstEta")
      return std::make_unique<InputVariable>(name,
                                             [](const xAOD::Jet& jet, const JetContext&) {
                                               return std::abs(jet.getAttribute<float>("JetConstitScaleMomentum_eta"));
                                             });

    if (name == "LOGmOe")
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       if(jet.m() / jet.e() <= 0){
						 return -1.e-6;
					       }
					       else{
						 return std::log(jet.m() / jet.e());
					       }
					     });
    if(name == "Tau32_wta"){
      static const SG::AuxElement::ConstAccessor<float> accTau3("Tau3_wta");
      static const SG::AuxElement::ConstAccessor<float> accTau2("Tau2_wta");
      
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return accTau2(jet) > 1e-8 ? accTau3(jet) / accTau2(jet) : -0.1; }
					     );
    }
    if(name == "Tau21_wta"){
      static const SG::AuxElement::ConstAccessor<float> accTau1("Tau1_wta");
      static const SG::AuxElement::ConstAccessor<float> accTau2("Tau2_wta");
      
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return accTau1(jet) > 1e-8 ? accTau2(jet) / accTau1(jet) : -0.1; }
					     );
    }
    if(name == "C2_forML"){
      static const SG::AuxElement::ConstAccessor<float> accC2("C2");
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return accC2(jet)>0 ? accC2(jet) : -0.1 ; }
					     );
    }    
    if(name == "D2_forML"){
      static const SG::AuxElement::ConstAccessor<float> accD2("D2");
      return std::make_unique<InputVariable>(name,
					     [](const xAOD::Jet& jet, const JetContext&) {
					       return accD2(jet)>0 ? accD2(jet) : -0.1 ; }
					     );
    }    

    if (name == "log_m")
      return std::make_unique<InputVariable>(name,
					     [scale](const xAOD::Jet& jet, const JetContext&) {
					       return log(jet.m()*scale) ;}
					     );
    if (name == "log_e")
      return std::make_unique<InputVariable>(name,
					     [scale](const xAOD::Jet& jet, const JetContext&) {
					       return log(jet.e()*scale) ;}
					     );

    
    if (type == "float")
      return std::make_unique<InputVariableAttribute<float>>(name);
            
    if (type == "int")
      return std::make_unique<InputVariableAttribute<int>>(name);

    std::cerr << "\nWARNING : user requested Jet InputVariable " << name << " is unsupported\n";
    return nullptr;


  }

  std::unique_ptr<InputVariable> createJetContextVariable(const std::string& name, const std::string& type,  float ) {
    

    // Variables not stored on the xAOD::Jet
    // Here, we need only to check the type of the variable
    // The variables are then stored in string-indexed maps
                         
    if(type == "int")
      return std::make_unique<InputVariableJetContext<int>>(name);
    
    if(type == "float")
      return std::make_unique<InputVariableJetContext<float>>(name);
    
    // Unsupported type for a non-jet-level variable
    std::cerr << "\nWARNING : user requested JetContext InputVariable type" << name << " is unsupported\n"; 
    return nullptr;
  }



  std::unique_ptr<InputVariable> InputVariable::createVariable(const std::string& name, const std::string& type,  bool isJetVar, float scale) {
    if( isJetVar){
      std::unique_ptr<InputVariable> iv = createJetVariable(name,type,scale);
      if(iv) iv->setScale(scale);
      return iv;
    } else {
      std::unique_ptr<InputVariable> iv = createJetContextVariable(name,type,scale);
      if(iv) iv->setScale(scale);
      return iv;
      
    }
  }

  
} // namespace JetHelper
