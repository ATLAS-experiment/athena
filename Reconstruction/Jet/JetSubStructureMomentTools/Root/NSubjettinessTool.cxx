/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/NSubjettinessTool.h"
#include "JetSubStructureUtils/Nsubjettiness.h"
#include "AthContainers/ConstAccessor.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "CxxUtils/ubsan_suppress.h"

#include "fastjet/contrib/Nsubjettiness.hh"
#include "fastjet/contrib/AxesDefinition.hh"

NSubjettinessTool::NSubjettinessTool(const std::string& name) :
  JetSubStructureMomentToolsBase(name)
{
  declareProperty("Alpha",      m_Alpha = 1.0);
  declareProperty("AlphaList",  m_rawAlphaVals = {});
  declareProperty("DoDichroic", m_doDichroic = false);
}

StatusCode NSubjettinessTool::initialize() {
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("NSubjettinessTool needs to have its input jet container name configured!");
    return StatusCode::FAILURE;
  }
  
  /// Call base class initialize to fix up m_prefix
  ATH_CHECK( JetSubStructureMomentToolsBase::initialize() );

  /// Add alpha = 1.0 by default
  m_moments.emplace_back( 1.0, moments_t(1.0, m_prefix) );

  /// Add alpha = m_Alpha by default to keep backwards compatibility
  if( std::abs(m_Alpha-1.0) > 1.0e-5 ) {
    
    /// Give warning about deprecation
    ATH_MSG_WARNING( "The Alpha property is deprecated, please use the AlphaList property to provide a list of values" );

    /// Use m_Alpha to not break analysis code
    m_moments.emplace_back( m_Alpha, moments_t(m_Alpha, m_prefix) );
  
  }

  /// Clean up input list of alpha values
  for( float alpha : m_rawAlphaVals ) {
    
    /// Round to the nearest 0.1
    float alphaFix = round( alpha * 10.0 ) / 10.0;
    if( std::abs(alpha-alphaFix) > 1.0e-5 ) ATH_MSG_DEBUG( "alpha = " << alpha << " has been rounded to " << alphaFix );

    /// Skip negative values of alpha
    if( alphaFix < 0.0 ) {
      ATH_MSG_WARNING( "alpha must be positive. Skipping alpha = " << alpha );
      continue;
    }

    /// Store value. std::map::emplace prevents duplicate entries
    m_moments.emplace_back( alphaFix, moments_t(alphaFix, m_prefix) );

  }

  for( auto const& moment : m_moments ) {
    ATH_MSG_DEBUG( "Including alpha = " << moment.first );
  }

  // Initialise WriteDecorHandleKeys
  for( const auto& [alpha, moment] : m_moments ) {
    m_Tau1_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "Tau1" + moment.suffix);
    m_Tau2_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "Tau2" + moment.suffix);
    m_Tau3_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "Tau3" + moment.suffix);
    m_Tau4_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "Tau4" + moment.suffix);

    m_Tau2_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "Tau2_ungroomed" + moment.suffix);
    m_Tau3_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "Tau3_ungroomed" + moment.suffix);
    m_Tau4_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "Tau4_ungroomed" + moment.suffix);

    m_Tau1_wta_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				 "Tau1_wta" + moment.suffix);
    m_Tau2_wta_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				 "Tau2_wta" + moment.suffix);
    m_Tau3_wta_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				 "Tau3_wta" + moment.suffix);
    m_Tau4_wta_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				 "Tau4_wta" + moment.suffix);

    m_Tau2_wta_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
					   "Tau2_wta_ungroomed" + moment.suffix);
    m_Tau3_wta_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
					   "Tau3_wta_ungroomed" + moment.suffix);
    m_Tau4_wta_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
					   "Tau4_wta_ungroomed" + moment.suffix);
  }

  ATH_CHECK(m_Tau1_Keys.initialize());
  ATH_CHECK(m_Tau2_Keys.initialize());
  ATH_CHECK(m_Tau3_Keys.initialize());
  ATH_CHECK(m_Tau4_Keys.initialize());

  ATH_CHECK(m_Tau2_ungroomed_Keys.initialize());
  ATH_CHECK(m_Tau3_ungroomed_Keys.initialize());
  ATH_CHECK(m_Tau4_ungroomed_Keys.initialize());

  ATH_CHECK(m_Tau1_wta_Keys.initialize());
  ATH_CHECK(m_Tau2_wta_Keys.initialize());
  ATH_CHECK(m_Tau3_wta_Keys.initialize());
  ATH_CHECK(m_Tau4_wta_Keys.initialize());

  ATH_CHECK(m_Tau2_wta_ungroomed_Keys.initialize());
  ATH_CHECK(m_Tau3_wta_ungroomed_Keys.initialize());
  ATH_CHECK(m_Tau4_wta_ungroomed_Keys.initialize());
  
  return StatusCode::SUCCESS;
}

StatusCode NSubjettinessTool::modify(xAOD::JetContainer& jets) const {

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau1;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau2;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau3;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau4;

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau2_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau3_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau4_ungroomed;

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau1_wta;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau2_wta;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau3_wta;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau4_wta;

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau2_wta_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau3_wta_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_Tau4_wta_ungroomed;

  for(unsigned int i=0; i<m_moments.size(); i++) {
    wdhs_Tau1.emplace_back(m_Tau1_Keys[i]);
    wdhs_Tau2.emplace_back(m_Tau2_Keys[i]);
    wdhs_Tau3.emplace_back(m_Tau3_Keys[i]);
    wdhs_Tau4.emplace_back(m_Tau4_Keys[i]);

    wdhs_Tau2_ungroomed.emplace_back(m_Tau2_ungroomed_Keys[i]);
    wdhs_Tau3_ungroomed.emplace_back(m_Tau3_ungroomed_Keys[i]);
    wdhs_Tau4_ungroomed.emplace_back(m_Tau4_ungroomed_Keys[i]);

    wdhs_Tau1_wta.emplace_back(m_Tau1_wta_Keys[i]);
    wdhs_Tau2_wta.emplace_back(m_Tau2_wta_Keys[i]);
    wdhs_Tau3_wta.emplace_back(m_Tau3_wta_Keys[i]);
    wdhs_Tau4_wta.emplace_back(m_Tau4_wta_Keys[i]);

    wdhs_Tau2_wta_ungroomed.emplace_back(m_Tau2_wta_ungroomed_Keys[i]);
    wdhs_Tau3_wta_ungroomed.emplace_back(m_Tau3_wta_ungroomed_Keys[i]);
    wdhs_Tau4_wta_ungroomed.emplace_back(m_Tau4_wta_ungroomed_Keys[i]);
  }

  for(const xAOD::Jet* injet : jets){
    fastjet::PseudoJet jet;
    fastjet::PseudoJet jet_ungroomed;

    /// Bool to decide whether calculation should be performed
    bool calculate = SetupDecoration(jet, *injet);

    /// Bool to decide if ungroomed jet moments should be calculated
    bool calculate_ungroomed = false;
  
    if( m_doDichroic ) {

      /// Get parent jet
      static const SG::ConstAccessor<ElementLink<xAOD::JetContainer> > ParentAcc ("Parent");
      ElementLink<xAOD::JetContainer> parentLink = ParentAcc (*injet);

      /// Return error is parent element link is broken
      if( !parentLink.isValid() ) {
	ATH_MSG_ERROR( "Parent element link is not valid. Aborting" );
	return StatusCode::FAILURE;
      }

      const xAOD::Jet* parentJet = *(parentLink);
      calculate_ungroomed = SetupDecoration(jet_ungroomed,*parentJet);
    }

    // Supress a warning about undefined behavior in the fastjet
    // WTA_KT_Axes ctor:
    // .../fastjet/contrib/AxesDefinition.hh:551:43: runtime error: member access within address 0x7ffd770850d0 which does not point to an object of type 'WTA_KT_Axes'
    // 0x7ffd770850d0: note: object has invalid vptr
    std::once_flag oflag;
    std::call_once (oflag, CxxUtils::ubsan_suppress,
		    []() { fastjet::contrib::WTA_KT_Axes x; });

    for(unsigned int i=0; i<m_moments.size(); i++) {

      float alpha = m_moments[i].first;

      /// This needs to be redone for each jet since it depends on the size parameter
      fastjet::contrib::NormalizedCutoffMeasure normalized_measure
	(alpha, injet->getSizeParameter(), 1000000);

      float Tau1_value = -999;
      float Tau2_value = -999;
      float Tau3_value = -999;
      float Tau4_value = -999;

      float Tau2_ungroomed_value = -999;
      float Tau3_ungroomed_value = -999;
      float Tau4_ungroomed_value = -999;

      float Tau1_wta_value = -999;
      float Tau2_wta_value = -999;
      float Tau3_wta_value = -999;
      float Tau4_wta_value = -999;

      float Tau2_wta_ungroomed_value = -999;
      float Tau3_wta_ungroomed_value = -999;
      float Tau4_wta_ungroomed_value = -999;

      if( calculate ) {

	/// These calculators need to be created for each jet due to
	/// the jet size parameter dependence in the normalized measure
	fastjet::contrib::KT_Axes kt_axes;
	JetSubStructureUtils::Nsubjettiness tau1(1, kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau2(2, kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau3(3, kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau4(4, kt_axes, normalized_measure);

	Tau1_value = tau1.result(jet);
	Tau2_value = tau2.result(jet);
	Tau3_value = tau3.result(jet);
	Tau4_value = tau4.result(jet);

	if( calculate_ungroomed ) {
	  Tau2_ungroomed_value = tau2.result(jet_ungroomed);
	  Tau3_ungroomed_value = tau3.result(jet_ungroomed);
	  Tau4_ungroomed_value = tau4.result(jet_ungroomed);
	}

	/// These calculators need to be created for each jet due to
	/// the jet size parameter dependence in the normalized measure
	fastjet::contrib::WTA_KT_Axes wta_kt_axes;
	JetSubStructureUtils::Nsubjettiness tau1_wta(1, wta_kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau2_wta(2, wta_kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau3_wta(3, wta_kt_axes, normalized_measure);
	JetSubStructureUtils::Nsubjettiness tau4_wta(4, wta_kt_axes, normalized_measure);

	Tau1_wta_value = tau1_wta.result(jet);
	Tau2_wta_value = tau2_wta.result(jet);
	Tau3_wta_value = tau3_wta.result(jet);
	Tau4_wta_value = tau4_wta.result(jet);

	if( calculate_ungroomed ) {
	  Tau2_wta_ungroomed_value = tau2_wta.result(jet_ungroomed);
	  Tau3_wta_ungroomed_value = tau3_wta.result(jet_ungroomed);
	  Tau4_wta_ungroomed_value = tau4_wta.result(jet_ungroomed);
	}

      }

      wdhs_Tau1[i](*injet) = Tau1_value;
      wdhs_Tau2[i](*injet) = Tau2_value;
      wdhs_Tau3[i](*injet) = Tau3_value;
      wdhs_Tau4[i](*injet) = Tau4_value;

      wdhs_Tau2_ungroomed[i](*injet) = Tau2_ungroomed_value;
      wdhs_Tau3_ungroomed[i](*injet) = Tau3_ungroomed_value;
      wdhs_Tau4_ungroomed[i](*injet) = Tau4_ungroomed_value;

      wdhs_Tau1_wta[i](*injet) = Tau1_wta_value;
      wdhs_Tau2_wta[i](*injet) = Tau2_wta_value;
      wdhs_Tau3_wta[i](*injet) = Tau3_wta_value;
      wdhs_Tau4_wta[i](*injet) = Tau4_wta_value;

      wdhs_Tau2_wta_ungroomed[i](*injet) = Tau2_wta_ungroomed_value;
      wdhs_Tau3_wta_ungroomed[i](*injet) = Tau3_wta_ungroomed_value;
      wdhs_Tau4_wta_ungroomed[i](*injet) = Tau4_wta_ungroomed_value;
    }

  }

  return StatusCode::SUCCESS;
}
