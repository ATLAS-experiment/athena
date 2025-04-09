/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAUTILS_SHOWERDEPTHTOOL_H
#define EGAMMAUTILS_SHOWERDEPTHTOOL_H

// System include(s).
#include <memory>
#include <optional>
#include <utility>

#include <TH1.h>

namespace CP {

class ShowerDepthUtil {
 public:
  ShowerDepthUtil();

  /** Shower depth (in mm) on EM1 vs. eta, considering misalignments **/
  float getCorrectedShowerDepthEM1(float etas1, float phi,
                                   bool isData = true) const;

  /** Shower depth (in mm) on EM2 vs. eta, considering misalignments **/
  float getCorrectedShowerDepthEM2(float etas2, float phi,
                                   bool isData = true) const;

  /** Return the shower depth in R,Z considering misalignments **/
  std::pair<float, float> getCorrectedRZ(float eta, float phi,
                                         bool isData = true,
                                         int sampling = 1) const;

  /** Return the calorimeter displacement in R(Z) for barrel (endcap) **/
  float getRZCorrection(float eta, float phi, bool isData = true) const;

  /** Eta direction from zvertex to the shower in the given sampling **/
  float getCorrectedEtaDirection(float zvertex, float eta, float phi,
                                 bool isData = true, int sampling = 1) const;

  /** Eta direction from samplings 1 and 2 (pointing) **/
  std::optional<float> getCaloPointingEta(float etas1, float etas2, float phi,
                                          bool isData = true) const;

  /** Shower depth (in mm) vs. eta on EM1 **/
  static float getShowerDepthEM1(float etas1);

  /** Shower depth (in mm) vs. eta on EM2 **/
  static float getShowerDepthEM2(float etas2);

  /** Shower depth in R,Z for the given sampling **/
  static std::pair<float, float> getRZ(float eta, int sampling);

  static float getEtaDirection(float zvertex, float R, float z);

 private:
  /** Return TH1* from file given fileName, histoName **/
  static std::unique_ptr<TH1> getHistoFromFile(const char* fileName,
                                               const char* histoName);

  std::unique_ptr<TH1> m_hData;
  std::unique_ptr<TH1> m_hMC;
};

}  // namespace CP

#endif  // ELECTRONISOLATIONSELECTION_SHOWERDEPTHTOOL_H
