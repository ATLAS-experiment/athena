/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFlowUtils/WeightPFOTool.h"
#include "xAODPFlow/FEHelpers.h"

static constexpr float GeV = 1000.f;

namespace CP {

  WeightPFOTool::WeightPFOTool(const std::string& name) : asg::AsgTool( name )
  {}

  // Further details of the motivation for this procedure and explanations
  // of how it works can be found in section 4 of:
  // Eur. Phys. J. C 81 (2021) 689: https://arxiv.org/abs/2007.02645 
  //
  //The intended result is briefly described below. 
  //CP::EM Case
  // Subtraction applied (inDenseEnvironment==false)
  // pt < 30 GeV: Ptrk [weight = 1]
  // 30 <= pt < 60 GeV: Ptrk ( E/P + (1-E/P)(1 - (pt - 30)/30 ) ) [weight = E/P + (1-E/P)(1 - (pt - 30)/30 )]
  // pt >= 60 GeV: Ptrk * E/P [weight = E/P]
  // 
  // Subtraction not applied (inDenseEnvironment==true)
  // pt < 30 GeV: Ptrk (1 - E/P) + Ecal [weight = (1-E/P]
  // 30 <= pt < 60 GeV: Ptrk ( (1-E/P)(1 - (pt - 30)/30 ) ) + Ecal [weight = (1-E/P)(1 - (pt - 30)/30 )]
  // pt >= 60 GeV: Ecal [weight = 0]
  //
  //CP::LC Case - this follows when you assuem E/p = 1 for the LC scale in the above formulae
  //Subtraction applied (inDenseEnvironment==false)
  // pt < 30 GeV: Ptrk weight = 1
  // 30 <= pt < 60 GeV: weight = 0
  // pt >= 60 GeV: weight = 1;
  //
  // Subtraction not applied (inDenseEnvironment==true)
  // All Pt ranges: weight = 0
  
  StatusCode WeightPFOTool::fillWeight( const xAOD::PFO& cpfo, float& weight ) const {

    //we need to convert the string scale back to the enum
    PFO_JetMETConfig_inputScale theNeutralPFOScale = CP::EM;
    CP::inputScaleMapper inputScaleMapper;
    bool answer = inputScaleMapper.getValue(m_theNeutralPFOScaleString,theNeutralPFOScale);
    if (false == answer) ATH_MSG_FATAL("Invalid neutral PFO Scale has been specified in PFlowUtils::PFOWeightTool");

    // Compute the weights internally
    weight = 0.;
    if(cpfo.pt()>100 * GeV) {
      ATH_MSG_WARNING("PFO with invalid pt " << cpfo.pt() << ", quitting.");
      return StatusCode::FAILURE;
    }
        
    int isInDenseEnvironment = false;
    float expectedEnergy = 0.0;
    bool gotVariable = cpfo.attribute(xAOD::PFODetails::PFOAttributes::eflowRec_isInDenseEnvironment,isInDenseEnvironment);
    gotVariable &= cpfo.attribute(xAOD::PFODetails::PFOAttributes::eflowRec_tracksExpectedEnergyDeposit,expectedEnergy);
    if (!gotVariable) {
      ATH_MSG_WARNING("This charged PFO did not have eflowRec_isInDenseEnvironment or eflowRec_tracksExpectedEnergyDeposit set");
      return StatusCode::FAILURE;
    } else {
      //EM case first
      if (CP::EM == theNeutralPFOScale){
        // Start by computing the correction as though we subtracted the calo energy
        // This interpolates between the full track P and the expected calo E
        float EoverP = expectedEnergy/cpfo.e(); // divide once only
        if(m_doEoverPweight) {
          if(cpfo.pt()<30*GeV) {        // take full track
            weight = 1.;
          } else if(cpfo.pt()<60*GeV) { // linearly interpolate between 1 and E/P
            float interpolf = (1.0 - (cpfo.pt()-(30*GeV))/(30*GeV));
            weight = EoverP + interpolf * (1-EoverP);
          } else {                    // take the expected energy
            weight = EoverP;
          }
        }

        ATH_MSG_VERBOSE("cpfo in dense environment? " << isInDenseEnvironment);
        ATH_MSG_VERBOSE("cpfo pt: " << cpfo.pt() << ", E/P: " << EoverP << ", weight: " << weight);

        if(isInDenseEnvironment) {
          // In this case we further remove the expected deposited energy from the track
          weight -= EoverP;
        }
      }//EM Scale
      else if (CP::LC == theNeutralPFOScale){
        if(!isInDenseEnvironment){
          if(cpfo.pt()<30*GeV || cpfo.pt() >= 60*GeV) weight = 1;
        }
      }
    }
        
    ATH_MSG_VERBOSE("Weight before zero check: " << weight);
    // If the weight went to 0, set it to the ghost scale, so that the cPFOs
    // are always added to the track.
    if (weight<1e-9) {weight = 1e-20;}
    ATH_MSG_VERBOSE("Final weight: " << weight);
    return StatusCode::SUCCESS;
  }

  StatusCode WeightPFOTool::fillWeight( const xAOD::FlowElement& cpfo, float& weight ) const {

    if(!(cpfo.signalType() & xAOD::FlowElement::PFlow)){
      ATH_MSG_WARNING("FlowElement was not a PFO. Signal type was: " << cpfo.signalType());
      return StatusCode::FAILURE;
    }

    //we need to convert the string scale back to the enum
    PFO_JetMETConfig_inputScale theNeutralPFOScale = CP::EM;
    CP::inputScaleMapper inputScaleMapper;
    bool answer = inputScaleMapper.getValue(m_theNeutralPFOScaleString,theNeutralPFOScale);
    if (false == answer) ATH_MSG_FATAL("Invalid neutral PFO Scale has been specified in PFlowUtils::PFOWeightTool");

    // Compute the weights internally
    weight = 0.;
    if(cpfo.pt()>100*GeV) {
      ATH_MSG_WARNING("PFO with invalid pt " << cpfo.pt() << ", quitting.");
      return StatusCode::FAILURE;
    }

    const static SG::ConstAccessor<int> accDenseEnv("IsInDenseEnvironment");
    int isInDenseEnvironment = accDenseEnv(cpfo);

    //EM case first
    if (CP::EM == theNeutralPFOScale){
      // Start by computing the correction as though we subtracted the calo energy
      // This interpolates between the full track P and the expected calo E
      if (m_doEoverPweight) fillInterpolationWeight(cpfo,weight);

      ATH_MSG_VERBOSE("cpfo in dense environment? " << isInDenseEnvironment);

      // In this case we further remove the expected deposited energy from the track
      if(isInDenseEnvironment) fillDoubleCountingWeight(cpfo,weight);

    }//EM Scale
    else if (CP::LC == theNeutralPFOScale){
      if(!isInDenseEnvironment){
        if(cpfo.pt()<30*GeV || cpfo.pt() >= 60*GeV) weight = 1;
      }
    }
        
    ATH_MSG_VERBOSE("Weight before zero check: " << weight);
    // If the weight went to 0, set it to the ghost scale, so that the cPFOs
    // are always added to the track.
    if (weight<1e-9) {weight = 1e-20;}
    ATH_MSG_VERBOSE("Final weight: " << weight);
    return StatusCode::SUCCESS;
  }

  void WeightPFOTool::fillInterpolationWeight(const xAOD::FlowElement& cpfo, float& weight) const{    

    const static SG::ConstAccessor<float> accExpE("TracksExpectedEnergyDeposit");
    float expectedEnergy = accExpE(cpfo);

    float EoverP = expectedEnergy/cpfo.e(); 
    if(cpfo.pt()<30*GeV) {        // take full track
      weight = 1.;
    } else if(cpfo.pt()<60*GeV) { // linearly interpolate between 1 and E/P
      float interpolf = (1.0 - (cpfo.pt()-(30*GeV))/(30*GeV));
      weight = EoverP + interpolf * (1-EoverP);
    } else {                    // take the expected energy
      weight = EoverP;
    }    

    ATH_MSG_VERBOSE("cpfo pt: " << cpfo.pt() << ", E/P: " << EoverP << ", weight: " << weight);

  }

  void WeightPFOTool::fillDoubleCountingWeight(const xAOD::FlowElement& cpfo, float& weight) const{

    const static SG::ConstAccessor<float> accExpE("TracksExpectedEnergyDeposit");
    float expectedEnergy = accExpE(cpfo);

    float EoverP = expectedEnergy/cpfo.e(); 
    weight -= EoverP;

  }

}//namespace CP
