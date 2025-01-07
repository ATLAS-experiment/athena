/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Silicon trackers includes
#include "PixelDefectsEmulatorCondAlg.h"
#include "InDetRawData/PixelRDORawData.h"

#include "Identifier/Identifier.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "AthenaKernel/RNGWrapper.h"

#include "StoreGate/WriteHandle.h"
#include <typeinfo>

#include "CLHEP/Random/RandPoisson.h"
#include "CLHEP/Random/RandFlat.h"

namespace {
   unsigned int makeCheckerboard(unsigned int pixel_idx, unsigned int n_rows, unsigned int n_columns,
                                 bool odd_row_toggle,
                                 bool odd_col_toggle
                                 ) {
      unsigned int row_i=pixel_idx / n_columns;
      unsigned int col_i = pixel_idx % n_columns;
      unsigned int row_odd = row_i < n_rows/2 || !odd_row_toggle ? 1 : 0;
      unsigned int div_row = n_rows/7;
      unsigned int div_cols = n_columns/7;
      row_i = std::min((((row_i / div_row ) & (0xffff-1)) + row_odd)*div_row + (row_i%div_row), n_rows-1);
      unsigned int col_odd = col_i < n_columns/2 || !odd_col_toggle ? 1 : 0;
      col_i = std::min((((col_i / div_cols ) & (0xffff-1)) + col_odd)*div_cols + (col_i%div_cols),n_columns-1);
      pixel_idx= row_i * n_columns + (col_i % n_columns);
      return pixel_idx;
   }

   class MyLockGuard {
   public:
      MyLockGuard(std::mutex &a_mutex, bool disable)
         : m_mutex( !disable ? &a_mutex : nullptr)
      {
         if (m_mutex) {m_mutex->lock(); }
      }
      ~MyLockGuard() {
         if (m_mutex) {m_mutex->unlock(); }
      }
   private:
      std::mutex *m_mutex;
   };
}

namespace InDet{
  using namespace InDet;
  PixelDefectsEmulatorCondAlg::PixelDefectsEmulatorCondAlg(const std::string &name, ISvcLocator *pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_idHelper(nullptr) {}

  StatusCode PixelDefectsEmulatorCondAlg::initialize(){
    ATH_CHECK(m_rndmSvc.retrieve());
    ATH_CHECK(m_pixelDetEleCollKey.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_idHelper,"PixelID"));
    m_rngName = name()+"RandomEngine";

    if (!m_histSvc.name().empty() && !m_histogramGroupName.value().empty()) {
       ATH_CHECK(m_histSvc.retrieve());
       m_histogrammingEnabled=true;
       // allow histogramming for at most 6 different pixel module types
       // histgram for additional module types will end up in the last histogram
       constexpr unsigned int n_different_pixel_matrices_max=6;
       m_dimPerHist.reserve(n_different_pixel_matrices_max);
       m_hist.reserve(n_different_pixel_matrices_max);
       std::array<TH2 *,3> hists {};
       std::array<std::string,3> hist_name {"defects_per_module",
                                            "corecolumn_defects_per_module",
                                            "matrix_type_id_per_module"};
       std::array<std::string,3> hist_title{"Defects per module",
                                            "Core column defects per module",
                                            "Matrix type id per module"};
       for (unsigned int hist_i=0; hist_i<hists.size(); ++hist_i) {
          // support idHashes from 0 to 10k
          hists.at(hist_i) = new TH2F(hist_name.at(hist_i).c_str(), hist_title.at(hist_i).c_str(),
                                      100, -0.5, 100-0.5,
                                      100, -0.5, 100-0.5
                                      );
          hists[hist_i]->GetXaxis()->SetTitle("ID hash % 100");
          hists[hist_i]->GetYaxis()->SetTitle("ID hash / 100");
          if ( m_histSvc->regHist(m_histogramGroupName.value() + hists[hist_i]->GetName(),hists[hist_i]).isFailure() ) {
             return StatusCode::FAILURE;
          }
       }
       m_moduleDefectsHist = hists[0];
       m_moduleCoreColDefectsHist = hists[1];
       m_matrixHist = hists[2];
    }

    return StatusCode::SUCCESS;
  }

  StatusCode PixelDefectsEmulatorCondAlg::finalize(){
     return StatusCode::SUCCESS;
  }

  StatusCode PixelDefectsEmulatorCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<InDet::PixelEmulatedDefects> defectsOut(m_writeKey, ctx);
    if (defectsOut.isValid()) {
       return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleColl(m_pixelDetEleCollKey,ctx);

    defectsOut.addDependency(pixelDetEleColl);

    unsigned int n_pixel=0u;
    unsigned int n_error=0u;
    unsigned int max_n_defects=0u;
    unsigned int max_n_col_group_defects=0u;
    unsigned int n_col_group_defects_total=0u;
    std::unique_ptr<InDet::PixelEmulatedDefects> defects = std::make_unique<InDet::PixelEmulatedDefects>();
    defects->m_detectorElements=pixelDetEleColl.cptr();
    defects->resize( pixelDetEleColl.cptr()->size());
    unsigned int n_defects_total=0;
    {
       ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_rngName);
       rngWrapper->setSeed( m_rngName, ctx );
       CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);
       unsigned int module_i=0;
       --module_i;

       for (const InDetDD::SiDetectorElement *det_ele:  *(pixelDetEleColl.cptr())) {
          ++module_i;
          PixelModuleHelper helper(det_ele->design());

          if (!helper) {
             ++n_error;
             continue;
          }
          unsigned int pixels = helper.nPixels();
          n_pixel += pixels;

          // bad to lock for the entire loop, but this algorithm is intended to run
          // only once per job anyway
          MyLockGuard lock(m_histMutex, m_histogrammingEnabled);
          TH2 *h2=findHist(helper.nSensorRows(), helper.nSensorColumns());

          unsigned int n_col_group_defects=static_cast<unsigned int>(std::max(0,static_cast<int>(CLHEP::RandPoisson::shoot(rndmEngine,helper.nSensorColumns()
                                                                                                                           * m_pixelColGroupdDefectProbability.value()))));
          n_col_group_defects_total += n_col_group_defects;
          max_n_col_group_defects = std::max(max_n_col_group_defects, n_col_group_defects);

          unsigned int n_defects=static_cast<unsigned int>(std::max(0,static_cast<int>(CLHEP::RandPoisson::shoot(rndmEngine,pixels * m_pixelDefectProbability.value()))));
          max_n_defects = std::max(max_n_defects,n_defects);

          std::vector<unsigned int> &module_defects=(*defects).at(module_i);
          module_defects.reserve(n_defects + n_col_group_defects);
          for (unsigned int defect_i=0; defect_i < n_col_group_defects; ++defect_i) {
             unsigned int pixel_idx=CLHEP::RandFlat::shoot(rndmEngine,pixels); // %pixels;

             // accumulate defects on checker board
             if (m_checkerBoardToggle) {
                pixel_idx=makeCheckerboard(pixel_idx,helper.nSensorRows(), helper.nSensorColumns(), m_oddRowToggle.value(), m_oddColToggle.value() );
             }

             unsigned int key = helper.columnGroupDefect(pixel_idx / helper.nSensorColumns(), pixel_idx % helper.nSensorColumns());
             auto [insert_iter,end_iter] = PixelEmulatedDefects::lower_bound( module_defects, key);
             if (insert_iter == end_iter) {
                module_defects.push_back(key);
                if (h2) {
                   std::array<unsigned int,4> ranges_row_col = helper.offlineRange(key);
                   for (unsigned int row_i=ranges_row_col[0]; row_i<ranges_row_col[1]; ++row_i) {
                      for (unsigned int col_i=ranges_row_col[2]; col_i<ranges_row_col[3]; ++col_i) {
                         h2->Fill(col_i, row_i);
                      }
                   }
                }

             }
             else {
                if (!helper.isSameDefectWithGroups(*insert_iter, key,helper.columnGroupRowColumnMask())) {
                   module_defects.insert( insert_iter, key);
                   if (h2) {
                      std::array<unsigned int,4> ranges_row_col = helper.offlineRange(key);
                      for (unsigned int row_i=ranges_row_col[0]; row_i<ranges_row_col[1]; ++row_i) {
                         for (unsigned int col_i=ranges_row_col[2]; col_i<ranges_row_col[3]; ++col_i) {
                            h2->Fill(col_i, row_i);
                         }
                      }
                   }
                }
             }
          }
          unsigned int n_col_group_defects_registered=module_defects.size();

          for (unsigned int defect_i=0; defect_i < n_defects; ++defect_i) {
             unsigned int pixel_idx=CLHEP::RandFlat::shoot(rndmEngine,pixels); // %pixels;

             // accumulate defects on checker board
             if (m_checkerBoardToggle) {
                pixel_idx=makeCheckerboard(pixel_idx,helper.nSensorRows(), helper.nSensorColumns(), m_oddRowToggle.value(), m_oddColToggle.value());
             }

             unsigned int key = helper.hardwareCoordinates(pixel_idx / helper.nSensorColumns(), pixel_idx % helper.nSensorColumns());
             // order keys in descending order
             // such that lower_bound with greater will return the matching element or the element before
             auto [insert_iter,end_iter] = PixelEmulatedDefects::lower_bound( module_defects, key);
             if (insert_iter == end_iter) {
                module_defects.push_back(key);
                if (h2) {
                   std::array<unsigned int,4> ranges_row_col = helper.offlineRange(key);
                   for (unsigned int row_i=ranges_row_col[0]; row_i<ranges_row_col[1]; ++row_i) {
                      for (unsigned int col_i=ranges_row_col[2]; col_i<ranges_row_col[3]; ++col_i) {
                         h2->Fill(col_i, row_i);
                      }
                   }
                }
             }
             else if (helper.isSameDefectWithGroups(*insert_iter, key,helper.columnGroupRowColumnMask())) {
                continue;
             }
             else {
                module_defects.insert( insert_iter, key);
                if (h2) {
                   std::array<unsigned int,4> ranges_row_col = helper.offlineRange(key);
                   for (unsigned int row_i=ranges_row_col[0]; row_i<ranges_row_col[1]; ++row_i) {
                      for (unsigned int col_i=ranges_row_col[2]; col_i<ranges_row_col[3]; ++col_i) {
                         h2->Fill(col_i, row_i);
                      }
                   }
                }
             }
          }
          if (m_histogrammingEnabled) {
             // all the following histograms are expected to have the same binning
             // i.e. one bin per ID hash organised in a matrix
             unsigned int ids_per_col = static_cast<unsigned int>(m_moduleDefectsHist->GetNbinsX());
             unsigned int bin_i=m_moduleDefectsHist->GetBin( module_i%ids_per_col+1, module_i/ids_per_col+1);
             m_moduleDefectsHist->SetBinContent(bin_i, module_defects.size()-n_col_group_defects_registered );
             m_moduleCoreColDefectsHist->SetBinContent(bin_i, n_col_group_defects_registered );
             // get the matrix "ID"
             for (unsigned int hist_i=0; hist_i < m_hist.size(); ++hist_i) {
                if (m_hist[hist_i]==h2) {
                   m_matrixHist->SetBinContent(bin_i, hist_i+1);
                   break;
                }
             }
          }
          n_defects_total+=module_defects.size();
       }
    }


    ATH_CHECK( defectsOut.record (std::move(defects)) );

    ATH_MSG_INFO("Total pixel " << n_pixel << " non-pixel modules " << n_error << " defects " << n_defects_total << " max /mod " << max_n_defects
                 << " core column defects " << n_col_group_defects_total << " max. / mod " << max_n_col_group_defects);

    return StatusCode::SUCCESS;
  }

  TH2 *PixelDefectsEmulatorCondAlg::findHist(unsigned int n_rows, unsigned int n_cols) const {
     unsigned int key=(n_rows << 16) | n_cols;
     std::vector<unsigned int>::const_iterator iter = std::find(m_dimPerHist.begin(),m_dimPerHist.end(), key );
     if (iter == m_dimPerHist.end()) {
        if (m_dimPerHist.size() == m_dimPerHist.capacity()) {
           if (m_dimPerHist.capacity()==0) {
              return nullptr;
           }
           return m_hist.back();
        }
        else {
           std::stringstream name;
           name << "defects_" << (m_hist.size()+1) << "_" << n_rows << "_" << n_cols;
           std::stringstream title;
           title << "Defects for " << n_rows << "(rows) #times " << n_cols << " (columns) ID " << (m_hist.size()+1);
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
