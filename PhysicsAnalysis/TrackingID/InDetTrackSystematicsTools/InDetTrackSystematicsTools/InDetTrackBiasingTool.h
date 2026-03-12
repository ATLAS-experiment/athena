// -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_INDETTRACKBIASINGTOOL_H

#include "InDetTrackSystematicsTools/IInDetTrackBiasingTool.h"
#include "InDetTrackSystematicsTools/InDetTrackSystematicsTool.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include <AsgDataHandles/ReadHandleKey.h>
#include "PATInterfaces/CorrectionTool.h"

#include <xAODEventInfo/EventInfo.h>
#include "xAODTracking/TrackParticleContainer.h"

#include <string>
#include <vector>
#include <memory> //for unique_ptr
#include <cstdint> //for uint32_t
#include <TH2.h>

#include "TRandom3.h"

class TFile;

namespace InDet {

  /// @class InDetTrackBiasingTool
  /// This class biases tracks to emulate systematic distortions of the tracking geometry.
  /// In data, it corrects the biases.
  /// In simulation, it applys the biases in the same direction they are observed in the data.
  /// @author Pawel Bruckman (pawel.bruckman.de.renstrom@cern.ch)
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class InDetTrackBiasingTool
    : public virtual IInDetTrackBiasingTool
    , public virtual InDetTrackSystematicsTool
    , public virtual CP::CorrectionTool< xAOD::TrackParticleContainer >
  {

    ASG_TOOL_CLASS( InDetTrackBiasingTool,
        InDet::IInDetTrackBiasingTool )

    public:

    InDetTrackBiasingTool (const std::string& name);
    virtual ~InDetTrackBiasingTool();

    virtual StatusCode initialize() override;
    virtual void prepare() override {};

    /// Computes the tracks origin
    virtual CP::CorrectionCode applyCorrection(xAOD::TrackParticle& track) override;
    virtual CP::CorrectionCode correctedCopy( const xAOD::TrackParticle& in,
                xAOD::TrackParticle*& out ) override;
    virtual CP::CorrectionCode applyContainerCorrection( xAOD::TrackParticleContainer& cont ) override;


    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const override;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const override;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const override;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) override;

  protected:

    StatusCode initHistograms();

    float readHistogram(float fDefault, TH2* histogram, float phi, float eta) const;

    // one entry per configured run period (ordered by ascending run number)
    std::vector<std::unique_ptr<TH2>> m_biasD0Histograms; //!
    std::vector<std::unique_ptr<TH2>> m_biasZ0Histograms; //!
    std::vector<std::unique_ptr<TH2>> m_biasQoverPsagittaHistograms; //!
    std::vector<std::unique_ptr<TH2>> m_biasD0HistErrors; //!
    std::vector<std::unique_ptr<TH2>> m_biasZ0HistErrors; //!
    std::vector<std::unique_ptr<TH2>> m_biasQoverPsagittaHistErrors; //!

    // paths and histogram names in the calibration files
    std::string m_d0_nominal_histName = "d0/d0_theNominal";
    std::string m_z0_nominal_histName = "z0/z0_theNominal";
    std::string m_sagitta_nominal_histName = "sagitta/sagitta_theNominal";
    std::string m_d0_uncertainty_histName = "d0/d0_theUncertainty";
    std::string m_z0_uncertainty_histName = "z0/z0_theUncertainty";
    std::string m_sagitta_uncertainty_histName = "sagitta/sagitta_theUncertainty";

    Gaudi::Property<float> m_biasD0{this, "biasD0", 0.f, "Overall d0 bias (mm)."};
    Gaudi::Property<float> m_biasZ0{this, "biasZ0", 0.f, "Overall z0 bias (mm)."};
    Gaudi::Property<float> m_biasQoverPsagitta{this, "biasQoverPsagitta", 0.f, "Overall QoverP sagitta bias (TeV^-1)."};

    Gaudi::Property<bool> m_applyD0Bias{this, "applyD0Bias", true, "Whether to apply the d0 bias from the calibration map."};
    Gaudi::Property<bool> m_applyZ0Bias{this, "applyZ0Bias", true, "Whether to apply the z0 bias from the calibration map."};
    Gaudi::Property<bool> m_applyQoverPBias{this, "applyQoverPBias", true, "Whether to apply the q/p sagitta bias from the calibration map."};

    Gaudi::Property<bool> m_isMC{this, "isMC", true};
    Gaudi::Property<uint32_t> m_runNumber{this, "runNumber", 0, "Manually override the run number used to select the calibration period."};

    // calibration files and run number bounds for each period, configured per MC campaign via the python config.
    // runNumberBounds has the form {lower0, upper0, upper1, upper2, ...} (N+1 entries for N periods),
    // where period i spans (runNumberBounds[i], runNumberBounds[i+1]].
    // May be omitted when only one calibration file is configured (all run numbers are accepted).
    Gaudi::Property<std::vector<std::string>> m_calibFiles{this, "calibFiles", {}, "Calibration files, one per run period."};
    Gaudi::Property<std::vector<unsigned int>> m_runNumberBounds{this, "runNumberBounds", {}, "Run number boundaries: {lower0, upper0, upper1, ...}. May be omitted when only one calibration file is configured."};

    SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};

  }; // class InDetTrackBiasingTool

} // namespace InDet

#endif
