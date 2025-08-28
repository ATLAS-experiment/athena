/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef BEAMEFFECTS_LRAVERTEXPOSITIONER_H
#define BEAMEFFECTS_LRAVERTEXPOSITIONER_H 1

#include <tuple>
#include <vector>

#include <TFile.h>
#include <TH3.h>

#include "AthenaBaseComps/AthAlgTool.h"
#include "HepMC_Interfaces/ILorentzVectorGenerator.h"

#include "AthenaKernel/IAthRNGSvc.h"

namespace Simulation
{
  /** @class LRAVertexPositioner

      Generates vertex positions based on a LRA histogram.

  */
  class LRAVertexPositioner : public extends<AthAlgTool, ILorentzVectorGenerator>
  {
  public:
    /** Constructor */
    LRAVertexPositioner(const std::string &t, const std::string &n, const IInterface *p);

    /** Destructor */
    virtual ~LRAVertexPositioner() = default;

    //---

    /** AthAlgTool initialization. */
    virtual StatusCode initialize() override final;

    /** AthAlgTool finalization. */
    virtual StatusCode finalize() override final;

    /** Generate a vertex position from the LRA input. */
    virtual CLHEP::HepLorentzVector *generate(const EventContext &ctx) const override final;


  private:
    Gaudi::Property<std::string> m_FileName{this, "FileName", "<<Unset>>", "LRA input file name."};      //!< LRA input file name.
    Gaudi::Property<std::string> m_HistName{this, "HistName", "<<Unset>>", "LRA input histogram name."}; //!< LRA input histogram name.

    std::unique_ptr<TFile> m_LRAFile; //!< Owning TFile * to the LRA file.
    const TH3F *m_LRAHist = nullptr;  //!< Non-owning TH3F * to the LRA histogram.

    //---

    ServiceHandle<IAthRNGSvc> m_RNGService{this, "RNGService", "AthRNGSvc"};                                                //!< Handle to the Athena RNG service.
    Gaudi::Property<std::string> m_RNGStream{this, "RNGStream", "LRAVertexPositioner", "Stream name for the RNG service."}; //!< Stream name for the RNG service.
    ATHRNG::RNGWrapper *m_RNGEngine ATLAS_THREAD_SAFE{};                                                                    //!< Non-owning RNGWrapper * to the RNG engine.

    //---

    using IntegralTuple = std::tuple<Int_t, Int_t, Int_t, Double_t>; //!< Tuple for [xBin, yBin, zBin, Integral].
    std::vector<IntegralTuple> m_Integral;                           //!< Vector to hold the running integral over bins.

    const TAxis *m_xAxis = nullptr; //!< Non-owning TAxis * to the histograms x-Axis.
    const TAxis *m_yAxis = nullptr; //!< Non-owning TAxis * to the histograms y-Axis.
    const TAxis *m_zAxis = nullptr; //!< Non-owning TAxis * to the histograms z-Axis.
  };
};

#endif //> !BEAMEFFECTS_LRAVERTEXPOSITIONER_H
