/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCONDITIONSALGS_HITMAPBUILDER_H
#define PIXELCONDITIONSALGS_HITMAPBUILDER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>
#include <utility> // pair

// EDM
#include "InDetRawData/PixelRDO_Container.h"

#include "TH1.h"
#include "TH2.h"


class PixelID;


namespace InDetDD{
  class PixelDetectorManager;
}

/**
 *
 * HitMapBuilder.h
 *
 * Creates hit maps and maps of noisy pixels for every module
 * of the pixel detector. The maps are stored in a root file.
 * They can be written to the conditions database using the
 * algorithm NoiseMapDBWriter.
 *
 * ruwiedel@physik.uni-bonn.de
 *
 */

class HitMapBuilder: public AthAlgorithm {

 public:

  //Delegate to base-class constructor
  using AthAlgorithm::AthAlgorithm;

  ~HitMapBuilder();

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 private:
  std::string getDCSIDFromPosition(int bec, int layer, int modPhi, int modEta);
  std::vector<std::string> &splitter(const std::string &str, char delim, std::vector<std::string> &elems);
  std::vector<std::string> splitter(const std::string &str, char delim);
  StatusCode registerHistograms();
  const std::string histoSuffix(const int bec, const int layer);

 private:
  ServiceHandle <ITHistSvc> m_tHistSvc{this, "THistSvc", "THistSvc/THistSvc"};
  SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this,"PixelRDOKey","PixelRDOs","StoreGate Key of Pixel RDOs"};

  const InDetDD::PixelDetectorManager *m_pixman=nullptr;
  const PixelID *m_pixelID=nullptr;

  // vector of modulename and vector(barrel/endcap, layer, phi, eta)
  std::vector< std::pair< std::string, std::vector<int> > > m_pixelMapping;

  double m_nEvents=0;
  std::vector<double> m_nEventsLB; // Events per LB
  std::vector<double> m_nEventsLBCategory; // Events per certain LB for LB category

  std::unique_ptr<TH1F> m_nEventsHist;
  std::unique_ptr<TH1F> m_nEventsLBHist;
  std::vector<std::unique_ptr<TH2F>> m_occupancyMaps;
  std::vector<std::unique_ptr<TH2F>> m_occupancyMapsIBL2dLB;
  std::vector<std::unique_ptr<TH1F>> m_TOTdistributions;
  std::vector<std::unique_ptr<TH1F>> m_TOTdistributionsIBL2dLB;
  std::vector<std::unique_ptr<TH1F>> m_occupancyLB;

  Gaudi::Property<int> m_hist_lbMax{this,"nLBmax", 3001, "Maximum number of LB (for histograms binning)"};   // max number of LB
  Gaudi::Property<int> m_evt_lbMin{this,"LBMin",0,"First lumi block to consider"}; // lower limit for LB to be taken into account
  Gaudi::Property<int> m_evt_lbMax{this,"LBMax",-1,"Last lumi block to consider"}; // upper limit for LB to be taken into account

  int m_LBrange_max = -9999;

  const unsigned int m_nIblFes = 14 * (4 + 6*2) * 2; // 14 stave * (4 3Ds + 6 2Ds * 2 Fes) * 2 sides = 448
  const int m_perLB_min=0, m_perLB_max=3000, m_perLB_step=100; // For plots per certain LBs
  const int m_perLB_n = (m_perLB_max-m_perLB_min)/m_perLB_step; // For plots per certain LBs

  const int m_fei4bPixels = 26880; // 80 * 336
  const int m_pixModPixels = 46080; // 144 * 320;
};


#endif // PIXELCONDITIONSALGS_HITMAPBUILDER_H
