// -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_PIXELDEFECTSEMULATORCONDALG_H
#define INDET_PIXELDEFECTSEMULATORCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ServiceHandle.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelEmulatedDefects.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ITHistSvc.h"

#include "TH2.h"

namespace InDet {
   /** Conditions algorithms for emulating ITK pixel defects.
    * The algorithm mask random pixels and core columns (group of 8 consecutive columns of a chip) as
    * defect. This data can be used to reject hits associated to these pixels or core columns.
    */
   class PixelDefectsEmulatorCondAlg : public AthReentrantAlgorithm
   {
   public:
      PixelDefectsEmulatorCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
      virtual ~PixelDefectsEmulatorCondAlg() override = default;

      virtual StatusCode initialize() override final;
      virtual StatusCode execute(const EventContext& ctx) const override final;
      virtual StatusCode finalize() override final;

   private:
      StatusCode initializeProbabilities();

      ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey
         {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
      Gaudi::Property<float> m_pixelDefectProbability
         {this,"DefectProbability", 1e-4};

      Gaudi::Property<std::vector<unsigned int> > m_matrixColumns
         {this, "MatrixColumns", {} , "Associated defintion is for a matrix with this number of sensor columns."};

      Gaudi::Property<std::vector<float> > m_probabilityModuleHasCoreColumnDefects
         {this, "ProbabilityOfModuleWithCoreColumnDefects", {}, "Probability of a module to have core column defects." };

      Gaudi::Property<std::vector<float> > m_pixelColGroupdDefectProbability
         {this, "CoreColumnDefectProbability", {},
          "Probability of a certain core column defect for modules with core colmn defects."
          "First probability for a certain matrix type means exactly 1 defect core column "
          "and so forth until extactly n defect core columns. Probabilities should add up to 1." };

      std::vector<unsigned int>        m_matrixTypeNColumns;
      std::vector<std::vector<float> > m_perMatrixTypeFractions;

      SG::WriteCondHandleKey<InDet::PixelEmulatedDefects> m_writeKey
         {this, "WriteKey", "", "Key of output PixelDefectsEmulator data"};

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

      const PixelID* m_idHelper = nullptr;
      std::string m_rngName;

      // calls during execute must be protected by m_histMutex
      TH2 *findHist(unsigned int n_rows, unsigned int n_cols) const;
      mutable std::mutex m_histMutex ;
      // during execute the following may only be accessed when m_histMutex is locked
      mutable std::vector<unsigned int>  m_dimPerHist ATLAS_THREAD_SAFE;

      mutable std::vector< TH2 *>        m_hist ATLAS_THREAD_SAFE;
      mutable std::vector< TH1 *>        m_histNCoreColumnDefects ATLAS_THREAD_SAFE;
      mutable TH2 *                      m_moduleDefectsHist ATLAS_THREAD_SAFE = nullptr;
      mutable TH2 *                      m_moduleCoreColDefectsHist ATLAS_THREAD_SAFE = nullptr;
      mutable TH2 *                      m_matrixHist ATLAS_THREAD_SAFE = nullptr;

      bool m_histogrammingEnabled = false;
   };
}
#endif
