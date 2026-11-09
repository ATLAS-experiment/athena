/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//JetConstituentModifierBase.h

#ifndef JETRECTOOLS_JETCONSTITUENTMODIFIERBASE_H
#define JETRECTOOLS_JETCONSTITUENTMODIFIERBASE_H

//// Michael Nelson, CERN & Univesity of Oxford
//// February, 2016

#include "AsgTools/AsgTool.h"
#include "xAODBase/IParticleContainer.h"
#include "JetInterface/IJetConstituentModifier.h"
#include "xAODJet/Jet.h"
#include "AthContainers/CurrentContext.h"


// JetConstituentModifierBase provides default execute () behaviour
// to all JetConstituentModifier tools. Can be used in Rootcore, without
// the need for a sequence.

class JetConstituentModifierBase: public asg::AsgTool, virtual public IJetConstituentModifier {
ASG_TOOL_CLASS(JetConstituentModifierBase, IJetConstituentModifier) 
public:
  JetConstituentModifierBase(const std::string & name);

  // Implement the IJetConstituentModifier interface
  // Do some type-checking on the input container
  StatusCode process(xAOD::IParticleContainer* cont, const EventContext& ctx) const override final;
  // Concrete implementation method to be overridden by child tools
  // We pass a pointer here, because some of the type-specific tools
  // will want to cast to the right container type
  virtual StatusCode process_impl(xAOD::IParticleContainer* /*cont*/, const EventContext& /*ctx*/) const
  { 
    ATH_MSG_ERROR("This is the JetConstituentModifierBase class, no corrections implemented!");
    return StatusCode::FAILURE;
  }

  virtual StatusCode configure_decor_handle_keys() {
    return StatusCode::SUCCESS;
  }

  void setInputContainer(const std::string& cont) {m_inputContainer = cont;}
  void setOutputContainer(const std::string& cont) {m_outputContainer = cont;}
  void setInputChargedContainer(const std::string& cont) {m_inputChargedContainer = cont;}
  void setOutputChargedContainer(const std::string& cont) {m_outputChargedContainer = cont;}
  void setInputNeutralContainer(const std::string& cont) {m_inputNeutralContainer = cont;}
  void setOutputNeutralContainer(const std::string& cont) {m_outputNeutralContainer = cont;}

  

protected:
  // Define four-momentum setters that will handle type-specific operations
  // Multiple versions are needed that only correct particular four-vector
  // components,  to avoid producing unnecessary shallow copy branches

  // Setter for eta/phi
  StatusCode setEtaPhi(xAOD::IParticle* obj, float eta, float phi) const;
  // Setter for e/pt
  StatusCode setEnergyPt(xAOD::IParticle* obj, float e, float pt) const;
  // Setter for full four-vector
  StatusCode setP4(xAOD::IParticle* obj, const xAOD::JetFourMom_t& p4) const;
  // Get the correction factor for a given object, based on the corrected energy and pt, or the corrected four-vector
  float getCorrectionFactor(xAOD::IParticle* obj, const float& corrected_energy, const float& corrected_pt) const;
  float getCorrectionFactor(xAOD::IParticle* obj, const xAOD::JetFourMom_t& corrected_p4) const;

  // We need to know the output type so that we can set the p4 appropriately,
  // because IParticle does not specify the p4 encoding.
  // The user must explicitly set what sort of object they modify
  // Define as a basic integer type because Gaudi
  // doesn't support arbitrary property types
  unsigned int m_inputType=xAOD::Type::Other;

  std::string m_inputContainer;
  std::string m_outputContainer;
  std::string m_inputChargedContainer;
  std::string m_inputNeutralContainer;
  std::string m_outputChargedContainer;
  std::string m_outputNeutralContainer;


  bool m_applyToChargedPFO=true;
  bool m_applyToNeutralPFO=true;
};

#endif

