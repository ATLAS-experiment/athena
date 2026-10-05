/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronEfficiencyCorrection/TElectronTestAlg.h"

#include "ElectronEfficiencyCorrection/ElRecomFileHelpers.h"
#include "PathResolver/PathResolver.h"
//
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
//
#include "xAODEgamma/Electron.h"
//
#include <cmath>
#include <optional>

StatusCode CP::TElectronTestAlg::initialize() {

  // input
  ATH_CHECK(m_electronContainer.initialize());

  // Decorations
  m_pimpl = std::make_unique<Root::TElectronEfficiencyCorrectionTool>(
      (this->name() + ".TElectronEfficiencyCorrection").c_str());
  //
  m_pimpl->msg().setLevel(this->msg().level());
  //
  std::string mapFileName = PathResolverFindCalibFile(m_mapFile);
  std::string key = ElRecomFileHelpers::convertToOneKey(m_recoKey, m_idKey,
                                                        m_isoKey, m_triggerKey);
  std::string value = ElRecomFileHelpers::getValueByKey(mapFileName, key);
  std::string filename = PathResolverFindCalibFile(value);
  if (filename.empty()) {
    ATH_MSG_ERROR("Could NOT resolve file name " << value);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO(" Using recommendations file = " << filename);
  }
  m_pimpl->addFileName(filename);
  //
  if (m_mode == Toys || m_mode == All) {
    m_pimpl->bookCombToyMCScaleFactors(m_number_of_toys);
  }
  // We use 0 as failure ...
  if (m_pimpl->initialize() == 0) {
    ATH_MSG_ERROR(
        "Could not initialize the TElectronEfficiencyCorrectionTool!");
    return StatusCode::FAILURE;
  }

  // Auxiliary info
  m_numberCorr = m_pimpl->getNSyst();
  std::map<float, std::vector<float>> tmp;
  m_pimpl->getNbins(tmp);
  m_lowestEt = tmp.begin()->first;
  m_doDetail = (m_mode != Toys && m_mode != Total);
  m_doToys = (m_mode == Toys || m_mode == All);

  // decorations if requested
  const std::string baseName = m_electronContainer.key();
  m_SF = baseName + ".EFF_" + key + "_SF";
  m_TotalUp = baseName + ".EFF_" + key + "_Total__Up";
  m_TotalDown = baseName + ".EFF_" + key + "_Total__Down";
  m_uncorrUp = baseName + ".EFF_" + key + "_UnCorr__Up";
  m_uncorrDown = baseName + ".EFF_" + key + "_UnCorr__Down";
  m_HistIndex = baseName + ".EFF_" + key + "_HistIndex";
  m_HistBin = baseName + ".EFF_" + key + "_HistBin";
  m_corrUp = baseName + ".EFF_" + key + "_Corr__Up";
  m_corrDown = baseName + ".EFF_" + key + "_Corr__Down";
  m_toys = baseName + ".EFF_" + key + "__toys";

  ATH_CHECK(m_SF.initialize(m_decorate));
  ATH_CHECK(m_TotalUp.initialize(m_decorate));
  ATH_CHECK(m_TotalDown.initialize(m_decorate));
  ATH_CHECK(m_uncorrUp.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_uncorrDown.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_HistIndex.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_HistBin.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_corrUp.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_corrDown.initialize(m_decorate && m_doDetail));
  ATH_CHECK(m_toys.initialize(m_decorate && m_doToys));

  return StatusCode::SUCCESS;
}

StatusCode CP::TElectronTestAlg::execute(const EventContext& ctx) const {

  //
  unsigned int runNumber = 428648;  // use a dummy default for testing

  int dataType = m_dataType;
  //
  SG::ReadHandle<xAOD::ElectronContainer> electrons{m_electronContainer, ctx};

  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, double>> SF;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, double>> TotalUp;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, double>>
      TotalDown;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, double>> uncorrUp;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, double>>
      uncorrDown;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, int>> HistIndex;
  std::optional<SG::WriteDecorHandle<xAOD::ElectronContainer, int>> HistBin;
  std::optional<
      SG::WriteDecorHandle<xAOD::ElectronContainer, std::vector<double>>>
      corrUp;
  std::optional<
      SG::WriteDecorHandle<xAOD::ElectronContainer, std::vector<double>>>
      corrDown;
  std::optional<
      SG::WriteDecorHandle<xAOD::ElectronContainer, std::vector<double>>>
      toys;

  if (m_decorate) {
    SF.emplace(m_SF, ctx);
    TotalUp.emplace(m_TotalUp, ctx);
    TotalDown.emplace(m_TotalDown, ctx);
    if (m_doDetail) {
      uncorrUp.emplace(m_uncorrUp, ctx);
      uncorrDown.emplace(m_uncorrDown, ctx);
      HistIndex.emplace(m_HistIndex, ctx);
      HistBin.emplace(m_HistBin, ctx);
      corrUp.emplace(m_corrUp, ctx);
      corrDown.emplace(m_corrDown, ctx);
    }
    if (m_doToys) {
      toys.emplace(m_toys, ctx);
    }
  }

  for (const xAOD::Electron* el : *electrons) {

    Root::TElectronEfficiencyCorrectionTool::Result result;

    // In case we fail
    if (std::abs(el->eta()) > 2.469 || el->pt() < 10000) {
      // if we decorate we need  to set everything to dummy
      // avoid decorating partially the collection ...
      if (m_decorate) {
        SF.value()(*el) = result.SF;
        TotalUp.value()(*el) = result.SF + result.Total;
        TotalDown.value()(*el) = result.SF - result.Total;
        if (m_doDetail) {
          uncorrUp.value()(*el) = result.SF + result.UnCorr;
          uncorrDown.value()(*el) = result.SF - result.UnCorr;
          HistIndex.value()(*el) = result.histIndex;
          HistBin.value()(*el) = result.histBinNum;
          corrUp.value()(*el) = result.Corr;
          corrDown.value()(*el) = result.Corr;
        }
        if (m_doToys) {
          toys.value()(*el) = result.toys;
        }
      }
      continue;
    }

    // Some logic for validity
    double cluster_eta(-9999.9);
    const xAOD::CaloCluster* cluster = el->caloCluster();
    if (!cluster) {
      ATH_MSG_ERROR("ERROR no cluster associated to the Electron \n");
      return StatusCode::FAILURE;
    }
    // we need to use different variables for central and forward electrons
    static const SG::ConstAccessor<uint16_t> accAuthor("author");
    if (accAuthor.isAvailable(*el) &&
        accAuthor(*el) == xAOD::EgammaParameters::AuthorFwdElectron) {
      cluster_eta = cluster->eta();
    } else {
      cluster_eta = cluster->etaBE(2);
    }

    // use et from cluster because it is immutable under syst variations of
    // electron energy scale
    const double energy = cluster->e();
    const double parEta = el->eta();
    const double coshEta = std::cosh(parEta);
    double et = (coshEta != 0.) ? energy / coshEta : 0.;
    // allow for a 5% margin at the lowest pT bin boundary
    if (et < m_lowestEt) {
      et = et * 1.05;
    }

    // do the calculations
    const int status = m_pimpl->calculate(
        static_cast<PATCore::ParticleDataType::DataType>(dataType), runNumber,
        cluster_eta, et, /* in MeV */
        result, (m_mode == Total) /* do work only for Total*/);
    if (!status) {
      ATH_MSG_ERROR("Something went wrong in the calculation");
      return StatusCode::FAILURE;
    }

    // print outs
    ATH_MSG_INFO("--------------------------------------------");
    ATH_MSG_INFO("Electron pt : " << el->pt() << " eta " << el->eta());
    ATH_MSG_INFO("SF  = " << result.SF << " +- " << result.Total);
    if (m_doDetail) {
      ATH_MSG_INFO("SF = " << result.SF << " +- " << result.UnCorr
                           << " stat only ");
      ATH_MSG_INFO("At histo  " << result.histIndex << " at bin "
                                << result.histBinNum);
      ATH_MSG_INFO("Num correlated " << result.Corr.size());
      for (double res : result.Corr) {
        ATH_MSG_INFO("+- " << res);
      }
    }
    if (m_doToys) {
      ATH_MSG_INFO("Run " << result.toys.size() << " Toys ");
      for (double res : result.toys) {
        ATH_MSG_INFO("+- " << res);
      }
    }
    if (m_decorate) {
      SF.value()(*el) = result.SF;
      TotalUp.value()(*el) = result.SF + result.Total;
      TotalDown.value()(*el) = result.SF - result.Total;
      if (m_doDetail) {
        uncorrUp.value()(*el) = result.SF + result.UnCorr;
        uncorrDown.value()(*el) = result.SF - result.UnCorr;
        HistIndex.value()(*el) = result.histIndex;
        HistBin.value()(*el) = result.histBinNum;
        const size_t corrsize = result.Corr.size();
        std::vector<double> up(corrsize, 0);
        std::vector<double> down(corrsize, 0);
        for (size_t i = 0; i < corrsize; ++i) {
          up[i] = result.SF + result.Corr[i];
          down[i] = result.SF - result.Corr[i];
        }
        corrUp.value()(*el) = std::move(up);
        corrDown.value()(*el) = std::move(down);
      }
      if (m_doToys) {
        toys.value()(*el) = std::move(result.toys);
      }
    }

  }  // end of loop over electrons
  return StatusCode::SUCCESS;
}

