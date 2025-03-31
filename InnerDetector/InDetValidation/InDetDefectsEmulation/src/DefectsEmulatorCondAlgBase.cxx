/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "DefectsEmulatorCondAlgBase.h"

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandPoisson.h"
#include "CLHEP/Random/RandFlat.h"

#include "HistUtil.h"
#include "ModuleIdentifierMatchUtil.h"
#include <numeric>
#include <cmath>

namespace {
   bool hasExtensions(const std::string &name, const std::string_view &ext) {
      return name.size()>=ext.size() && name.substr(name.size()-ext.size(),ext.size())==ext;
   }
}

namespace InDet{

  DefectsEmulatorCondAlgBase::DefectsEmulatorCondAlgBase(const std::string &name, ISvcLocator *pSvcLocator)
     : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode DefectsEmulatorCondAlgBase::initializeBase(unsigned int n_masks, unsigned int wafer_hash_max){
    ATH_CHECK(m_rndmSvc.retrieve());
    m_rngName.reserve( m_rngPerDefectType.value() ?  kMaskDefects + n_masks : 1);
    if (m_rngPerDefectType.value() ) {
       m_rngName.push_back(name()+"RngModuleDefects");
       m_rngName.push_back(name()+"RngCornerDefects");
       for (unsigned int mask_i=0; mask_i<n_masks; ++mask_i) {
          std::stringstream rng_name;
          rng_name << name() << "RngMaskDefect" << mask_i;
          m_rngName.push_back(rng_name.str());
       }
       assert(kMaskDefects+n_masks == m_rngName.size());
    }
    else {
       m_rngName.push_back(name()+"RandomEngine");
    }
    ATH_CHECK( initializeProbabilities(n_masks) );
    ATH_CHECK( initializeCornerDefects() );
    if (!m_outputFile.empty() && (!hasExtensions(m_outputFile.value(), ".root") && !hasExtensions(m_outputFile.value(), ".json"))) {
       ATH_MSG_ERROR("Output file \"" << m_outputFile.value() << "\" does not have extensions \".root\" or \".json\".");
       return StatusCode::FAILURE;
    }
    for (const std::string &input_file : m_inputFiles.value()) {
       if ((!hasExtensions(input_file, ".root") && !hasExtensions(input_file, ".json"))) {
          ATH_MSG_ERROR("Input file \"" << input_file << "\" does not have extensions \".root\" or \".json\".");
          return StatusCode::FAILURE;
       }
    }

    if (!m_histSvc.name().empty() && !m_histogramGroupName.value().empty()) {
       ATH_CHECK(m_histSvc.retrieve());
       m_histogrammingEnabled=true;
       // allow histogramming for at most 6 different pixel module types
       // histgram for additional module types will end up in the last histogram
       unsigned int n_pattern_for_histogramming = m_fillHistogramsPerPattern ? m_modulePattern.size() : 1;
       constexpr unsigned int n_different_matrices_max=6;
       m_dimPerHist.resize(n_pattern_for_histogramming);
       m_hist.resize(n_pattern_for_histogramming);
       if (!m_groupDefectHistNames.empty()) {
          m_groupDefectHists.resize(n_pattern_for_histogramming);
       }
       for (unsigned int pattern_i=0; pattern_i < n_pattern_for_histogramming; ++pattern_i) {
          m_dimPerHist[pattern_i].reserve(n_different_matrices_max);
          m_hist[pattern_i].reserve(n_different_matrices_max);
       }
       std::vector<std::string> hist_name {
          "matrix_type_id_per_module;Matrix type ID per module",
          "defect_module;Module is defect",
          "defects_per_module;Defect cells per module"};
       for (const std::string &group_name: m_groupDefectHistNames) {
          HistUtil::StringCat hname;
          hname << group_name << "defects_per_module;" << group_name <<" defects per module";
          hist_name.push_back(hname.str());
       }
       m_moduleHist.resize(n_pattern_for_histogramming);
       unsigned int max_y_axis = (((wafer_hash_max+99)/100+9)/10)*10;

       for (unsigned int pattern_i=0; pattern_i<m_moduleHist.size(); ++pattern_i) {
          m_moduleHist.at(pattern_i).resize(hist_name.size(), nullptr);
          for (unsigned int hist_i=0; hist_i<hist_name.size(); ++hist_i) {
             // support idHashes from 0 to 10k
             std::string::size_type pos = hist_name.at(hist_i).find(";");
             HistUtil::StringCat a_name;
             a_name << hist_name[hist_i].substr(0, (pos != std::string::npos ? pos : hist_name[hist_i].size())) << "_" << pattern_i;
             HistUtil::StringCat a_title;
             a_title << hist_name[hist_i].substr((pos != std::string::npos ? pos+1 : 0), hist_name[hist_i].size())
                     << " ( module pattern " << pattern_i << ")";
             {
                HistUtil::ProtectHistogramCreation protect;
                m_moduleHist.at(pattern_i).at(hist_i) = new TH2F(a_name.str().c_str(), a_title.str().c_str(),
                                                                 100, -0.5, 100-0.5,
                                                                 max_y_axis, -0.5, max_y_axis-0.5);
             }
             m_moduleHist[pattern_i][hist_i]->GetXaxis()->SetTitle("ID hash % 100");
             m_moduleHist[pattern_i][hist_i]->GetYaxis()->SetTitle("ID hash / 100");
             ATH_MSG_VERBOSE("Create histogram pattern " << pattern_i << " hist " << hist_i << " name " << a_name.str().c_str()
                             << " -> " << m_moduleHist[pattern_i][hist_i]->GetName());
             if ( m_histSvc->regHist(m_histogramGroupName.value() + m_moduleHist[pattern_i][hist_i]->GetName(),
                                     m_moduleHist[pattern_i][hist_i]).isFailure() ) {
                return StatusCode::FAILURE;
             }
          }
       }

       unsigned int n_eta_phi_histograms = m_fillEtaPhiHistogramsPerPattern ? m_modulePattern.size() : 1;
       for (unsigned int pos_i=0; pos_i<m_defectModuleEtaPhiHist.size(); ++pos_i) {
          m_defectModuleEtaPhiHist.at(pos_i).resize(n_eta_phi_histograms, nullptr);
          m_defectModuleEtaLayerHist.at(pos_i).resize(n_eta_phi_histograms, nullptr);
       }
       std::array<std::string_view, kNPosHists> pos_hist_name {
          "defect_modules",
          "modules_with_defects"
       };
       std::array<std::string_view, kNPosHists> pos_hist_title {
          "Defect modules",
          "Modules with defects"
       };
       for (unsigned pattern_i=0; pattern_i< n_eta_phi_histograms; ++pattern_i) {
          for (unsigned int pos_i=0; pos_i<m_defectModuleEtaPhiHist.size(); ++pos_i) {
             {
                HistUtil::StringCat a_name;
                a_name << pos_hist_name.at(pos_i) << "_zr" << "_" << pattern_i;
                HistUtil::StringCat a_title;
                a_title << pos_hist_title.at(pos_i) << " vs global zr" << " ( module pattern " << pattern_i << ")";
                {
                   HistUtil::ProtectHistogramCreation protect;
                   m_defectModuleEtaPhiHist.at(pos_i).at(pattern_i) = new TH2F(a_name.str().c_str(), a_title.str().c_str(),
                                                                               200, -3000, 3000,
                                                                               200, 0, 1050
                                                                               );
                }
                if ( m_histSvc->regHist(m_histogramGroupName.value() + m_defectModuleEtaPhiHist.at(pos_i).at(pattern_i)->GetName(),
                                        m_defectModuleEtaPhiHist.at(pos_i).at(pattern_i)).isFailure() ) {
                   return StatusCode::FAILURE;
                }
             }
             {
                HistUtil::StringCat a_name;
                a_name << pos_hist_name.at(pos_i) << "_xy" << "_" << pattern_i;
                HistUtil::StringCat a_title;
                a_title << pos_hist_title.at(pos_i) << " vs global xy" << " ( module pattern " << pattern_i << ")";
                {
                   HistUtil::ProtectHistogramCreation protect;
                   m_defectModuleEtaLayerHist.at(pos_i).at(pattern_i) = new TH2F(a_name.str().c_str(), a_title.str().c_str(),
                                                                                 200, -1050,1050,
                                                                                 200, -1050,1050
                                                                                 );
                }
                if ( m_histSvc->regHist(m_histogramGroupName.value() + m_defectModuleEtaLayerHist.at(pos_i).at(pattern_i)->GetName(),
                                        m_defectModuleEtaLayerHist.at(pos_i).at(pattern_i)).isFailure() ) {
                   return StatusCode::FAILURE;
                }
             }
          }
       }

       if (n_masks>1) {
          m_maxNGroupDefects.resize( n_masks-1, 0u);
          for (const std::vector<std::vector<float> >  &fractions_per_pattern : m_perPatternAndMaskFractions) {
             for (unsigned int mask_i=0;  mask_i<n_masks-1; ++mask_i) {
                m_maxNGroupDefects[mask_i] = std::max(m_maxNGroupDefects[mask_i], static_cast<unsigned int>(fractions_per_pattern[mask_i].size()));
             }
          }

          if (m_groupDefectHistNames.size() != n_masks-1) {
             ATH_MSG_FATAL("m_groupDefectHistNames does not contain a name per mask starting from the second mask "
                           "(the first mask must be the full matrix)");
             return StatusCode::FAILURE;
          }
          if (m_maxNGroupDefects.size() != n_masks-1) {
             ATH_MSG_FATAL("m_maxNGroupDefects does not contain a upper bin value for number of expected defects "
                           "per mask starting from the second mask (the first mask must be the full matrix)");
             return StatusCode::FAILURE;
          }
       }
    }

    return StatusCode::SUCCESS;
  }


  StatusCode DefectsEmulatorCondAlgBase::checkProbabilities(unsigned int n_probabilities) const {
     if (!(m_modulePattern.size() == m_defectProbability.size()
           && ModuleIdentifierMatchUtil::verifyModulePatternList(m_modulePattern.value())
           && ModuleIdentifierMatchUtil::verifyElementCount<double>(m_defectProbability.value(), n_probabilities))) {
        if (m_modulePattern.size() != m_defectProbability.size()) {
           ATH_MSG_ERROR( "size difference modulePattern vs defectProbability : " << m_modulePattern.size() << " != " << m_defectProbability.size() << "?");
        }
        bool ret = ModuleIdentifierMatchUtil::verifyModulePatternList(m_modulePattern.value());
        if (!ret) {
           ATH_MSG_ERROR( "Order problem in  modulePattern  : " <<  ret << "?");
        }
        ret = ModuleIdentifierMatchUtil::verifyElementCount<double>(m_defectProbability.value(), n_probabilities);
        if (!ret) {
           ATH_MSG_ERROR( "Size problem in  defectProbability : " << ret << " ( n_prob" << n_probabilities << ")?");
        }
     }
     return    m_modulePattern.size() == m_defectProbability.size()
        && ModuleIdentifierMatchUtil::verifyModulePatternList(m_modulePattern.value())
        && ModuleIdentifierMatchUtil::verifyElementCount<double>(m_defectProbability.value(), n_probabilities)
        ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }

  StatusCode DefectsEmulatorCondAlgBase::finalize(){
     if (m_modulesWithoutDefectParameters>0) {
        ATH_MSG_WARNING("No defect parameters for " << m_modulesWithoutDefectParameters << " modules.");
     }
     return StatusCode::SUCCESS;
  }


  unsigned int DefectsEmulatorCondAlgBase::throwNumberOfDefects(std::span<CLHEP::HepRandomEngine *>rndmEngine,
                                                                const std::vector<unsigned int> &module_pattern_idx,
                                                                unsigned int n_masks,
                                                                unsigned int n_cells,
                                                                std::vector<unsigned int> &n_mask_defects) const
  {
     n_mask_defects.clear();
     n_mask_defects.resize(n_masks, 0u);

     // to avoid throwing random numbers if not necessary
     // first identify the masks for which random numbers need to be thrown.
     std::vector<bool> has(n_masks, false);
     for (unsigned int mask_i=n_masks; mask_i-->1; ) {
        for (unsigned int match_i: module_pattern_idx) {
           assert(!m_perPatternAndMaskFractions.at(match_i).at(mask_i-1).empty());
           if (m_perPatternAndMaskFractions.at(match_i).at(mask_i-1).back()>0.) {
              has[mask_i]=true;
              break;
           }
        }
     }

     for (unsigned int mask_i=n_masks; mask_i-->1; ) {
        assert(mask_i>0 && mask_i<rndmEngine.size());
        float prob = !has.at(mask_i) ? 1. : CLHEP::RandFlat::shoot(rndmEngine[mask_i],1.);

        for (unsigned int match_i: module_pattern_idx) {
           unsigned int n_mask_defects_idx=m_perPatternAndMaskFractions.at(match_i).at(mask_i-1).size();
           for (; n_mask_defects_idx-->0 && prob <= m_perPatternAndMaskFractions[match_i][mask_i-1][n_mask_defects_idx];);
           if (++n_mask_defects_idx < m_perPatternAndMaskFractions[match_i][mask_i-1].size()) {
              n_mask_defects[mask_i] =  n_mask_defects_idx+1;
              break;
           }

           prob -= m_perPatternAndMaskFractions[match_i][mask_i-1].back();
           if (prob<=0.f) break;
        }
     }
     double defect_prob = totalProbability(module_pattern_idx,kCellDefectProb);
     n_mask_defects[0]= static_cast<unsigned int>(std::max(0,static_cast<int>(
                                                   CLHEP::RandPoisson::shoot(rndmEngine[0],
                                                                             n_cells * defect_prob))));
     return std::accumulate(n_mask_defects.begin(),n_mask_defects.end(), 0u);
  }

  void DefectsEmulatorCondAlgBase::printSummaryOfDefectGeneration([[maybe_unused]] unsigned int n_masks,
                                                                  unsigned int n_error,
                                                                  unsigned int n_defects_total,
                                                                  const std::vector<std::array<unsigned int,kNCounts> >&counts) const {
     assert(counts.size() == n_masks+1 );
     msg(MSG::INFO) << "Total cells: " << counts[0][kNElements] << ", module design of wrong type: " << n_error << ", defects "
                    << n_defects_total << ".\n";
     for (unsigned int mask_i=0; mask_i < counts.size(); ++mask_i) {
        msg() << "Defect " << (mask_i==0 ? "cells" : (mask_i>m_groupDefectHistNames.size() ? "modules" : m_groupDefectHistNames.at(mask_i-1)))
              << " " << counts[mask_i][kNDefects] << " / " << counts[mask_i][kNElements];
        if (counts[mask_i][kNRetries]>0 || counts[mask_i][kNMaxRtriesExceeded]>0) {
           msg() << "(";
           if (counts[mask_i][kNRetries]>0) {
              msg() << "retries " << counts[mask_i][kNRetries];
              if (counts[mask_i][kNMaxRtriesExceeded]>0) {
                 msg() << ";";
              }
           }
           if (counts[mask_i][kNMaxRtriesExceeded]>0) {
              msg() << "exceeded max attempts " << counts[mask_i][kNMaxRtriesExceeded];
           }
           msg() << ")";
        }
        if (counts[mask_i][kMaxDefectsPerModule]>0) {
           msg() << " max/module "  <<counts[mask_i][kMaxDefectsPerModule];
        }
        if (mask_i+1<counts.size()) {
           msg() << ".\n";
        }
     }
     msg() << endmsg;
  }

  std::pair<unsigned int,unsigned int> DefectsEmulatorCondAlgBase::findHist(unsigned int pattern_i, unsigned int n_rows, unsigned int n_cols) const {
     unsigned int key=(n_rows << 16) | n_cols;
     assert( pattern_i < m_dimPerHist.size());
     assert( pattern_i < m_hist.size());
     unsigned int matrix_type_id;
     {
        std::vector<unsigned int>::const_iterator global_iter = std::find(m_matrixTypeId.begin(), m_matrixTypeId.end(), key );
        if (global_iter != m_matrixTypeId.end()) {
           matrix_type_id = global_iter - m_matrixTypeId.begin();
        }
        else {
           matrix_type_id =m_matrixTypeId.size();
           m_matrixTypeId.push_back(key);
        }
     }

     std::vector<unsigned int>::const_iterator iter = std::find(m_dimPerHist[pattern_i].begin(), m_dimPerHist[pattern_i].end(), key );
     if (iter == m_dimPerHist[pattern_i].end()) {
        HistUtil::StringCat name;
        name << "defects_" << pattern_i << "_" << (matrix_type_id) << "_" << n_rows << "_" << n_cols;
        HistUtil::StringCat title;
        title << "Defects for " << n_rows << "(rows) #times " << n_cols << " (columns) ID " << (matrix_type_id) << ", pattern " << pattern_i;
        {
           HistUtil::ProtectHistogramCreation protect;
           m_hist[pattern_i].push_back(new TH2F(name.str().c_str(), title.str().c_str(),
                                                n_cols, -0.5, n_cols-0.5,
                                                n_rows, -0.5, n_rows-0.5
                                                ));
        }
        m_hist[pattern_i].back()->GetXaxis()->SetTitle("offline column");
        m_hist[pattern_i].back()->GetYaxis()->SetTitle("offline row");
        if ( m_histSvc->regHist(m_histogramGroupName.value() + name.str(),m_hist[pattern_i].back()).isFailure() ) {
           throw std::runtime_error("Failed to register histogram.");
        }
        m_dimPerHist[pattern_i].push_back(key);

        if (!m_groupDefectHists.empty()) {
           assert( pattern_i < m_groupDefectHists.size());
           assert( m_groupDefectHistNames.size() == m_maxNGroupDefects.size());
           m_groupDefectHists[pattern_i].emplace_back();
           m_groupDefectHists[pattern_i].back().resize(m_groupDefectHistNames.size());
           for (unsigned int group_i=0u; group_i < m_groupDefectHistNames.size(); ++group_i) {
              const std::string &group_name = m_groupDefectHistNames[group_i];
              HistUtil::StringCat hname;
              hname << "n_" << group_name << "_" << pattern_i << "_" << matrix_type_id << "_" << n_rows << "_" << n_cols;
              HistUtil::StringCat htitle;
              htitle << "Number of " << group_name << " defects for " << n_rows << "(rows) #times " << n_cols << " (columns) ID "
                     << (matrix_type_id) << ", pattern " << pattern_i;

              {
                 HistUtil::ProtectHistogramCreation protect;
                 assert(group_i < m_groupDefectHists[pattern_i].back().size());
                 m_groupDefectHists[pattern_i].back()[group_i]= new TH1F(hname.str().c_str(), htitle.str().c_str(),
                                                                            m_maxNGroupDefects[group_i]+1, -0.5,m_maxNGroupDefects[group_i]+.5);
              }
              if (m_histSvc->regHist(m_histogramGroupName.value() + m_groupDefectHists[pattern_i].back()[group_i]->GetName(),
                                     m_groupDefectHists[pattern_i].back()[group_i]).isFailure() ) {
                 throw std::runtime_error("Failed to register histogram.");
              }
           }
        }
        return std::make_pair(static_cast<unsigned int>(m_hist[pattern_i].size()-1), matrix_type_id);
     }
     else {
        return std::make_pair(static_cast<unsigned int>(iter-m_dimPerHist[pattern_i].begin()), matrix_type_id);
     }
  }

  namespace {
     MsgStream &operator<<(MsgStream &out, const std::vector<float> &values) {
        for (float value : values) {
           out << " " << value;
        }
        return out;
     }
  }

  StatusCode DefectsEmulatorCondAlgBase::initializeProbabilities(unsigned int n_masks) {
     if (n_masks>1) {
        if (m_nDefectFractionsPerPattern.size() != m_modulePattern.size()) {
           ATH_MSG_ERROR("The number of fraction lists per pattern does not match the number of module patterns: "
                         << m_nDefectFractionsPerPattern.size() << " != " << m_modulePattern.size());
           return StatusCode::FAILURE;
        }
        if (m_defectProbability.size() != m_modulePattern.size()) {
           ATH_MSG_ERROR("The number of probability lists per pattern does not match the number of module patterns: "
                         << m_defectProbability.size() << " != " << m_modulePattern.size());
           return StatusCode::FAILURE;
        }
        m_perPatternAndMaskFractions.resize( m_nDefectFractionsPerPattern.size() );
        for (unsigned int pattern_i=0; pattern_i< m_perPatternAndMaskFractions.size(); ++pattern_i) {
           m_perPatternAndMaskFractions[pattern_i].reserve( n_masks-1);
           m_perPatternAndMaskFractions[pattern_i].emplace_back();
           if (m_defectProbability[pattern_i].size() != kCellDefectProb + n_masks) {
              ATH_MSG_ERROR("There should be one probability for the module to be defect, one probability for a pixel/strip etc. "
                            "to be defect and one probability for each group to be defect i.e. "
                            << (kCellDefectProb + n_masks) << " probabilities, but there are "
                            << m_defectProbability.size() << " for pattern " << pattern_i);
              return StatusCode::FAILURE;
           }
           double sum=0.;
           for (unsigned int value_i=0; value_i< m_nDefectFractionsPerPattern[pattern_i].size(); ++value_i) {
              if (m_nDefectFractionsPerPattern[pattern_i][value_i]<0.) {
                 if (value_i+1 < m_nDefectFractionsPerPattern[pattern_i].size()) {
                    if (m_perPatternAndMaskFractions[pattern_i].size() == n_masks-1) {
                       ATH_MSG_ERROR("More fraction lists than number of masks: "
                                     << m_perPatternAndMaskFractions[pattern_i].size()+1 << " > " << (n_masks-1)
                                     << " for pattern " << pattern_i);
                       return StatusCode::FAILURE;
                    }
                    m_perPatternAndMaskFractions[pattern_i].emplace_back();
                    sum=0.;
                 }
              }
              else {
                 sum += m_nDefectFractionsPerPattern[pattern_i][value_i];
                 m_perPatternAndMaskFractions[pattern_i].back().push_back(sum);
              }
           }
           for (unsigned int mask_i=0; mask_i< m_perPatternAndMaskFractions[pattern_i].size(); ++mask_i) {
              if (   m_perPatternAndMaskFractions[pattern_i][mask_i].empty()
                  || std::abs(m_perPatternAndMaskFractions[pattern_i][mask_i].back()-1.)>1e-5) {
                 ATH_MSG_ERROR("Empty fraction list or fractions do not add up to 1: "
                               << (!m_perPatternAndMaskFractions[pattern_i][mask_i].empty()
                                   ?m_perPatternAndMaskFractions[pattern_i][mask_i].back() : -1.f)
                               << " for pattern " << pattern_i << ", mask " << mask_i
                               << " (" << m_groupDefectHistNames.at(mask_i) << ")");
                 return StatusCode::FAILURE;
              }
              ATH_MSG_DEBUG("Fractions for pattern " << pattern_i << " mask " << mask_i
                           << " (" << m_groupDefectHistNames.at(mask_i) << "):"
                           << m_perPatternAndMaskFractions[pattern_i][mask_i]);
              assert( pattern_i < m_defectProbability.size() );
              assert( kNProb + mask_i < m_defectProbability[pattern_i].size() );
              for (float &value : m_perPatternAndMaskFractions[pattern_i][mask_i] ) {
                 value *= m_defectProbability[pattern_i][kNProb+mask_i];
              }
              ATH_MSG_DEBUG("Probabilities for pattern " << pattern_i << " mask " << mask_i
                           << " (" << m_groupDefectHistNames.at(mask_i) << ") for 1.."
                           << m_perPatternAndMaskFractions[pattern_i][mask_i].size() << " defects:"
                           << m_perPatternAndMaskFractions[pattern_i][mask_i]);
           }
        }
     }
     return checkProbabilities(kCellDefectProb+n_masks);
   }

   StatusCode DefectsEmulatorCondAlgBase::initializeCornerDefects() {
      if (!m_cornerDefectParamsPerPattern.empty()) {
        if (m_cornerDefectParamsPerPattern.size() != m_modulePattern.size()) {
           ATH_MSG_ERROR("Expected exactly one set of corner defect parameters per module pattern but the numbers disagree "
                         << m_cornerDefectParamsPerPattern.size() << " != " << m_modulePattern.size());
           return StatusCode::FAILURE;
        }
        if (m_cornerDefectParamsPerPattern.size() != m_cornerDefectNCornerFractionsPerPattern.size()) {
           ATH_MSG_ERROR("Expected exactly one set of n corner fractions per module pattern but the numbers disagree "
                         << m_cornerDefectParamsPerPattern.size() << " != " << m_cornerDefectNCornerFractionsPerPattern.size());
           return StatusCode::FAILURE;
        }
        m_perPatternCornerDefectNCornerCummulativeProb.resize( m_cornerDefectParamsPerPattern.size() );
        for (unsigned int pattern_i=0; pattern_i < m_cornerDefectParamsPerPattern.size(); ++pattern_i) {
           if (m_cornerDefectParamsPerPattern[pattern_i].empty()) continue;
           if (m_cornerDefectParamsPerPattern[pattern_i].size() != kNCornerDefectParams) {
              ATH_MSG_ERROR("Mismatch in number of corner defect parameters for pattern " << pattern_i << ". Expected " << kNCornerDefectParams
                            <<  " but got " << m_cornerDefectParamsPerPattern[pattern_i].size() );
              return StatusCode::FAILURE;
           }
           if (m_cornerDefectNCornerFractionsPerPattern[pattern_i].size()>4) {
              ATH_MSG_ERROR("Too many Fractions for corner defects for pattern " << pattern_i <<
                            ". Expected fractions for at most 4 corners but got " << m_cornerDefectNCornerFractionsPerPattern[pattern_i].size()
                            << ".");
              return StatusCode::FAILURE;
           }
           double scale = std::accumulate( m_cornerDefectNCornerFractionsPerPattern.value()[pattern_i].begin(),
                                           m_cornerDefectNCornerFractionsPerPattern.value()[pattern_i].end(),
                                           0.);
           if (std::abs(scale-1.)>1e-3) {
              ATH_MSG_ERROR("The fractions for corner defects in 1.." << m_cornerDefectNCornerFractionsPerPattern[pattern_i].size()
                            << " corner(s) for pattern " << pattern_i << " do not add up to ~1 but " << scale);
              return StatusCode::FAILURE;
           }
           m_perPatternCornerDefectNCornerCummulativeProb[pattern_i].reserve( m_cornerDefectNCornerFractionsPerPattern[pattern_i].size());
           scale = m_cornerDefectParamsPerPattern[pattern_i][kCornerDefectProb]/scale;
           double total = 0.;
           for (double fraction : m_cornerDefectNCornerFractionsPerPattern[pattern_i]) {
              total += fraction * scale;
              m_perPatternCornerDefectNCornerCummulativeProb[pattern_i].push_back( static_cast<float>(total) );
           }
        }
      }
      return StatusCode::SUCCESS;
   }


   void DefectsEmulatorCondAlgBase::histogramDefectModule(unsigned int module_pattern_i,
                                                          unsigned int hist_pattern_i,
                                                          unsigned int id_hash,
                                                          const Amg::Vector3D &center) const {
     assert( hist_pattern_i < m_moduleHist.size() );
     unsigned int ids_per_col = static_cast<unsigned int>(m_moduleHist[hist_pattern_i][kDefectModule]->GetNbinsX());
     unsigned int bin_i=m_moduleHist[hist_pattern_i][kDefectModule]->GetBin( id_hash%ids_per_col+1, id_hash/ids_per_col+1);
     m_moduleHist[hist_pattern_i][kDefectModule]->SetBinContent(bin_i,1);
     unsigned int eta_phi_pattern_i = m_fillEtaPhiHistogramsPerPattern ? module_pattern_i  : 0;
     assert( kDefectModulePos < m_defectModuleEtaPhiHist.size());
     assert( eta_phi_pattern_i < m_defectModuleEtaPhiHist[kDefectModulePos].size() );
     m_defectModuleEtaPhiHist[kDefectModulePos][eta_phi_pattern_i]->Fill(center.z(), center.perp());
     m_defectModuleEtaLayerHist[kDefectModulePos][eta_phi_pattern_i]->Fill(center.x(), center.y());
  }

  void DefectsEmulatorCondAlgBase::fillPerModuleHistograms(unsigned int module_pattern_i,
                                                           unsigned int pattern_hist_i,
                                                           unsigned int matrix_histogram_index,
                                                           unsigned int matrix_index,
                                                           unsigned int module_i,
                                                           unsigned int n_masks,
                                                           const std::vector<unsigned int> &n_mask_defects,
                                                           const Amg::Vector3D &center) const
  {
     // all the following histograms are expected to have the same binning
     // i.e. one bin per ID hash organised in a matrix
     assert( pattern_hist_i < m_moduleHist.size());
     assert( kMatrixTypeId < m_moduleHist[pattern_hist_i].size());
     assert( kDefectModule < m_moduleHist[pattern_hist_i].size());
     unsigned int ids_per_col = static_cast<unsigned int>(m_moduleHist[pattern_hist_i][kDefectModule]->GetNbinsX());
     unsigned int bin_i=m_moduleHist[pattern_hist_i][kDefectModule]->GetBin( module_i%ids_per_col+1, module_i/ids_per_col+1);
     unsigned int mask_i=n_masks;
     for (; --mask_i>0; ) {
        assert( mask_i < n_mask_defects.size() && mask_i>0);
        if (!m_groupDefectHists.empty()) {
           m_groupDefectHists[pattern_hist_i][matrix_histogram_index][mask_i-1]->Fill(n_mask_defects[mask_i]);
        }
        assert( (kDefectCell+mask_i) < m_moduleHist[pattern_hist_i].size());
        m_moduleHist[pattern_hist_i][kDefectCell+mask_i]->SetBinContent(bin_i, n_mask_defects[mask_i]);
     }
     assert( (kDefectCell+mask_i) < m_moduleHist[pattern_hist_i].size());
     m_moduleHist[pattern_hist_i][kDefectCell+mask_i]->SetBinContent(bin_i, n_mask_defects[mask_i]);
     m_moduleHist[pattern_hist_i][kMatrixTypeId]->SetBinContent(bin_i, matrix_index+1 );
     unsigned int n_defects_total = std::accumulate(n_mask_defects.begin(),n_mask_defects.end(), 0u);
     if (n_defects_total>0) {
        unsigned int eta_phi_pattern_i = m_fillEtaPhiHistogramsPerPattern ? module_pattern_i  : 0;
        assert( kModulesWithDefectsPos < m_defectModuleEtaPhiHist.size());
        assert( eta_phi_pattern_i < m_defectModuleEtaPhiHist[kModulesWithDefectsPos].size() );
        m_defectModuleEtaPhiHist[kModulesWithDefectsPos][eta_phi_pattern_i]->Fill(center.z(), center.perp());
        m_defectModuleEtaLayerHist[kModulesWithDefectsPos][eta_phi_pattern_i]->Fill(center.x(), center.y(),n_defects_total);
     }
  }


}// namespace closure
