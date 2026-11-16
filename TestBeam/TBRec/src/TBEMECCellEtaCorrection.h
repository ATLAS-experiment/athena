/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBEMECCELLETACORRECTION_H
#define TBREC_TBEMECCELLETACORRECTION_H

/********************************************************************

NAME:     TBEMECCellEtaCorrection.cxx
PACKAGE:  offline/TestBeam/TBRec

AUTHORS:  Andrey Minaenko
CREATED:  08& February 2006

PURPOSE:  Apply correction of EMEC response eta-dependence due to 
          sampling ratio variation on the radius

UPDATED: to have EMEC and FCAL global scale factor
UPDATED: to have also one scale factor per layer (P.Strizenec)

********************************************************************/

#include "CaloUtils/CaloCellCorrection.h"

class LArCell;

class TBEMECCellEtaCorrection : public CaloCellCorrection
{

 public:

    TBEMECCellEtaCorrection (const std::string& type, const std::string& name, 
		const IInterface* parent);

    virtual ~TBEMECCellEtaCorrection();
    virtual StatusCode initialize() override;

    virtual void MakeCorrection (CaloCell* theCell,
                                 const EventContext& ctx) const override;

 private: 

  bool m_EMEC_response_eta_correction{false};
  Gaudi::Property<double> m_EMEC_eta1{this, "LowEta", 2.5, "Low eta boundary of EMEC eta-dependence correction"};
  Gaudi::Property<double> m_EMEC_eta2{this, "MiddleEta", 2.8, "Middle eta boundary of EMEC eta-dependence correction"};
  Gaudi::Property<double> m_EMEC_eta3{this, "HighEta", 3.2, "High eta boundary of EMEC eta-dependence correction"};
  double m_EMEC_eta0_1{0};
  double m_EMEC_eta0_2{0};
  Gaudi::Property<double> m_EMEC_beta1{this, "EMEC_beta1", 1., "EMEC eta-dependence correction beta (first eta range)"};
  Gaudi::Property<double> m_EMEC_alpha1{this, "EMEC_alpha1", 0., "EMEC eta-dependence correction alpha (first eta range)"};
  Gaudi::Property<double> m_EMEC_beta2{this, "EMEC_beta2", 1., "EMEC eta-dependence correction beta (second eta range)"};
  Gaudi::Property<double> m_EMEC_alpha2{this, "EMEC_alpha2", 0., "EMEC eta-dependence correction alpha (second eta range)"};
  // Temporary ramp correction for EMEC cells with eta=2.85
  Gaudi::Property<double> m_ramp_corr{this, "ramp_corr", 1.045, "Temporary ramp correction for EMEC cells with eta=2.85"};
  double m_ramp_corr_eta1{2.8};
  double m_ramp_corr_eta2{2.9};
  // Change mkA->MeV constant with a scale factor
  Gaudi::Property<double> m_EMEC_rescale{this, "EMEC_rescale", 1., "EMEC mkA->MeV scale factor"};
  Gaudi::Property<double> m_FCAL_rescale{this, "FCAL_rescale", 0.9818, "FCAL mkA->MeV scale factor"};
  // layer scale factors
  Gaudi::Property<double> m_EMEC_1_scale{this, "EMEC1_scale", 1., "EMEC layer 1 scale factor"};
  Gaudi::Property<double> m_EMEC_2_scale{this, "EMEC2_scale", 1., "EMEC layer 2 scale factor"};
  Gaudi::Property<double> m_HEC_0_scale{this, "HEC0_scale", 1., "HEC layer 0 scale factor"};
  Gaudi::Property<double> m_HEC_1_scale{this, "HEC1_scale", 1., "HEC layer 1 scale factor"};
  Gaudi::Property<double> m_HEC_2_scale{this, "HEC2_scale", 1., "HEC layer 2 scale factor"};
  Gaudi::Property<double> m_FCAL_0_scale{this, "FCAL0_scale", 1., "FCAL layer 0 scale factor"};
  Gaudi::Property<double> m_FCAL_1_scale{this, "FCAL1_scale", 1., "FCAL layer 1 scale factor"};
};

#endif     




