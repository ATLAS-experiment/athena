/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Source file for the JetConstituentModSequence.h
// Michael Nelson, CERN & University of Oxford 
// Will later add the intermediate step

#include "JetRecTools/JetConstituentModSequence.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "JetRecTools/JetConstituentModifierBase.h"


#include "AsgDataHandles/WriteHandle.h"

#ifndef XAOD_ANALYSIS
#include "AthenaMonitoringKernel/Monitored.h"
#endif

JetConstituentModSequence::JetConstituentModSequence(const std::string &name):
  asg::AsgTool(name) {

#ifdef ASG_TOOL_ATHENA
  declareInterface<IJetConstituentModifier>(this);
#endif
  declareProperty("InputType", m_inputType, "The xAOD type name for the input container.");
  declareProperty("SaveAsShallow", m_saveAsShallow=true, "Save as shallow copy");

}

StatusCode JetConstituentModSequence::initialize() {
  ATH_MSG_INFO("Initializing tool " << name() << "...");
  ATH_MSG_DEBUG("initializing version with data handles");

  ATH_CHECK( m_modifiers.retrieve() );
  ATH_CHECK(m_vertexContainerKey.initialize(m_byVertex));

  // Shallow copies are not supported for by-vertex jet reconstruction
  if (m_byVertex){
    m_saveAsShallow = false;
  }

#ifndef XAOD_ANALYSIS
  ATH_CHECK( m_monTool.retrieve( DisableTool{m_monTool.empty()} ) );
#endif
  
  // Set and initialise DataHandleKeys only for the correct input type
  // Die if the input type is unsupported
  switch(m_inputType) {
  case xAOD::Type::CaloCluster:
    {
      m_inClusterKey = m_inputContainer;
      m_outClusterKey = m_outputContainer;
  
      ATH_CHECK(m_inClusterKey.initialize());
      ATH_CHECK(m_outClusterKey.initialize());

      for (auto& tool : m_modifiers) {
        auto* base = dynamic_cast<JetConstituentModifierBase*>(tool.get());
        if (base) {
          base->setInputContainer(m_inClusterKey.key());
          base->setOutputContainer(m_outClusterKey.key());
          ATH_CHECK(base->configure_decor_handle_keys());
        }
      }

      break;
    }
  case xAOD::Type::ParticleFlow:
    {
      std::string inputContainerBase = m_inputContainer;
      std::string outputContainerBase = m_outputContainer;

      // Know what the user means if they give the full input/output container name in this format
      size_t pos = inputContainerBase.find("ParticleFlowObjects");
      if(pos != std::string::npos) inputContainerBase.erase(pos);

      pos = outputContainerBase.find("ParticleFlowObjects");
      if(pos != std::string::npos) outputContainerBase.erase(pos);

      m_inChargedPFOKey = inputContainerBase + "ChargedParticleFlowObjects";
      m_inNeutralPFOKey = inputContainerBase + "NeutralParticleFlowObjects";

      m_outChargedPFOKey = outputContainerBase + "ChargedParticleFlowObjects";
      m_outNeutralPFOKey = outputContainerBase + "NeutralParticleFlowObjects";
      m_outAllPFOKey = outputContainerBase + "ParticleFlowObjects";

      ATH_CHECK(m_inChargedPFOKey.initialize());
      ATH_CHECK(m_inNeutralPFOKey.initialize());
      ATH_CHECK(m_outChargedPFOKey.initialize());
      ATH_CHECK(m_outNeutralPFOKey.initialize());
      ATH_CHECK(m_outAllPFOKey.initialize());

      for (auto& tool : m_modifiers) {
        auto* base = dynamic_cast<JetConstituentModifierBase*>(tool.get());
        if (base) {
          base->setInputChargedContainer(m_inChargedPFOKey.key());
          base->setOutputChargedContainer(m_outChargedPFOKey.key());
          base->setInputNeutralContainer(m_inNeutralPFOKey.key());
          base->setOutputNeutralContainer(m_outNeutralPFOKey.key());
          ATH_CHECK(base->configure_decor_handle_keys());
        }
      }

      ATH_CHECK(m_constituentCopyIndexKey.assign(m_outNeutralPFOKey.key() + "." + m_constituentCopyIndexKey.key()));

      break;
    }
    break;
  case xAOD::Type::TrackCaloCluster:
    m_inTCCKey = m_inputContainer;
    m_outTCCKey = m_outputContainer;

    ATH_CHECK(m_inTCCKey.initialize());
    ATH_CHECK(m_outTCCKey.initialize());
    for (auto& tool : m_modifiers) {
      auto* base = dynamic_cast<JetConstituentModifierBase*>(tool.get());
      if (base) {
        base->setInputContainer(m_inTCCKey.key());
        base->setOutputContainer(m_outTCCKey.key());
        ATH_CHECK(base->configure_decor_handle_keys());
      }
    }
    break;
  case xAOD::Type::FlowElement:
    {
      // TODO: This assumes a PFlow-style neutral and charged collection.
      //       More general FlowElements (e.g. CaloClusters) may necessitate a rework here later.

      const std::string subString = "ParticleFlowObjects";
      const std::string subStringCharged = "ChargedParticleFlowObjects";
      const std::string subStringNeutral = "NeutralParticleFlowObjects";

      std::string inputContainerBase = m_inputContainer;
      std::string outputContainerBase = m_outputContainer;

      m_inChargedFEKey = inputContainerBase;
      m_inNeutralFEKey = inputContainerBase;

      m_outChargedFEKey = outputContainerBase;
      m_outNeutralFEKey = outputContainerBase;
      
      //For both the input and output container basenames we first check if the string
      //contains "ParticleFlowObjects". If it does we swap this for "ChargedParticleFlowObjects"
      //and "NeutralParticleFlowObjects" respectively. If it doesn't we just append these two
      //substrings to the end of the strings.

      size_t pos = inputContainerBase.find(subString);
      if(pos != std::string::npos) {
        std::string inChargedString = m_inChargedFEKey.key();
        m_inChargedFEKey = inChargedString.replace(pos,subString.size(),subStringCharged);
        std::string inNeutralString = m_inNeutralFEKey.key();
        m_inNeutralFEKey = inNeutralString.replace(pos,subString.size(),subStringNeutral);
      }
      else {
        m_inChargedFEKey = inputContainerBase + subStringCharged;
        m_inNeutralFEKey = inputContainerBase + subStringNeutral;
      }

      pos = outputContainerBase.find(subString);
      if(pos != std::string::npos) {
        std::string outChargedString = m_outChargedFEKey.key();
        m_outChargedFEKey = outChargedString.replace(pos,subString.size(),subStringCharged);
        std::string outNeutralString = m_outNeutralFEKey.key();
        m_outNeutralFEKey = outNeutralString.replace(pos,subString.size(),subStringNeutral);
      }
      else{
        m_outChargedFEKey = outputContainerBase + subStringCharged;
        m_outNeutralFEKey = outputContainerBase + subStringNeutral;
      }

      //The all FE container is a bit different. If the input container base contains
      //"ParticleFlowObjects" we do nothing, otherwise we add this string onto the end.
      pos = outputContainerBase.find(subString);  
      if(pos == std::string::npos) m_outAllFEKey = outputContainerBase + subString;
      else m_outAllFEKey = outputContainerBase;

      ATH_CHECK(m_inChargedFEKey.initialize());
      ATH_CHECK(m_inNeutralFEKey.initialize());

      ATH_CHECK(m_outChargedFEKey.initialize());
      ATH_CHECK(m_outNeutralFEKey.initialize());
      ATH_CHECK(m_outAllFEKey.initialize());

      for (auto& tool : m_modifiers) {
        auto* base = dynamic_cast<JetConstituentModifierBase*>(tool.get());
        if (base) {
          base->setInputChargedContainer(m_inChargedFEKey.key());
          base->setOutputChargedContainer(m_outChargedFEKey.key());
          base->setInputNeutralContainer(m_inNeutralFEKey.key());
          base->setOutputNeutralContainer(m_outNeutralFEKey.key());
          ATH_CHECK(base->configure_decor_handle_keys());
        }
      }

      // Prepend the FE input container name to the decorations
      for (auto& key : m_inChargedFEDecorKeys) {
        const std::string keyString = m_inChargedFEKey.key() + "." + key.key();
        ATH_CHECK(key.assign(keyString));

      }

      for (auto& key : m_inNeutralFEDecorKeys) {
        const std::string keyString = m_inNeutralFEKey.key() + "." + key.key();
        ATH_CHECK(key.assign(keyString));
      }

      ATH_CHECK(m_inChargedFEDecorKeys.initialize());
      ATH_CHECK(m_inNeutralFEDecorKeys.initialize());

      ATH_CHECK(m_constituentCopyIndexKey.assign(m_outNeutralFEKey.key() + "." + m_constituentCopyIndexKey.key()));

      break;
    }
  default:
    ATH_MSG_ERROR(" Unsupported input type "<< m_inputType );
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_constituentCopyIndexKey.initialize());

  return StatusCode::SUCCESS;
}
  
int JetConstituentModSequence::execute() const {

#ifndef XAOD_ANALYSIS
  // Define monitored quantities
  auto t_exec     = Monitored::Timer<std::chrono::milliseconds>( "TIME_constitmod"  );
#endif

  // Create the shallow copy according to the input type
  switch(m_inputType){

    case xAOD::Type::CaloCluster: {
      auto sc  = copyModRecord(m_inClusterKey,
                               m_outClusterKey);
      if(!sc.isSuccess()) return 1;
      break;
    }

    case xAOD::Type::ParticleFlow: {
      auto sc = copyModRecordFlowLike<xAOD::PFOContainer, xAOD::PFO>(m_inNeutralPFOKey, m_inChargedPFOKey, m_outNeutralPFOKey, m_outChargedPFOKey, m_outAllPFOKey);
      if(!sc.isSuccess()) return 1;
      break;
    }
    case xAOD::Type::FlowElement: {
      auto sc = copyModRecordFlowLike<xAOD::FlowElementContainer, xAOD::FlowElement>(m_inNeutralFEKey, m_inChargedFEKey, m_outNeutralFEKey, m_outChargedFEKey, m_outAllFEKey);
      if(!sc.isSuccess()) return 1;
      break;
    }

    case xAOD::Type::TrackCaloCluster : {
      auto sc  = copyModRecord(m_inTCCKey, 
                                m_outTCCKey);
      if(!sc.isSuccess()){return 1;}
      break;
    }

    default: {
      ATH_MSG_WARNING( "Unsupported input type " << m_inputType );
    }

  }

  #ifndef XAOD_ANALYSIS
    auto mon = Monitored::Group(m_monTool, t_exec);
  #endif
    return 0;
}

