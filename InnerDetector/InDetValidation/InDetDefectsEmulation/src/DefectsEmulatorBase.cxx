/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// Silicon trackers includes
#include "DefectsEmulatorBase.h"
#include "TH2.h"
#include <sstream>


namespace InDet{
  using namespace InDet;
  DefectsEmulatorBase::DefectsEmulatorBase(const std::string &name, ISvcLocator *pSvcLocator)
     : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode DefectsEmulatorBase::initialize(){
     if (!m_histSvc.name().empty() && !m_histogramGroupName.value().empty()) {
        ATH_CHECK( m_histSvc.retrieve() );
        // allow histogramming for at most 6 different pixel module types
        // histgram for additional module types will end up in the last histogram
        constexpr unsigned int n_different_pixel_matrices_max=6;
        m_dimPerHist.reserve(n_different_pixel_matrices_max);
        m_hist.reserve(n_different_pixel_matrices_max);

        m_moduleHist = new TH2F("rejected_hits_per_module","Rejected hits per module",
                                100, -0.5, 100-0.5,
                                100, -0.5, 100-0.5
                                );
        m_moduleHist->GetXaxis()->SetTitle("ID hash % 100");
        m_moduleHist->GetYaxis()->SetTitle("ID hash / 100");
        if ( m_histSvc->regHist(m_histogramGroupName.value() + m_moduleHist->GetName(),m_moduleHist).isFailure() ) {
           return StatusCode::FAILURE;
        }
        m_histogrammingEnabled=true;
     }
     return StatusCode::SUCCESS;
  }
  StatusCode DefectsEmulatorBase::finalize(){
     ATH_MSG_INFO( "Total number of rejected RDOs " << m_rejectedRDOs << ", kept " << m_totalRDOs);
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
              return m_hist.back();
           }
        }
        else {
           std::stringstream name;
           name << "rejected_hits_" << n_rows << "_" << n_cols;
           std::stringstream title;
           title << "Rejected hits for " << n_rows << "(rows) #times " << n_cols << " (columns)";
           m_hist.push_back(new TH2F(name.str().c_str(), title.str().c_str(),
                                     n_cols, -0.5, n_cols-0.5,
                                     n_rows, -0.5, n_rows-0.5
                                     ));
           m_hist.back()->GetXaxis()->SetTitle("offline column");
           m_hist.back()->GetYaxis()->SetTitle("offline row");
           if ( m_histSvc->regHist(m_histogramGroupName.value() + name.str(),m_hist.back()).isFailure() ) {
              throw std::runtime_error("Failed to register histogram.");
           }
           m_dimPerHist.push_back(key);
           return m_hist.back();
        }
     }
     else {
        return m_hist.at(iter-m_dimPerHist.begin());
     }
  }

}// namespace closure
