/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * ----------------------------------------------------------------
 * The default behavior of this tool is to use alpha = 1.0, but multiple
 * values of alpha can be used simultaneously. The property AlphaList
 * should be passed a list of floats. Values of < 0 or > 10 may result
 * in problematic output variable names and all values will be rounded
 * to the nearest 0.1. No suffix will be added to the outputs for alpha = 1.0
 * and for other values a suffix of _AlphaN will be added where N = int(10*alpha).
 *
 * The DoDichroic option adds dichroic N-subjettiness ratios described in
 * https://arxiv.org/abs/1612.03917
 * ----------------------------------------------------------------
 */

#ifndef jetsubstructuremomenttools_nsubjetinesstool_header
#define jetsubstructuremomenttools_nsubjetinesstool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"
#include "JetSubStructureMomentTools/NSubjettinessHelper.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"

class NSubjettinessTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(NSubjettinessTool, IJetModifier)

    public:
      // Constructor and destructor
      NSubjettinessTool(const std::string& name);

      StatusCode initialize() override;

      StatusCode modify(xAOD::JetContainer& jets) const override;

    private:
      Gaudi::Property<std::string> m_jetContainerName{
	this, "JetContainer", "", "SG key for the input jet container"};

      /**
       * --------------------------------------------------------------------------------
       * Structure to hold all of the necessary moment information for a single set of
       * NSubjettiness calculations. This includes the prefix and suffix, alpha, and the
       * necessary decorators.
       * --------------------------------------------------------------------------------
       **/
  
      /// N-subjettiness moments structure
      struct moments_t{
	/// Prefix for decorations
	std::string prefix;

	/// Suffix for decorations
	std::string suffix;

	/// Alpha value for calculations
	float alpha;

	moments_t (float Alpha, const std::string& Prefix)
	  : prefix (Prefix),
	    suffix (GetAlphaSuffix(Alpha)),
	    alpha (Alpha) {}
      };

      /// Configurable as properties
      float m_Alpha;
      std::vector<float> m_rawAlphaVals; /// Vector of input values before cleaning
      bool m_doDichroic;

      /// Map of decorators using alpha as the key
      std::vector<std::pair< float, moments_t >> m_moments;

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau1_Keys{
	this, "Tau1_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau2_Keys{
	this, "Tau2_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau3_Keys{
	this, "Tau3_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau4_Keys{
	this, "Tau4_Keys", {}};

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau2_ungroomed_Keys{
	this, "Tau2_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau3_ungroomed_Keys{
	this, "Tau3_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau4_ungroomed_Keys{
	this, "Tau4_ungroomed_Keys", {}};

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau1_wta_Keys{
	this, "Tau1_wta_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau2_wta_Keys{
	this, "Tau2_wta_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau3_wta_Keys{
	this, "Tau3_wta_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau4_wta_Keys{
	this, "Tau4_wta_Keys", {}};

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau2_wta_ungroomed_Keys{
	this, "Tau2_wta_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau3_wta_ungroomed_Keys{
	this, "Tau3_wta_ungroomed_Keys", {}};
      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_Tau4_wta_ungroomed_Keys{
	this, "Tau4_wta_ungroomed_Keys", {}};

};

#endif
