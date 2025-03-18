/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "DefectsEmulatorBase.h"
#include "TH2.h"
#include <sstream>
#include "HistUtil.h"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

namespace InDet{
  const std::array<std::string_view,DefectsEmulatorBase::kNHistTypes> DefectsEmulatorBase::s_histNames{
     "rejected",
     "noise"
  };
  const std::array<std::string_view,DefectsEmulatorBase::kNHistTypes> DefectsEmulatorBase::s_histTitles{
     "Rejected",
     "Noise"
  };

  DefectsEmulatorBase::DefectsEmulatorBase(const std::string &name, ISvcLocator *pSvcLocator)
     : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode DefectsEmulatorBase::initializeBase(unsigned int wafer_hash_max){
     ATH_CHECK( m_trackingGeometryTool.retrieve( DisableTool{m_noiseProbability.empty()} ));
     if (!m_noiseProbability.value().empty()) {
        if (m_modulePattern.value().size() != m_noiseProbability.value().size()) {
           ATH_MSG_FATAL("Number of module patterns and noise probabilities  differs: "
                         << m_modulePattern.value().size() << " != " << m_noiseProbability.size() );
           return StatusCode::FAILURE;
        }

        if (m_noiseProbability.value().size() != m_noiseShape.value().size()) {
           ATH_MSG_FATAL("Number of noise probabilities and number of noise shapes differs: "
                         << m_noiseProbability.size() << " != " << m_noiseShape.size());
           return StatusCode::FAILURE;
        }
        ATH_CHECK(m_rndmSvc.retrieve());
        m_rngName = name()+"RandomEngine";

        if (wafer_hash_max>std::numeric_limits<unsigned short>::max()) {
           ATH_MSG_FATAL("Maximjum wafer hash too large (" << wafer_hash_max
                         << "). This algorithm only supports wafer hashes up to "
                         << std::numeric_limits<unsigned short>::max());
           return StatusCode::FAILURE;
        }
        m_noiseParamIdx.resize(wafer_hash_max, static_cast<unsigned short>(m_noiseProbability.size()));
        m_maxNShape=0;
        std::shared_ptr<const Acts::TrackingGeometry> tracking_geometry = m_trackingGeometryTool->trackingGeometry();

        ModuleIdentifierMatchUtil::ModuleData_t module_data;
        std::vector<unsigned int> module_pattern_idx;
        module_pattern_idx.reserve( m_modulePattern.value().size() );

        // Associate noise probabilities and noise shapes to module identifier hashes.
        // The association is done according to the module match criteria. The derived
        // class has to ensure that only modules with umambigous identifier hash are
        // considered.
        using Counter = struct { unsigned int n_detector_elements, n_missing_detector_elements, n_wrong_type,
                                 n_no_matching_pattern, n_detector_elements_of_correct_type; };
        Counter counter {0u,0u,0u,0u,0u};
        tracking_geometry->visitSurfaces([&counter, &module_data, &module_pattern_idx, this](const Acts::Surface *surface_ptr) {
           if (!surface_ptr) return;
           const Acts::Surface &surface = *surface_ptr;
           const Acts::DetectorElementBase*detector_element = surface.associatedDetectorElement();
           if (detector_element) {
              const ActsDetectorElement *acts_detector_element = dynamic_cast<const ActsDetectorElement*>(detector_element);
              if (acts_detector_element) {
                 if (setModuleData(*acts_detector_element, module_data)) {
                    ModuleIdentifierMatchUtil::moduleMatches(m_modulePattern.value(), module_data, module_pattern_idx);
                    if (module_pattern_idx.empty()) {
                       ++counter.n_no_matching_pattern;
                    }
                    else {
                       m_noiseParamIdx.at(acts_detector_element->identifyHash()) = module_pattern_idx.front();
                    }
                    ++counter.n_detector_elements_of_correct_type;
                 }
              }
              else {
                 ++counter.n_wrong_type;
              }
              ++counter.n_detector_elements;
           }
           else {
              ++counter.n_missing_detector_elements;
           }
        }, true /*sensitive surfaces*/);
        ATH_MSG_DEBUG("Visited surfaces with " << counter.n_detector_elements << " / "
                      << (counter.n_missing_detector_elements + counter.n_detector_elements)
                      << " (wrong type " << counter.n_wrong_type << ")"
                      << " expected detector type " << counter.n_detector_elements_of_correct_type
                      << " without match " << counter.n_no_matching_pattern);
        if (counter.n_missing_detector_elements || counter.n_wrong_type>0) {
           ATH_MSG_ERROR("Encountered " << counter.n_wrong_type << " associated detector elements with wrong type and "
                         << counter.n_missing_detector_elements << " surfaces without detector element.");
        }

        // create normalised cummulative distributions of noise shape distributions
        m_noiseShapeCummulative.resize( m_noiseShape.size());
        unsigned int pattern_i=0;
        for (const std::vector<double> &shape : m_noiseShape.value()) {
           m_noiseShapeCummulative[pattern_i].reserve( shape.size());
           double scale=std::accumulate(shape.begin(), shape.end(),0.);
           if (std::abs(scale-1.)>1e-5) {
              ATH_MSG_FATAL("Noise shape integral for pattern " << pattern_i << " not 1. but " << scale);
              return StatusCode::FAILURE;
           }
           scale = 1./scale;
           double sum =0.;
           for (double value : shape) {
              sum += value * scale;
              m_noiseShapeCummulative[pattern_i].push_back( sum );
           }
           m_maxNShape=std::max(m_maxNShape,static_cast<unsigned int>(shape.size()));
           ++pattern_i;
        }
        ATH_CHECK(  (m_noiseProbability.size() == m_noiseShapeCummulative.size()));
     }

     if (!m_histSvc.name().empty() && !m_histogramGroupName.value().empty()) {
        ATH_CHECK( m_histSvc.retrieve() );
        // reserve space for histograms for 6 different sensor types
        constexpr unsigned int n_different_pixel_matrices_max=6;
        m_dimPerHist.reserve(n_different_pixel_matrices_max);
        for (unsigned int hist_type_i=0; hist_type_i<kNHistTypes; ++hist_type_i) {
           m_hist[hist_type_i].reserve(n_different_pixel_matrices_max);
        }

        // create per id-hash histograms
        m_noiseShapeHist.reserve(n_different_pixel_matrices_max);

        unsigned int max_y_axis = (((wafer_hash_max+99)/100+9)/10)*10;
        for (unsigned int hist_type_i=0; hist_type_i<kNHistTypes; ++hist_type_i) {
           {
              HistUtil::StringCat hist_name;
              hist_name << s_histNames.at(hist_type_i) << "_hits_per_module";
              HistUtil::StringCat hist_title;
              hist_title << s_histTitles.at(hist_type_i) << " hits per module";

              HistUtil::ProtectHistogramCreation protect;
              m_moduleHist.at(hist_type_i) = new TH2F(hist_name.str().c_str(), hist_title.str().c_str(),
                                                      100, -0.5, 100-0.5,
                                                      max_y_axis, -0.5, max_y_axis-0.5
                                                      );
           }
           m_moduleHist[hist_type_i]->GetXaxis()->SetTitle("ID hash % 100");
           m_moduleHist[hist_type_i]->GetYaxis()->SetTitle("ID hash / 100");
           if ( m_histSvc->regHist(m_histogramGroupName.value() + m_moduleHist[hist_type_i]->GetName(),m_moduleHist[hist_type_i]).isFailure() ) {
              return StatusCode::FAILURE;
           }
        }
        m_histogrammingEnabled=true;
     }
     return StatusCode::SUCCESS;
  }
  StatusCode DefectsEmulatorBase::finalize(){
     ATH_MSG_INFO( "Total number of rejected RDOs " << m_rejectedRDOs << ", kept " << m_totalRDOs
                   << (m_splitRDOs>0 ?  (", split " + std::to_string(m_splitRDOs) ) : "")
                   << ", additional noise " << m_totalNoise);
     return StatusCode::SUCCESS;
  }

  std::tuple<TH2 *,TH2 *, TH1 *> DefectsEmulatorBase::findHist(unsigned int n_rows, unsigned int n_cols) const {
     unsigned int key=(n_rows << 16) | n_cols;
     std::vector<unsigned int>::const_iterator iter = std::find(m_dimPerHist.begin(),m_dimPerHist.end(), key );
     if (iter == m_dimPerHist.end()) {
        if (m_dimPerHist.size() == m_dimPerHist.capacity()) {
           if (m_dimPerHist.capacity()==0) {
              return std::make_tuple(nullptr,nullptr,nullptr);
           }
           else {
              return std::make_tuple(m_hist[kRejectedHits].back(),
                                     m_hist[kNoiseHits].empty() ? nullptr : m_hist[kNoiseHits].back(),
                                     m_noiseShapeHist.empty() ? nullptr : m_noiseShapeHist.back());
           }
        }
        else {
           // if no "sensor" with this dimensions has been registered yet, create histograms
           // and register it.
           for (unsigned int hist_type_i=0; hist_type_i<kNHistTypes; ++hist_type_i) {
              HistUtil::StringCat name;
              name << s_histNames.at(hist_type_i) << "_hits_" << n_rows << "_" << n_cols;
              HistUtil::StringCat title;
              title << s_histTitles.at(hist_type_i) << "hits for " << n_rows << "(rows) #times " << n_cols << " (columns)";
              {
                 HistUtil::ProtectHistogramCreation protect;
                 m_hist.at(hist_type_i).push_back(new TH2F(name.str().c_str(), title.str().c_str(),
                                           n_cols, -0.5, n_cols-0.5,
                                           n_rows, -0.5, n_rows-0.5
                                           ));
              }
              m_hist[hist_type_i].back()->GetXaxis()->SetTitle("offline column");
              m_hist[hist_type_i].back()->GetYaxis()->SetTitle("offline row");
              if ( m_histSvc->regHist(m_histogramGroupName.value() + name.str(),m_hist[hist_type_i].back()).isFailure() ) {
                 throw std::runtime_error("Failed to register histogram.");
              }
           }
           {
              std::stringstream name;
              name << "noise_shape_" << n_rows << "_" << n_cols;
              std::stringstream title;
              title << "Noise shape for " << n_rows << "(rows) #times " << n_cols << " (columns)";
              m_noiseShapeHist.push_back(new TH1F(name.str().c_str(), title.str().c_str(), m_maxNShape+1, -0.5, m_maxNShape+.5));
              m_noiseShapeHist.back()->GetXaxis()->SetTitle("offline column");
              m_noiseShapeHist.back()->GetYaxis()->SetTitle("offline row");
              if ( m_histSvc->regHist(m_histogramGroupName.value() + name.str(),m_noiseShapeHist.back()).isFailure() ) {
                 throw std::runtime_error("Failed to register histogram.");
              }
           }
           m_dimPerHist.push_back(key);
           return std::make_tuple(m_hist[kRejectedHits].back(),
                                  m_hist[kNoiseHits].empty() ? nullptr : m_hist[kNoiseHits].back(),
                                  m_noiseShapeHist.empty() ? nullptr : m_noiseShapeHist.back()
                                  );
        }
     }
     else {
        return std::make_tuple(m_hist[kRejectedHits].at(iter-m_dimPerHist.begin()),
                               m_hist[kNoiseHits].empty() ? nullptr : m_hist[kNoiseHits].at(iter-m_dimPerHist.begin()),
                               m_noiseShapeHist.empty() ? nullptr : m_noiseShapeHist.at(iter-m_dimPerHist.begin()));
     }
  }

}// namespace closure
