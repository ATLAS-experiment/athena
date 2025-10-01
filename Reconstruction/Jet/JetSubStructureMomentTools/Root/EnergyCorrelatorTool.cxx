/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/EnergyCorrelatorTool.h"
#include "JetSubStructureUtils/EnergyCorrelator.h"
#include "AthContainers/ConstAccessor.h"

EnergyCorrelatorTool::EnergyCorrelatorTool(const std::string& name) : 
  JetSubStructureMomentToolsBase(name)
{
  declareProperty("Beta",       m_Beta = 1.0);
  declareProperty("BetaList",   m_rawBetaVals = {});
  declareProperty("DoC3",       m_doC3 = false);
  declareProperty("DoC4",       m_doC4 = false);
  declareProperty("DoDichroic", m_doDichroic = false);
}

StatusCode EnergyCorrelatorTool::initialize() {
  if(m_jetContainerName.empty()){
    ATH_MSG_ERROR("EnergyCorrelatorTool needs to have its input jet container name configured!");
    return StatusCode::FAILURE;
  }

  /// Call base class initialize to fix up m_prefix
  ATH_CHECK( JetSubStructureMomentToolsBase::initialize() );

  /// Add beta = 1.0 by default
  m_moments.emplace_back( 1.0, moments_t(1.0, m_prefix) );

  /// Add beta = m_Beta by default to keep backwards compatibility
  if( std::abs(m_Beta-1.0) > 1.0e-5 ) {

    /// Give warning about deprecation
    ATH_MSG_WARNING( "The Beta property is deprecated, please use the BetaList property to provide a list of values");

    /// Use m_Beta to not break analysis code
    m_moments.emplace_back( m_Beta, moments_t(m_Beta, m_prefix) );

  }

  /// Clean up input list of beta values
  for( float beta : m_rawBetaVals ) {

    /// Round to the nearest 0.1
    float betaFix = round( beta * 10.0 ) / 10.0;
    if( std::abs(beta-betaFix) > 1.0e-5 ) ATH_MSG_DEBUG( "beta = " << beta << " has been rounded to " << betaFix );

    /// Skip negative values of beta
    if( betaFix < 0.0 ) {
      ATH_MSG_WARNING( "beta must be positive. Skipping beta = " << beta );
      continue;
    }

    /// Store value. std::map::emplace prevents duplicate entries
    m_moments.emplace_back( betaFix, moments_t(betaFix, m_prefix) );

  }

  /// Print out list of beta values to debug stream
  for( auto const& moment : m_moments ) {
    ATH_MSG_DEBUG( "Including beta = " << moment.first );
  }

  /// If DoC4 is set to true, set DoC3 to true by default since it won't
  /// add any additional computational overhead
  if( m_doC4 ) m_doC3 = true;

  // Initialise WriteDecorHandleKeys
  for( const auto& [beta, moment] : m_moments ) {
    m_ECF1_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "ECF1" + moment.suffix);
    m_ECF2_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "ECF2" + moment.suffix);
    m_ECF3_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "ECF3" + moment.suffix);
    m_ECF4_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "ECF4" + moment.suffix);
    m_ECF5_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
			     "ECF5" + moment.suffix);
    m_ECF1_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "ECF1_ungroomed" + moment.suffix);
    m_ECF2_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "ECF2_ungroomed" + moment.suffix);
    m_ECF3_ungroomed_Keys.emplace_back(m_jetContainerName + "." + moment.prefix +
				       "ECF3_ungroomed" + moment.suffix);
  }

  ATH_CHECK(m_ECF1_Keys.initialize());
  ATH_CHECK(m_ECF2_Keys.initialize());
  ATH_CHECK(m_ECF3_Keys.initialize());
  ATH_CHECK(m_ECF4_Keys.initialize());
  ATH_CHECK(m_ECF5_Keys.initialize());
  ATH_CHECK(m_ECF1_ungroomed_Keys.initialize());
  ATH_CHECK(m_ECF2_ungroomed_Keys.initialize());
  ATH_CHECK(m_ECF3_ungroomed_Keys.initialize());

  return StatusCode::SUCCESS;

}

StatusCode EnergyCorrelatorTool::modify(xAOD::JetContainer& jets) const {

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF1;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF2;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF3;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF4;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF5;

  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF1_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF2_ungroomed;
  std::vector<SG::WriteDecorHandle<xAOD::JetContainer, float>> wdhs_ECF3_ungroomed;

  for(unsigned int i=0; i<m_moments.size(); i++) {
    wdhs_ECF1.emplace_back(m_ECF1_Keys[i]);
    wdhs_ECF2.emplace_back(m_ECF2_Keys[i]);
    wdhs_ECF3.emplace_back(m_ECF3_Keys[i]);
    wdhs_ECF4.emplace_back(m_ECF4_Keys[i]);
    wdhs_ECF5.emplace_back(m_ECF5_Keys[i]);

    wdhs_ECF1_ungroomed.emplace_back(m_ECF1_ungroomed_Keys[i]);
    wdhs_ECF2_ungroomed.emplace_back(m_ECF2_ungroomed_Keys[i]);
    wdhs_ECF3_ungroomed.emplace_back(m_ECF3_ungroomed_Keys[i]);
  }

  for(const xAOD::Jet* injet : jets){

    fastjet::PseudoJet jet;
    fastjet::PseudoJet jet_ungroomed;

    /// Bool to decide whether calculation should be performed
    bool calculate = SetupDecoration(jet,*injet);

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

    for(unsigned int i=0; i<m_moments.size(); i++) {
      float beta = m_moments[i].first;

      float ECF1_value = -999;
      float ECF2_value = -999;
      float ECF3_value = -999;
      float ECF4_value = -999;
      float ECF5_value = -999;

      float ECF1_ungroomed_value = -999;
      float ECF2_ungroomed_value = -999;
      float ECF3_ungroomed_value = -999;

      if( calculate ) {

	JetSubStructureUtils::EnergyCorrelator ECF1(1, beta, JetSubStructureUtils::EnergyCorrelator::pt_R);
	JetSubStructureUtils::EnergyCorrelator ECF2(2, beta, JetSubStructureUtils::EnergyCorrelator::pt_R);
	JetSubStructureUtils::EnergyCorrelator ECF3(3, beta, JetSubStructureUtils::EnergyCorrelator::pt_R);

	ECF1_value = ECF1.result(jet);
	ECF2_value = ECF2.result(jet);
	ECF3_value = ECF3.result(jet);

	if( m_doC3 ) {
	  JetSubStructureUtils::EnergyCorrelator ECF4(4, beta, JetSubStructureUtils::EnergyCorrelator::pt_R);
	  ECF4_value = ECF4.result(jet);
	}

	if( m_doC4 ) {
	  JetSubStructureUtils::EnergyCorrelator ECF5(5, beta, JetSubStructureUtils::EnergyCorrelator::pt_R);
	  ECF5_value = ECF5.result(jet);
	}

	if( calculate_ungroomed ) {
	  ECF1_ungroomed_value = ECF1.result(jet_ungroomed);
	  ECF2_ungroomed_value = ECF2.result(jet_ungroomed);
	  ECF3_ungroomed_value = ECF3.result(jet_ungroomed);
	}

      }

      wdhs_ECF1[i](*injet) = ECF1_value;
      wdhs_ECF2[i](*injet) = ECF2_value;
      wdhs_ECF3[i](*injet) = ECF3_value;
      wdhs_ECF4[i](*injet) = ECF4_value;
      wdhs_ECF5[i](*injet) = ECF5_value;

      wdhs_ECF1_ungroomed[i](*injet) = ECF1_ungroomed_value;
      wdhs_ECF2_ungroomed[i](*injet) = ECF2_ungroomed_value;
      wdhs_ECF3_ungroomed[i](*injet) = ECF3_ungroomed_value;
    }

  }

  return StatusCode::SUCCESS;
}
