/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_DEFECTSEMULATORBASE_H
#define INDET_DEFECTSEMULATORBASE_H

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "ModuleIdentifierMatchUtil.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsGeometry/ActsDetectorElement.h"

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <string_view>
#include <array>

class TH2;

namespace InDet {

/** Common base class for the specializations of the DefectsEmulatorAlg template.
  * Provides access to debug histograms to optionally monitor the dropped RDOs.
  */
class DefectsEmulatorBase : public AthReentrantAlgorithm {
public:
  DefectsEmulatorBase(const std::string &name,ISvcLocator *pSvcLocator);

  StatusCode initializeBase(unsigned int wafer_hash_max);
  virtual StatusCode finalize() override;

protected:
   /** Set the module data for matching for the given detector element.
    * @param acts_detector_element the detector element in question
    * @param module_data destination for the module data for matching
    * @return True if the given module is to be considered for matching.
    * The implementation has to ensure that only modules are considered which have
    * unique identifier hashes i.e. modules of one detector type.
    */
   virtual bool setModuleData(const ActsDetectorElement &acts_detector_element,
                              ModuleIdentifierMatchUtil::ModuleData_t &module_data) const = 0;

   /** Get rejected hits and noise histograms for a certain module design
    * @param n_rows the number of rows of the module design
    * @param n_cols the number of columns of the module design
    * @return rejected_hits per cell, noise per cell, noise shape histograms
    * calls during execute must be protected by m_histMutex
    */
   std::tuple<TH2 *,TH2 *, TH1 *> findHist(unsigned int n_rows, unsigned int n_cols) const;

   ServiceHandle<ITHistSvc> m_histSvc
      {this,"HistSvc","THistSvc"};
   Gaudi::Property<std::string> m_histogramGroupName
      {this,"HistogramGroupName","", "Histogram group name or empty to disable histogramming"};

   Gaudi::Property<std::vector<std::vector<int> > > m_modulePattern
      {this,"ModulePatterns", {},
       "Integer ranges to select: (0-1) barrel/end-cap range, (2-3) layer, (4-5) eta index range, (6-7) phi index range, "
       "(8-9) module number of columns or strips, (10) both sides (0,1), (11) all rows (0,1)" };

   Gaudi::Property<std::vector<float> > m_noiseProbability
      {this,"NoiseProbability",0., "Probability for a cell to produce a spurious hit per module pattern"};
   Gaudi::Property<std::vector<std::vector<double>> > m_noiseShape
      {this,"NoiseShape",{}, "Shape of noise distribution (e.g. used for Pixel tot distribution or strip time bin distribution."};

   enum EHistType {
      kRejectedHits,
      kNoiseHits,
      kNHistTypes
   };
   static const std::array<std::string_view,kNHistTypes> s_histNames;
   static const std::array<std::string_view,kNHistTypes> s_histTitles;

   mutable std::mutex m_histMutex ;
   // access to the following must be protected by m_histMutex during execute
   mutable std::vector<unsigned int>  m_dimPerHist ATLAS_THREAD_SAFE;
   mutable std::array<TH2 *,kNHistTypes>            m_moduleHist ATLAS_THREAD_SAFE {};
   mutable std::array<std::vector< TH2 *>,2>        m_hist ATLAS_THREAD_SAFE;
   mutable std::vector< TH1 *>                      m_noiseShapeHist ATLAS_THREAD_SAFE;

   // Simple counters for dropped and copied RDOs
   mutable std::atomic<std::size_t>   m_rejectedRDOs {};
   mutable std::atomic<std::size_t>   m_totalRDOs {};
   mutable std::atomic<std::size_t>   m_splitRDOs {};
   mutable std::atomic<std::size_t>   m_totalNoise {};

   PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool
      {this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

   ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};
   std::string m_rngName;
   std::vector<unsigned short> m_noiseParamIdx;
   std::vector<std::vector<float> >  m_noiseShapeCummulative;

   unsigned int m_maxNShape {};

   bool m_histogrammingEnabled = false;

};

}

#endif
