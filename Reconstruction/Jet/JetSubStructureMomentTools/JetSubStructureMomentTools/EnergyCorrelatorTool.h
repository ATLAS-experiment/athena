/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * ----------------------------------------------------------------
 * The default behavior of this tool is to use beta = 1.0, but multiple
 * values of beta can be used simultaneously. The property BetaList
 * should be passed a list of floats. Values of < 0 or > 10 may result
 * in problematic output variable names and all values will be rounded
 * to the nearest 0.1. No suffix will be added to the outputs for beta = 1.0
 * and for other values a suffix of _BetaN will be added where N = int(10*beta).
 *
 * The DoDichroic option adds dichroic energy correlator ratios described
 * on page 120 in https://arxiv.org/abs/1803.07977
 * ----------------------------------------------------------------
 */

#ifndef jetsubstructuremomenttools_energycorrelatortool_header
#define jetsubstructuremomenttools_energycorrelatortool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"
#include "JetSubStructureMomentTools/ECFHelper.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"
#include "AsgDataHandles/WriteDecorHandle.h"

class EnergyCorrelatorTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(EnergyCorrelatorTool, IJetModifier)

    public:
      
      /// Constructor
      EnergyCorrelatorTool(const std::string& name);
     
      virtual StatusCode initialize() override;

      StatusCode modify(xAOD::JetContainer& jets) const override;

    private:
      Gaudi::Property<std::string> m_jetContainerName{
	this, "JetContainer", "", "SG key for the input jet container"};

      /**
       * --------------------------------------------------------------------------------
       * Structure to hold all of the necessary moment information for a single set of
       * EnergyCorrelator calculations. This includes the prefix and suffix, and beta.
       * --------------------------------------------------------------------------------
       **/

      /// ECF moments structure
      struct moments_t {

	/// Prefix for decorations
	std::string prefix;

	/// Suffix for decorations
	std::string suffix;

	/// Beta value for calculations
	float beta;

	moments_t (float Beta, const std::string& Prefix)
	  : prefix (Prefix),
	    suffix (GetBetaSuffix(Beta)),
	    beta (Beta) {}
      };

      /// Configurable as properties
      float m_Beta;
      bool m_doC3;
      bool m_doC4;
      std::vector<float> m_rawBetaVals; /// Vector of input values before cleaning
      bool m_doDichroic;
      
      /// Map of moment calculators and decorators using beta as the key
      std::vector<std::pair< float, moments_t >> m_moments;

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF1_Keys{
	this, "ECF1_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF2_Keys{
	this, "ECF2_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF3_Keys{
	this, "ECF3_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF4_Keys{
	this, "ECF4_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF5_Keys{
	this, "ECF5_Keys", {}};

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF1_ungroomed_Keys{
	this, "ECF1_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF2_ungroomed_Keys{
	this, "ECF2_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_ECF3_ungroomed_Keys{
	this, "ECF3_ungroomed_Keys", {}};

};

#endif
