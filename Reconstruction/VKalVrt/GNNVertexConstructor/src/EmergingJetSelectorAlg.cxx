/*
    Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Frederic Renner

#include "GNNVertexConstructor/EmergingJetSelectorAlg.h"
#include "AthContainers/AuxElement.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>

namespace Rec
{
  EmergingJetSelectorAlg ::EmergingJetSelectorAlg(const std::string &name,
						  ISvcLocator *pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode EmergingJetSelectorAlg ::initialize()
  {
    ATH_CHECK(m_containerInKey.initialize());
    ATH_CHECK(m_EventInfoKey.initialize());
    ATH_CHECK(m_containerOutKey.initialize());

    // make decorators for the four vectors
    std::vector<std::string> vars{
      "pt",
	"eta",
	"phi",
	"m",
	};
    for (std::string var : vars)
      {
	std::string deco_var = m_containerOutKey.key() + "_" + var;
	SG::AuxElement::Decorator<std::vector<float>> deco(deco_var);
	m_fourVecDecos.emplace(deco_var, deco);
      };
    return StatusCode::SUCCESS;
  }

  StatusCode EmergingJetSelectorAlg ::execute()
  {
    // container we read in
    SG::ReadHandle<xAOD::JetContainer> inContainer(m_containerInKey);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoKey);
    ATH_CHECK(inContainer.isValid());
    ATH_CHECK(eventInfo.isValid());

    // make some accessors and decorators
    SG::AuxElement::ConstAccessor<float> PTF("pT_frac_prompt");
    SG::AuxElement::Decorator<unsigned int> nSelectedParticles_dec(
								   m_containerOutKey.key() + "_n");

    // fill workContainer with "views" of the inContainer
    auto workContainer = std::make_unique<ConstDataVector<xAOD::JetContainer>>(
									       SG::VIEW_ELEMENTS);

    // loop over jets
    for (const xAOD::Jet *jet : *inContainer)
      {
	// cuts
	if (jet->pt() < m_minPt || std::abs(jet->eta()) > m_maxEta)
	  continue;

	workContainer->push_back(jet);
      }

    int nJets = workContainer->size();
    // decorate # of selected particles to the eventinfo
    nSelectedParticles_dec(*eventInfo) = nJets;

    // if we have less than the requested nr, empty the workcontainer to write
    // defaults/return empty container
    // if (nJets < m_minimumAmount)
    // {
    //   workContainer->clear();
    //   nJets = 0;
    // }

    // sort and truncate
    int nKeep;
    if (nJets < m_truncateAtAmount)
      {
	nKeep = nJets;
      }
    else
      {
	nKeep = m_truncateAtAmount;
      }

    if (m_pTsort)
      {
	// if we give -1, sort the whole container
	if (m_truncateAtAmount == -1)
	  {
	    nKeep = nJets;
	  }
	std::partial_sort(
			  workContainer->begin(), // Iterator from which to start sorting
			  workContainer->begin() + nKeep, // Use begin + N to sort first N
			  workContainer->end(), // Iterator marking the end of range to sort
			  [](const xAOD::IParticle *left, const xAOD::IParticle *right)
			  {
			    return left->pt() > right->pt();
			  }); // lambda function here just handy, could also be another
	// function that returns bool

	// keep only the requested amount
	workContainer->erase(workContainer->begin() + nKeep,
			     workContainer->end());
      }

    // decorate eventInfo
    std::vector<float> jet_pt;
    std::vector<float> jet_eta;
    std::vector<float> jet_phi;
    std::vector<float> jet_m;

    // set defaults
    if (workContainer->size() == 0)
      {
	jet_pt.push_back(-100);
	jet_eta.push_back(-100);
	jet_phi.push_back(-100);
	jet_m.push_back(-100);
      }
    else
      {
	for (const xAOD::Jet *jet : *workContainer)
	  {
	    jet_pt.push_back(jet->pt());
	    jet_eta.push_back(jet->eta());
	    jet_phi.push_back(jet->phi());
	    jet_m.push_back(jet->m());
	  }
      }
    // clang-format off
    m_fourVecDecos.at(m_containerOutKey.key() + "_pt")(*eventInfo) = jet_pt;
    m_fourVecDecos.at(m_containerOutKey.key() + "_eta")(*eventInfo) = jet_eta;
    m_fourVecDecos.at(m_containerOutKey.key() + "_phi")(*eventInfo) = jet_phi;
    m_fourVecDecos.at(m_containerOutKey.key() + "_m")(*eventInfo) = jet_m;
    // clang-format on

    // write to eventstore
    SG::WriteHandle<ConstDataVector<xAOD::JetContainer>> Writer(
								m_containerOutKey);
    ATH_CHECK(Writer.record(std::move(workContainer)));

    return StatusCode::SUCCESS;
  }
}
