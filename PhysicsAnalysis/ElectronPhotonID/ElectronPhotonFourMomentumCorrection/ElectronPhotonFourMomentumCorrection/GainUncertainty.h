/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRONPHOTONFOURMOMENTUMCORRECTION_GAINUNCERTAINTYY_H
#define ELECTRONPHOTONFOURMOMENTUMCORRECTION_GAINUNCERTAINTYY_H

#include <AsgMessaging/AsgMessaging.h>
#include <PATCore/PATCoreEnums.h>

#include <string>

class TH1;

namespace egGain {

class GainUncertainty : public asg::AsgMessaging {
 public:
  GainUncertainty(const std::string& filename, bool splitGainUnc = false,
                  const std::string& name = "GainUncertainty",
                  bool setInterpolation = false);
  ~GainUncertainty();
  enum class GainType { MEDIUM, LOW, MEDIUMLOW };

  // return relative uncertainty on energy from gain uncertainty
  // input etaCalo_input = eta in Calo frame
  //       et_input = Et in MeV
  //       ptype    = particle type

  double getUncertainty(
      double etaCalo_input, double et_input,
      PATCore::ParticleType::Type ptype = PATCore::ParticleType::Electron,
      bool useUncertainty = false,
      GainType gainType = GainType::MEDIUMLOW) const;

 private:
  static const int s_nEtaBins = 5;
  TH1* m_alpha_specialGainRun;
  TH1* m_gain_impact_Zee;
  TH1* m_gain_Impact_elec[s_nEtaBins]{};
  TH1* m_gain_Impact_conv[s_nEtaBins]{};
  TH1* m_gain_Impact_unco[s_nEtaBins]{};
  TH1* m_gain_Impact_elec_medium[s_nEtaBins]{};
  TH1* m_gain_Impact_conv_medium[s_nEtaBins]{};
  TH1* m_gain_Impact_unco_medium[s_nEtaBins]{};
  TH1* m_gain_Impact_elec_low[s_nEtaBins]{};
  TH1* m_gain_Impact_conv_low[s_nEtaBins]{};
  TH1* m_gain_Impact_unco_low[s_nEtaBins]{};

  bool m_useInterpolation;
};

}  // namespace egGain

#endif
