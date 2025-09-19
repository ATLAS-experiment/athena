// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_DEFECTSEMULATORCONDALGBASE_H
#define INDET_DEFECTSEMULATORCONDALGBASE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ServiceHandle.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelEmulatedDefects.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH2.h"

#include <span>

namespace CLHEP {
   class HepRandomEngine;
}
namespace InDet {
   /** Conditions algorithms for emulating ITK pixel defects.
    * The algorithm mask random pixels and core columns (group of 8 consecutive columns of a chip) as
    * defect. This data can be used to reject hits associated to these pixels or core columns.
    */
   class DefectsEmulatorCondAlgBase : public AthReentrantAlgorithm
   {
   public:
      DefectsEmulatorCondAlgBase(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~DefectsEmulatorCondAlgBase() override = default;

      virtual StatusCode finalize() override;

   protected:
      StatusCode initializeBase(unsigned int n_masks, unsigned int wafer_hash_max);
      StatusCode initializeProbabilities(unsigned int n_masks);
      StatusCode initializeCornerDefects();

      ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};

      Gaudi::Property<std::vector<std::vector<int> > > m_modulePattern
         {this,"ModulePatterns", {},
          "Integer ranges to select: (0-1) barrel/end-cap range, (2-3) layer, (4-5) eta index range, (6-7) phi index range, "
          "(8-9) module number of columns or strips, (10) both sides (0,1), (11) all rows (0,1)" };
      Gaudi::Property<std::vector<std::vector<double> > > m_defectProbability
         {this,"DefectProbabilities", {},
         "Defect probabilities per module pattern: defect module, defect strip." };
      Gaudi::Property<std::vector<std::vector<double> > > m_nDefectFractionsPerPattern
         {this,"NDefectFractionsPerPattern", {},
         "List of fractions per pattern for exactly 1 to n  defects under the codition that there is a defect, where -1. marks the"
         "end of this lists, before the fractions for the next mask start." };
      Gaudi::Property<std::vector<std::vector<double> > > m_cornerDefectParamsPerPattern
         {this,"CornerDefectParamsPerPattern", {},
         "Set of corner defect parameters (probability, x-intersection pos min, max, y-intersection pos min, max, sagitta minm max). "
         "per module pattern.  " };
      Gaudi::Property<std::vector<std::vector<double> > > m_cornerDefectNCornerFractionsPerPattern
         {this,"NCornerDefectFractionsPerPattern", {},
         "List of fractions per pattern for exactly 1 to 4 corner defects under the codition that there is a defect." };

      std::vector<std::vector<std::vector<float> > > m_perPatternAndMaskFractions;
      std::vector<std::vector<float> > m_perPatternCornerDefectNCornerCummulativeProb;

      Gaudi::Property<bool> m_rngPerDefectType
         {this, "RngPerDefectType",  false, "One RNG per defect type to decorrelate randomness of defects."};

      // Properties to add a checker board like pattern to the defects
      // for debugging
      Gaudi::Property<bool> m_oddRowToggle
         {this, "OddRowToggle",  false};
      Gaudi::Property<bool> m_oddColToggle
         {this, "OddColToggle",  false};
      Gaudi::Property<bool> m_checkerBoardToggle
         {this, "CheckerBoardDefects",  false};
      Gaudi::Property<unsigned int> m_maxAttempts
         {this, "MaxRandomPositionAttempts",  10};

      ServiceHandle<ITHistSvc> m_histSvc{this,"HistSvc","THistSvc"};
      Gaudi::Property<std::string> m_histogramGroupName
         {this,"HistogramGroupName","", "Histogram group name or empty to disable histogramming"};
      Gaudi::Property<bool> m_fillHistogramsPerPattern
         {this, "FillHistogramsPerPattern",  false, "If true, histogram per module defects etc. separately per pattern."};
      Gaudi::Property<bool> m_fillEtaPhiHistogramsPerPattern
         {this, "FillEtaPhiHistogramsPerPattern",  false, "If true, histogram per eta, phi amd z, R are filled separately per pattern."};

      std::vector<std::string> m_rngName;

      enum DefectTypes {
         kModuleDefects,
         kCornerDefects,
         kMaskDefects
      };

      enum EProbabilities {
         kModuleDefectProb,
         kCellDefectProb,
         kNProb
      };

      enum ECornerDefectParams {
         kCornerDefectProb,
         kCornerDefectWidthColumnDirectionOffset,
         kCornerDefectWidthColumnDirection,
         kCornerDefectkWidthRowDirectionOffset,
         kCornerDefectkWidthRowDirection,
         kCornerDefectkSagittaOffset,
         kCornerDefectkSagitta,
         kNCornerDefectParams
      };

      /** Consistency check of module patterns, probabilities.
       */
      StatusCode checkProbabilities(unsigned int n_probabilities) const;

      /** Compute the total probability using the probabilities associated to the given list of patterns.
       * @param module_pattern_idx the index of the patterns
       * @param the index of the probabilities to be used (e.g. module defect probability, single element defect probability)
       * @return sum of the referenced probabilities.
       */
      double totalProbability(const std::vector<unsigned int> &module_pattern_idx, unsigned int prob_idx) const {
         double total_prob=0;
         for (unsigned int pattern_i : module_pattern_idx) {
            assert( prob_idx < m_defectProbability.value().at(pattern_i).size() );
            total_prob += m_defectProbability.value()[pattern_i][prob_idx];
         }
         return total_prob;
      }
      /** Create a cumulative distribution from the referenced set of probabilities
       * @param module_pattern_idx index of patterns for which the associated probabilities are to be considered
       * @param prob_idx the index of the probability (e.g. module defect probability, single element defect probability)
       * @param cumulative_prob output vector to be filled with the cumulative_distribution in ascending order
       * Create a cumulative distributions using the specified probabilities associated to each pattern referenced by
       * module_pattern_idx
       */
      void makeCumulativeProbabilityDist(std::vector<unsigned int> &module_pattern_idx,
                                         unsigned int prob_idx,
                                         std::vector<double> &cumulative_prob) const {
         cumulative_prob.clear();
         double total_prob=0.f;
         for (unsigned int pattern_i : module_pattern_idx) {
            assert( prob_idx < m_defectProbability.value().at(pattern_i).size() );
            total_prob += m_defectProbability.value()[pattern_i][prob_idx];
            cumulative_prob.push_back(total_prob);
         }
      }

      enum ECounts {
         kNElements,
         kNDefects,
         kNRetries,
         kNMaxRtriesExceeded,
         kNModulesWithDefects,
         kMaxDefectsPerModule,
         kNCounts
      };

      // Get number of cell, and group defects and return total number of defects.
      // @param rndmEngine random engine which will be used to throw the random number of defects one per mask.
      // @param module_pattern_idx list of pattern indices which match the module.
      // @param n_masks the number of defect categories e.g. individual pixel, core-column, chip defect.
      // @param n_cels the number cells per module e.g. total number of pixel or strips.
      // @param n_mask_defects output array of number of defects per category.
      // @return total number of defects i.e. the sum of the number of defects per category.
      // n_mask_defects will be resized to n_masks and filled with number of defects per category, where the first element
      // will be the number of individual cell defects.
      unsigned int throwNumberOfDefects(std::span<CLHEP::HepRandomEngine *> rndmEngine,
                                        const std::vector<unsigned int> &module_pattern_idx,
                                        unsigned int n_masks,
                                        unsigned int n_cells,
                                        std::vector<unsigned int> &n_mask_defects) const;

      // Print a message summarizing the defect generation
      // @param n_masks number of masks.
      // @param n_error number of modules with wrong module design.
      // @param n_defects total the total number of generated defects.
      // @param counts more detailed count per mask which is e.g. pixels, core-columns, chips.
      void printSummaryOfDefectGeneration(unsigned int n_masks,
                                          unsigned int n_error,
                                          unsigned int n_defects_total,
                                          const std::vector<std::array<unsigned int,kNCounts> >  &counts) const;

      // Get histogram index and matrix type id for a certain matrix for a certain module pattern.
      // calls during execute must be protected by m_histMutex and must not be called if histogramming is
      // disabled.
      // will return histogram index, and matrix type id
      // usage:
      // <verb>
      // auto [histogram_index, type_id] = findHist(pattern_i, n_rows, n_cols);
      // m_hist[pattern_i][histogram_index] or m_nGroupDefectHistspattern_i][histogram_index]
      // </verb>
      std::pair<unsigned int, unsigned int> findHist(unsigned int pattern_i, unsigned int n_rows, unsigned int n_cols) const;
      void histogramDefectModule(unsigned int module_pattern_i, unsigned int hist_pattern_i, unsigned int id_hash, const Amg::Vector3D &center) const;
      // fill per module histograms
      // @param module_pattern_i the pattern index for which to histogram the position
      // @param pattern_hist_i module pattern index or zero if there is only one set of histograms for all patterns
      // @param matrix_histogram_index an index which is unique for each matrix type defined by the number of rows and columns
      // @param module_i the ID hash
      // @param n_masks the number of maskes defined int the module helper e.g. all pixels, core-columns, chips
      // @param n_mask_defects the number of created defects per mask for the given module
      // @param center global position of the module
      // Will fill the number of defects per module and matrix type  ID per hash ID histograms. Must not be called
      // if histogramming is disabled.
      void fillPerModuleHistograms(unsigned int module_pattern_i,
                                   unsigned int pattern_hist_i,
                                   unsigned int matrix_histogram_index,
                                   unsigned int matrix_index,
                                   unsigned int module_i,
                                   unsigned int n_masks,
                                   const std::vector<unsigned int> &n_mask_defects,
                                   const Amg::Vector3D &center) const;


      std::vector<std::string> m_groupDefectHistNames; // must be filled with one name per nask except for the first one
      std::vector<unsigned int> m_maxNGroupDefects;    // max number of group defects for histograms; one element per nask starting from the second maske 
      mutable std::mutex m_histMutex ;
      // during execute the following may only be accessed when m_histMutex is locked
      mutable std::vector<unsigned int>                        m_matrixTypeId ATLAS_THREAD_SAFE;
      mutable std::vector<std::vector<unsigned int> >          m_dimPerHist ATLAS_THREAD_SAFE;

      mutable std::vector< std::vector< TH2 *> >               m_hist ATLAS_THREAD_SAFE;
      mutable std::vector< std::vector<std::vector< TH1 *> > > m_groupDefectHists ATLAS_THREAD_SAFE;
      mutable std::vector< std::vector< TH2 *> >               m_moduleHist ATLAS_THREAD_SAFE;

      enum EPosHistType {
         kDefectModulePos,
         kModulesWithDefectsPos,
         kNPosHists
      };
      mutable std::array<std::vector<TH2 *>,kNPosHists>        m_defectModuleEtaPhiHist ATLAS_THREAD_SAFE;
      mutable std::array<std::vector<TH2 *>,kNPosHists>        m_defectModuleEtaLayerHist ATLAS_THREAD_SAFE;

      enum EPerModuleHistType {
         kMatrixTypeId,
         kDefectModule,
         kDefectCell,
         kNPerModuleHistTypes
      };

      mutable std::atomic<unsigned int>  m_modulesWithoutDefectParameters {};

      bool m_histogrammingEnabled = false;
   };
}
#endif
