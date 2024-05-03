/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAPHYSVALMONITORING_PHOTONCONVERSIONPLOTS_H
#define EGAMMAPHYSVALMONITORING_PHOTONCONVERSIONPLOTS_H

#include "TrkValHistUtils/PlotBase.h"

#include "xAODEgamma/Photon.h"
#include "xAODEventInfo/EventInfo.h"

#include <string>

namespace Egamma{
    class PhotonConversionPlots:public PlotBase {
        public:
          PhotonConversionPlots(PlotBase* pParent, const std::string& sDir, const std::string& sParticleType);
          void fill(const xAOD::Photon& photon, const xAOD::EventInfo& eventInfo);
          
          std::string m_sParticleType;

          TH1* m_nVtx;
          TH1* m_convR;
          TH1* m_convZ;
          TH2* m_convRvsEta;
          TH2* m_convRvsType;
          TH1* m_convType;
          TH1* m_convDeltaEta;
          TH1* m_convDeltaPhi;

        private:
          virtual void initializePlots();
    };
}

#endif