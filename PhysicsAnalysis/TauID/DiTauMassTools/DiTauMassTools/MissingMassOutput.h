/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Class handling the output of the MissingMassCalculator
// author Michael Huebner <michael.huebner@no.spam.cern.ch>

#ifndef DITAUMASSTOOLS_MISSINGMASSOUTPUT_H
#define DITAUMASSTOOLS_MISSINGMASSOUTPUT_H

// local include(s):
#include "DiTauMassTools/HelperFunctions.h"

// ROOT include(s):
#include <TH1.h>
#include <memory> //shared_ptr

namespace DiTauMassTools{
  using ROOT::Math::PtEtaPhiMVector;
  using ROOT::Math::XYVector;

class MissingMassCalculator;

class MissingMassOutput {

  public:
    MissingMassOutput();
    ~MissingMassOutput();

    int GetFitStatus() const; // return fit status
    double GetFittedMass(int fitcode) const; // returns fitted Mass
    double GetFittedMassErrorUp(int fitcode) const; // returns upper error on fitted Mass
    double GetFittedMassErrorLow(int fitcode) const; // returns lower error on fitted Mass
    std::shared_ptr<TH1F> GetMassHistogram() const; // return mass histogram
    std::shared_ptr<TH1F> GetMassHistogramNoWeight() const; // return mass histogram without weights
    int GetNTrials() const;// total number of point scanned
    int GetNSuccesses() const;// total number of point with at least 1 solutions
    int GetNSolutions() const;// total number of solutions
    double GetSumW() const; // sum of weights
    double GetAveSolRMS() const; // ave RMS of solutions (for one event)

    double GetRms2Mpv() const; // returns RMS/MPV according to histogram method
    PtEtaPhiMVector GetNeutrino4vec(int fitcode, int ind) const; // returns neutrino 4-vec
    double GetFitSignificance(int fitcode) const; // returns fit significance
    PtEtaPhiMVector GetTau4vec(int fitcode, int ind) const; // returns full tau 4-vec
    PtEtaPhiMVector GetResonanceVec(int fitcode) const; // returns 4-vec for resonance
    XYVector GetFittedMetVec(int fitcode) const; // returns 2-vec for fitted MET

    friend class MissingMassCalculator;

  private:
    void ClearOutput(bool fUseVerbose);
    int m_FitStatus{};
    double m_FitSignificance[MMCFitMethod::MAX]{};
    double m_FittedMass[MMCFitMethod::MAX]{};
    double m_FittedMassUpperError[MMCFitMethod::MAX]{};
    double m_FittedMassLowerError[MMCFitMethod::MAX]{};
    PtEtaPhiMVector m_nuvec1[MMCFitMethod::MAX]{};
    PtEtaPhiMVector m_objvec1[MMCFitMethod::MAX]{};
    PtEtaPhiMVector m_nuvec2[MMCFitMethod::MAX]{};
    PtEtaPhiMVector m_objvec2[MMCFitMethod::MAX]{};
    PtEtaPhiMVector m_totalvec[MMCFitMethod::MAX]{};
    XYVector m_FittedMetVec[MMCFitMethod::MAX]{};
    double m_RMS2MPV{};
    std::shared_ptr<TH1F> m_hMfit_all;
    std::shared_ptr<TH1F> m_hMfit_allNoWeight;
    int m_NTrials{};
    int m_NSuccesses{};
    int m_NSolutions{};
    double m_SumW{};
    double m_AveSolRMS{};
};
} // namespace DiTauMassTools

#endif
