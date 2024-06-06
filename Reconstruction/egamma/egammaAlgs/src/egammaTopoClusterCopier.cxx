/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaTopoClusterCopier.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "xAODCore/ShallowCopy.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include <cmath>
#include <map>

namespace{
// Special egamma EMFraction which includes presampler and E4 cells.
const SG::AuxElement::Accessor<float> s_acc_emfraction {"EMFraction"};

//comparison function
bool greater(xAOD::CaloCluster const* a, xAOD::CaloCluster const* b) {
  const double emfrac_a = s_acc_emfraction(*a);
  const double emfrac_b = s_acc_emfraction(*b);
  return (a->et() * emfrac_a) > (b->et() * emfrac_b);
}

double addTileGapCellsEnergy(xAOD::CaloCluster* cluster) {
  double eg_tilegap = 0;

  xAOD::CaloCluster::const_cell_iterator cell_itr = cluster->cell_cbegin();
  xAOD::CaloCluster::const_cell_iterator cell_end = cluster->cell_cend();

  for (; cell_itr != cell_end; ++cell_itr) {
    const CaloCell* cell = *cell_itr;
    if (!cell) { continue; }

    const CaloDetDescrElement *dde = cell->caloDDE();
    if (!dde) { continue; }

    // Add TileGap3. Consider only E4 cell.
    if (CaloCell_ID::TileGap3 == dde->getSampling()) {
      double ddAbsEta = std::abs(dde->eta_raw());
      if (ddAbsEta > 1.4 && ddAbsEta < 1.6) {
        eg_tilegap += cell->e() * cell_itr.weight();
      }
    }
  }

  return eg_tilegap;
}

bool checkIfValidForCentral(
  double aeta,
  float etaCut,
  double clusterE,
  float ECut
) {
  return aeta <= etaCut && clusterE >= ECut;
}

bool checkIfValidForFwd(
  double aeta,
  float etaCut,
  double ETCut,
  xAOD::CaloCluster *clus,
  bool doForwardClusters,
  bool hasITk
) {
  if (doForwardClusters) {
    // LC variables are Local Hadronic Calorimeter calibrated and are included
    // for compatibility with Run3.
    const double clusterET = clus->et();
    const double clusterETLC = clus->getSisterCluster()->et();
    const double aetaLC = std::abs(clus->getSisterCluster()->eta());

    if (hasITk) {
      // When we do actually ITK these should be EM.
      return aeta >= etaCut && clusterET >= ETCut;
    } else {
      // Without ITK we need the LC.
      return aetaLC >= etaCut && clusterETLC >= ETCut;
    }
  } else {
    return false;
  }
}
}  // namespace

egammaTopoClusterCopier::egammaTopoClusterCopier(const std::string& name,
                                                 ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator)
{ }

StatusCode egammaTopoClusterCopier::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_inputTopoCollection.initialize());
  ATH_CHECK(m_outputTopoCollection.initialize());

  m_doForwardClusters = !m_outputFwdTopoCollection.empty();
  ATH_CHECK(m_outputFwdTopoCollection.initialize(m_doForwardClusters));

  m_outputTopoCollectionShallow = "tmp_"+ m_outputTopoCollection.key();
  ATH_CHECK(m_outputTopoCollectionShallow.initialize());

  ATH_MSG_DEBUG("Initialization successful");

  return StatusCode::SUCCESS;
}

StatusCode egammaTopoClusterCopier::finalize() {
  ATH_MSG_INFO("=====> Selected Topo cluster statistics ============");
  ATH_MSG_INFO(" All Clusters " << m_AllClusters );
  ATH_MSG_INFO(" Pass Preselection Clusters " << m_PassPreSelection );
  ATH_MSG_INFO(" Pass Selection " << m_PassSelection );
  ATH_MSG_INFO(" Central: Pass Preselection Clusters " << m_CentralPassPreSelection );
  ATH_MSG_INFO(" Central: Pass Selection " << m_CentralPassSelection );
  ATH_MSG_INFO(" Fwd: Pass Preselection Clusters " << m_FwdPassPreSelection );
  ATH_MSG_INFO(" Fwd: Pass Selection " << m_FwdPassSelection );
  ATH_MSG_INFO(" Shared: Pass Preselection Clusters " << m_SharedPassPreSelection );
  ATH_MSG_INFO(" Shared: Pass Selection " << m_SharedPassSelection );
  ATH_MSG_INFO(" All Clusters " << m_AllClusters );
  ATH_MSG_INFO("===================================================");

  return StatusCode::SUCCESS;
}

StatusCode egammaTopoClusterCopier::execute(const EventContext& ctx) const {
  SG::ReadHandle<xAOD::CaloClusterContainer> inputTopoclusters(m_inputTopoCollection, ctx);
  SG::WriteHandle<xAOD::CaloClusterContainer> outputTopoclustersShallow(m_outputTopoCollectionShallow, ctx);
  SG::WriteHandle<ConstDataVector<xAOD::CaloClusterContainer>> outputTopoclusters(m_outputTopoCollection, ctx);

  // Create a shallow copy, the elements of this can be modified, but no need to
  // recreate the cluster.
  std::pair<
    std::unique_ptr<xAOD::CaloClusterContainer>,
    std::unique_ptr<xAOD::ShallowAuxContainer>
  > inputShallowcopy = xAOD::shallowCopyContainer(*inputTopoclusters, ctx);

  ATH_CHECK(outputTopoclustersShallow.record(
    std::move(inputShallowcopy.first),
    std::move(inputShallowcopy.second)
  ));

  // Here it just needs to be a view copy, i.e the collection we create does not
  // really own its elements.
  auto viewCopy = std::make_unique<ConstDataVector<xAOD::CaloClusterContainer>>(SG::VIEW_ELEMENTS);

  // Declare and conditionally initialise forward cluster objects.
  SG::WriteHandle<ConstDataVector<xAOD::CaloClusterContainer>> outputFwdTopoclusters;
  std::unique_ptr<ConstDataVector<xAOD::CaloClusterContainer>> fwdViewCopy;

  if (m_doForwardClusters) {
    outputFwdTopoclusters = SG::WriteHandle<ConstDataVector<xAOD::CaloClusterContainer>>(
      m_outputFwdTopoCollection,
      ctx
    );

    fwdViewCopy = std::make_unique<ConstDataVector<xAOD::CaloClusterContainer>>(SG::VIEW_ELEMENTS);
  }

  auto buff_AllClusters = m_AllClusters.buffer();
  auto buff_PassPreSelection = m_PassPreSelection.buffer();
  auto buff_PassSelection = m_PassSelection.buffer();
  auto buff_CentralPassPreSelection = m_CentralPassPreSelection.buffer();
  auto buff_CentralPassSelection = m_CentralPassSelection.buffer();
  auto buff_FwdPassPreSelection = m_FwdPassPreSelection.buffer();
  auto buff_FwdPassSelection = m_FwdPassSelection.buffer();
  auto buff_SharedPassPreSelection = m_SharedPassPreSelection.buffer();
  auto buff_SharedPassSelection = m_SharedPassSelection.buffer();

  // Loop over the output shallow copy.
  for (xAOD::CaloCluster* clus : *outputTopoclustersShallow) {
    ATH_MSG_DEBUG(
        "->CHECKING Cluster at eta,phi,et " <<
        clus->eta() << " , " <<
        clus->phi() << " , " <<
        clus->et()
    );

    ++buff_AllClusters;
    s_acc_emfraction(*clus) = 0.0;  // Always decorate

    const double clusterE = clus->e();
    const double aeta = std::abs(clus->eta());

    const bool valid_for_central = checkIfValidForCentral(aeta, m_etaCut, clusterE, m_ECut);
    const bool valid_for_fwd = checkIfValidForFwd(aeta, m_fwdEtaCut, m_fwdETCut, clus, m_doForwardClusters, m_hasITk);
    const bool valid_for_both = valid_for_central && valid_for_fwd;
    const bool valid_for_either = valid_for_central || valid_for_fwd;

    if (valid_for_central) { ++buff_CentralPassPreSelection; }
    if (valid_for_fwd) { ++buff_FwdPassPreSelection; }
    if (valid_for_both) { ++buff_SharedPassPreSelection; }
    if (valid_for_either) { ++buff_PassPreSelection; }
    else {
      continue;
    }

    if (!m_hasITk && m_doForwardClusters && valid_for_fwd) {
      fwdViewCopy->push_back(clus->getSisterCluster());
      ++buff_FwdPassSelection;
    }

    // Add the relevant TileGap3/E4 cells.
    double eg_tilegap = 0;
    if (valid_for_central) {
      if (aeta > 1.35 && aeta < 1.65 && clusterE > 0) {
        eg_tilegap += addTileGapCellsEnergy(clus);
      }
    }

    const double emfrac= (clus->energyBE(0) + clus->energyBE(1) +
                    clus->energyBE(2) + clus->energyBE(3) + eg_tilegap) / clusterE;

    s_acc_emfraction(*clus) = emfrac;
    if ((emfrac > m_EMFracCut && (clusterE * emfrac) > m_ECut) || xAOD::EgammaHelpers::isFCAL(clus)) {
      ATH_MSG_DEBUG(
        "-->Selected Cluster at eta,phi,et,EMFraction " << clus->eta() <<
        " , " << clus->phi() <<
        " , " << clus->et() <<
        " , " << emfrac
      );

      ++buff_PassSelection;

      if (valid_for_central) {
        viewCopy->push_back(clus);
        ++buff_CentralPassSelection;
      }

      if (m_hasITk && valid_for_fwd) {
        fwdViewCopy->push_back(clus);
        ++buff_FwdPassSelection;
      }

      if (valid_for_both) {
        ++buff_SharedPassSelection;
      }
    }
  } // End loop on clusters.

  // Sort in descenting em energy.
  std::sort(viewCopy->begin(), viewCopy->end(), greater);

  ATH_MSG_DEBUG(
      "Cloned container has size: " << viewCopy->size() <<
      " selected out of : " << inputTopoclusters->size()
  );

  ATH_CHECK(outputTopoclusters.record(std::move(viewCopy)));

  if (m_doForwardClusters) {
    if (m_hasITk) {
      std::sort(fwdViewCopy->begin(), fwdViewCopy->end(), greater);
    }

    ATH_MSG_DEBUG(
      "Cloned fwd container has size: " << fwdViewCopy->size() <<
      " selected out of : " << inputTopoclusters->size()
    );

    ATH_CHECK(outputFwdTopoclusters.record(std::move(fwdViewCopy)));
  }

  return StatusCode::SUCCESS;
}

