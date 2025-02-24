/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "DefectsEmulatorBase.h"
#include "TH2.h"
#include <sstream>
#include "HistUtil.h"

namespace InDet{

  const std::array<std::string_view,DefectsEmulatorBase::kNHistTypes> DefectsEmulatorBase::s_histNames{
     "rejected"
  };
  const std::array<std::string_view,DefectsEmulatorBase::kNHistTypes> DefectsEmulatorBase::s_histTitles{
     "Rejected"
  };

  DefectsEmulatorBase::DefectsEmulatorBase(const std::string &name, ISvcLocator *pSvcLocator)
     : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode DefectsEmulatorBase::initializeBase(unsigned int wafer_hash_max){
     if (!m_histSvc.name().empty() && !m_histogramGroupName.value().empty()) {
        ATH_CHECK( m_histSvc.retrieve() );
        // reserve space for histograms for 6 different sensor types
        constexpr unsigned int n_different_pixel_matrices_max=6;
        m_dimPerHist.reserve(n_different_pixel_matrices_max);
        for (unsigned int hist_type_i=0; hist_type_i<kNHistTypes; ++hist_type_i) {
           m_hist[hist_type_i].reserve(n_different_pixel_matrices_max);
        }

        // create per id-hash histograms
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
                   << (m_splitRDOs>0 ?  (", split " + std::to_string(m_splitRDOs) ) : ""));
     return StatusCode::SUCCESS;
  }

  TH2 *DefectsEmulatorBase::findHist(unsigned int n_rows, unsigned int n_cols) const {
     unsigned int key=(n_rows << 16) | n_cols;
     std::vector<unsigned int>::const_iterator iter = std::find(m_dimPerHist.begin(),m_dimPerHist.end(), key );
     if (iter == m_dimPerHist.end()) {
        if (m_dimPerHist.size() == m_dimPerHist.capacity()) {
           if (m_dimPerHist.capacity()==0) {
              return nullptr;
           }
           else {
              return m_hist[kRejectedHits].back();
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
           m_dimPerHist.push_back(key);
           return m_hist[kRejectedHits].back();
        }
     }
     else {
        return m_hist[kRejectedHits].at(iter-m_dimPerHist.begin());
     }
  }

}// namespace closure
