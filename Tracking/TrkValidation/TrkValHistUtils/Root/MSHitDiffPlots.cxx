/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkValHistUtils/MSHitDiffPlots.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "AthContainers/ConstAccessor.h"
// #include "TrkValHistUtils/TrkValHistUtilities.h"

namespace Trk {
  MSHitDiffPlots::MSHitDiffPlots(PlotBase *pParent, const std::string& sDir) :
    PlotBase(pParent, sDir),
    nprecLayers(this, "nprecLayers", "Precision Layers", -5, 5),
    nphiLayers(this, "nphiLayers", "Phi Layers", -5, 5),
    ntrigEtaLayers(this, "ntrigEtaLayers", "Eta Trigger Layers", -5, 5) {
  }

  void
  MSHitDiffPlots::fill(const xAOD::TrackParticle &trkprt, const xAOD::TruthParticle &truthprt, float weight) {
    fillPlot(nprecLayers, xAOD::numberOfPrecisionLayers, "nprecLayers", trkprt, truthprt, weight);
    fillPlot(nphiLayers, xAOD::numberOfPhiLayers, "nphiLayers", trkprt, truthprt, weight);
    fillPlot(ntrigEtaLayers, xAOD::numberOfTriggerEtaLayers, "ntrigEtaLayers", trkprt, truthprt, weight);
  }

  void
  MSHitDiffPlots::fillPlot(HitTypePlots &hitPlots, const xAOD::SummaryType &info, const std::string &sInfo,
                           const xAOD::TrackParticle &trkprt, const xAOD::TruthParticle &truthprt, float weight) {
    uint8_t hitval = 0;

    if (!trkprt.summaryValue(hitval, info)) {
      return;
    }
    SG::ConstAccessor<uint8_t> infoAcc (sInfo);
    if (!infoAcc.isAvailable(truthprt)) {
      return;
    }
    uint8_t truthhitval = infoAcc(truthprt);
    hitPlots.fill(truthhitval - hitval, trkprt.eta(), trkprt.phi(), weight);
  }
}
