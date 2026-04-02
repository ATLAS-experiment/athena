/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "PFlowUtils/WeightPFOTool.h"
#include "xAODPFlow/FEHelpers.h"

namespace CP {

  WeightPFOTool::WeightPFOTool(const std::string& name) : asg::AsgTool( name )
  {
    declareProperty("DoEoverPWeight", m_doEoverPweight=true);
    declareProperty("NeutralPFOScale",m_theNeutralPFOScaleString="EM");
  }

  // Further details of the motivation for this procedure and explanations
  // of how it works can be found in section 4 of:
  // Eur. Phys. J. C 81 (2021) 689: https://arxiv.org/abs/2007.02645 
  //
  //The intended result is briefly described below. 
  //CP::EM Case
  // Subtraction applied (inDenseEnvironment==false)
  // pt < 30 GeV: Ptrk [correction_factor = 1]
  // 30 <= pt < 60 GeV: Ptrk ( E/P + (1-E/P)(1 - (pt - 30)/30 ) ) [correction_factor = E/P + (1-E/P)(1 - (pt - 30)/30 )]
  // pt >= 60 GeV: Ptrk * E/P [correction_factor = E/P]
  // 
  // Subtraction not applied (inDenseEnvironment==true)
  // pt < 30 GeV: Ptrk (1 - E/P) + Ecal [correction_factor = (1-E/P]
  // 30 <= pt < 60 GeV: Ptrk ( (1-E/P)(1 - (pt - 30)/30 ) ) + Ecal [correction_factor = (1-E/P)(1 - (pt - 30)/30 )]
  // pt >= 60 GeV: Ecal [correction_factor = 0]
  //
  //CP::LC Case - this follows when you assume E/p = 1 for the LC scale in the above formulae
  //Subtraction applied (inDenseEnvironment==false)
  // All Pt ranges: Ptrk [correction_factor = 1 ... no correction applied]
  //
  // Subtraction not applied (inDenseEnvironment==true)
  // All Pt ranges: Ecal [correction_factor = 0]
  
  StatusCode WeightPFOTool::fillWeight( const xAOD::PFO& cpfo, float& weight ) const {

    //we need to convert the string scale back to the enum
    PFO_JetMETConfig_inputScale theNeutralPFOScale = CP::EM;
    CP::inputScaleMapper inputScaleMapper;
    bool answer = inputScaleMapper.getValue(m_theNeutralPFOScaleString,theNeutralPFOScale);
    if (false == answer) ATH_MSG_FATAL("Invalid neutral PFO Scale has been specified in PFlowUtils::PFOWeightTool");

    // Compute the weights internally
    weight = 1.;
    if(cpfo.pt()>100e3) {
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
          if(cpfo.pt()<30e3) {        // take full track
            weight = 1.;
          } else if(cpfo.pt()<60e3) { // linearly interpolate between 1 and E/P
            float interpolf = (1.0 - (cpfo.pt()-30000)/30000);
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
          weight = 1.0;
        } else {
          weight = 0.0;
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

  StatusCode WeightPFOTool::fillWeight( const xAOD::FlowElement& cpfo, float& p4_trk_correction_factor ) const {

    if(!(cpfo.signalType() & xAOD::FlowElement::PFlow)){
      ATH_MSG_ERROR("FlowElement was not a PFO. Signal type was: " << cpfo.signalType());
      return StatusCode::FAILURE;
    }

    //we need to convert the string scale back to the enum
    PFO_JetMETConfig_inputScale theNeutralPFOScale = CP::EM;
    CP::inputScaleMapper inputScaleMapper;
    if (!inputScaleMapper.getValue(m_theNeutralPFOScaleString, theNeutralPFOScale)) {
      ATH_MSG_ERROR("Invalid neutral PFO Scale has been specified in PFlowUtils::PFOWeightTool");
      return StatusCode::FAILURE;
    }

    if(cpfo.pt()>100e3) {
      ATH_MSG_ERROR("PFO with invalid pt " << cpfo.pt() << ", quitting.");
      return StatusCode::FAILURE;
    }

    p4_trk_correction_factor = 1.;
    //EM case first
    if (CP::EM == theNeutralPFOScale) {
      p4_trk_correction_factor = calculateEMScaleCorrection(cpfo);      
    }//EM Scale
    else if (CP::LC == theNeutralPFOScale){
      p4_trk_correction_factor = calculateLCScaleCorrection(cpfo);
    } else {
      ATH_MSG_ERROR("Invalid neutral PFO Scale has been specified in WeightPFOTool");
      return StatusCode::FAILURE;
    }
        
    ATH_MSG_VERBOSE("Weight before zero check: " << p4_trk_correction_factor);
    // If the p4_trk_correction_factor went to 0, set it to the ghost scale, so that the cPFOs
    // are always added to the track.
    if (p4_trk_correction_factor<1e-9) {
      p4_trk_correction_factor = 1e-20;
    }
    ATH_MSG_VERBOSE("Final p4_trk_correction_factor: " << p4_trk_correction_factor);
    return StatusCode::SUCCESS;
  }

  float WeightPFOTool::calculateEMScaleCorrection(const xAOD::FlowElement& cpfo) const {
    float p4_trk_correction_factor = 1.0;

    const static SG::AuxElement::ConstAccessor<float> accExpE("TracksExpectedEnergyDeposit");
    if (!accExpE.isAvailable(cpfo)) {
      throw std::runtime_error("WeightPFOTool: Auxiliary element 'TracksExpectedEnergyDeposit' is not available for PFO with index " + std::to_string(cpfo.index()));
    }
    const float expectedEnergy = accExpE(cpfo);
    // Start by computing the correction as though we subtracted the calo energy
    // This interpolates between the full track P and the expected calo E
    if (m_doEoverPweight) {
      p4_trk_correction_factor = getSmoothingScaleFactorFromInterpolation(cpfo, expectedEnergy);
    }

    ATH_MSG_VERBOSE("cpfo in dense environment? " << isInDenseEnvironment(cpfo));

    // In this case we further remove the expected deposited energy from the track
    if (isInDenseEnvironment(cpfo)) {
      p4_trk_correction_factor -= getDoubleCountingCorrection(cpfo, expectedEnergy);
    }
    return p4_trk_correction_factor;
  }

  float WeightPFOTool::getSmoothingScaleFactorFromInterpolation(const xAOD::FlowElement& cpfo, const float& expectedEnergy) const{    

    const float EoverP = expectedEnergy/cpfo.e(); 
    const float pt_interp_low = 30e3;
    const float pt_interp_high = 60e3;

    auto interpolate = [](const float& x, const float& x_low, const float& x_high, const float& y_low, const float& y_high) {
      if (x < x_low)
        return y_low;
      else if (x >= x_high)
        return y_high;
      else
        return y_low + (y_high - y_low) * (x - x_low) / (x_high - x_low);
    };

    const float smoothingScaleFactor = interpolate(cpfo.pt(), pt_interp_low, pt_interp_high, 1.0, EoverP);
    ATH_MSG_VERBOSE("cpfo pt: " << cpfo.pt() << ", E/P: " << EoverP << ", Smoothing Scale Factor from Interpolation: " << smoothingScaleFactor);
    return smoothingScaleFactor;
  }

  float WeightPFOTool::getDoubleCountingCorrection(const xAOD::FlowElement& cpfo, const float& expectedEnergy) const{
    return expectedEnergy / cpfo.e(); 
  }

  float WeightPFOTool::calculateLCScaleCorrection(const xAOD::FlowElement& cpfo) const {
    if (isInDenseEnvironment(cpfo)) {
      return 0.0;
    } else {
      return 1.0;
    }
  }

  bool WeightPFOTool::isInDenseEnvironment(const xAOD::FlowElement& cpfo) const {
    const static SG::AuxElement::ConstAccessor<int> accDenseEnv("IsInDenseEnvironment");
    if (!accDenseEnv.isAvailable(cpfo)) {
      throw std::runtime_error("WeightPFOTool: Auxiliary element 'IsInDenseEnvironment' is not available for PFO with index " + std::to_string(cpfo.index()));
    }
    return accDenseEnv(cpfo);
  }

}//namespace CP