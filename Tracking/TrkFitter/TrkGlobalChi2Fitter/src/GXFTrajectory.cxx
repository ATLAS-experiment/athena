/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkGlobalChi2Fitter/GXFTrajectory.h"
#include "TrkTrack/GXFMaterialEffects.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkGeometry/MagneticFieldProperties.h"


namespace Trk {
  GXFTrajectory::GXFTrajectory() {
    m_straightline = true;
    m_ndof = 0;
    m_nperpars = -1;
    m_nscatterers = 0;
    m_ncaloscatterers = 0;
    m_nbrems = 0;
    m_nupstreamstates = 0;
    m_nupstreamscatterers = 0;
    m_nupstreamcaloscatterers = 0;
    m_nupstreambrems = 0;
    m_nsihits = 0;
    m_ntrthits = 0;
    m_ntrtprechits = 0;
    m_ntrttubehits = 0;
    m_npseudo = 0;
    m_nhits = 0;
    m_noutl = 0;
    m_nmeasoutl = 0;
    m_chi2 = 0;
    m_prevchi2 = 0;
    m_converged = false;
    m_prefit = 0;
    m_refpar.reset(nullptr);
    m_totx0 = 0;
    m_toteloss = 0;
    m_mass = 0;
    m_caloelossstate = nullptr;
  }

  GXFTrajectory::GXFTrajectory(const GXFTrajectory & rhs)
    : m_straightline (rhs.m_straightline),
      m_fieldprop (rhs.m_fieldprop),
      m_ndof (rhs.m_ndof),
      m_chi2 (rhs.m_chi2),
      m_prevchi2 (rhs.m_prevchi2),
      m_nperpars (rhs.m_nperpars),
      m_nscatterers (rhs.m_nscatterers),
      m_ncaloscatterers (rhs.m_ncaloscatterers),
      m_nbrems (rhs.m_nbrems),
      m_nupstreamstates (rhs.m_nupstreamstates),
      m_nupstreamscatterers (rhs.m_nupstreamscatterers),
      m_nupstreamcaloscatterers (rhs.m_nupstreamcaloscatterers),
      m_nupstreambrems (rhs.m_nupstreambrems),
      m_nhits (rhs.m_nhits),
      m_noutl (rhs.m_noutl),
      m_nsihits (rhs.m_nsihits),
      m_ntrthits (rhs.m_ntrthits),
      m_ntrtprechits (rhs.m_ntrtprechits),
      m_ntrttubehits (rhs.m_ntrttubehits),
      m_npseudo (rhs.m_npseudo),
      m_nmeasoutl (rhs.m_nmeasoutl),
      m_refpar(rhs.m_refpar != nullptr ? rhs.m_refpar->clone() : nullptr),
      m_converged (rhs.m_converged),
      m_scatteringangles (rhs.m_scatteringangles),
      m_scatteringsigmas (rhs.m_scatteringsigmas),
      m_brems (rhs.m_brems),
      m_res (rhs.m_res),
      m_errors (rhs.m_errors),
      m_weightresderiv (rhs.m_weightresderiv),
      m_totx0 (rhs.m_totx0),
      m_toteloss (rhs.m_toteloss),
      m_mass (rhs.m_mass),
      m_prefit (rhs.m_prefit),
      m_caloelossstate (nullptr),
      m_upstreammat (rhs.m_upstreammat)
  {

    m_states.reserve(rhs.m_states.size());
    for (const std::unique_ptr<GXFTrackState> & i : rhs.m_states) {
      m_states.emplace_back(std::make_unique<GXFTrackState>(*i));
    }

    if (rhs.m_caloelossstate != nullptr) {
      for (auto & state : m_states) {
        conditionalSetCalorimeterEnergyLossState(state.get());
      }
    }
  }

  GXFTrajectory & GXFTrajectory::operator =(const GXFTrajectory & rhs) {
    if (this != &rhs) {
      m_straightline = rhs.m_straightline;
      m_fieldprop = rhs.m_fieldprop;
      m_ndof = rhs.m_ndof;
      m_nperpars = rhs.m_nperpars;
      m_nscatterers = rhs.m_nscatterers;
      m_ncaloscatterers = rhs.m_ncaloscatterers;
      m_nbrems = rhs.m_nbrems;
      m_nupstreamstates = rhs.m_nupstreamstates;
      m_nupstreamscatterers = rhs.m_nupstreamscatterers;
      m_nupstreamcaloscatterers = rhs.m_nupstreamcaloscatterers;
      m_nupstreambrems = rhs.m_nupstreambrems;
      m_nsihits = rhs.m_nsihits;
      m_ntrthits = rhs.m_ntrthits;
      m_ntrtprechits = rhs.m_ntrtprechits;
      m_ntrttubehits = rhs.m_ntrttubehits;
      m_nhits = rhs.m_nhits;
      m_npseudo = rhs.m_npseudo;
      m_noutl = rhs.m_noutl;
      m_nmeasoutl = rhs.m_nmeasoutl;
      m_chi2 = rhs.m_chi2;
      m_prevchi2 = rhs.m_prevchi2;
      m_converged = rhs.m_converged;
      m_prefit = rhs.m_prefit;
      m_refpar.reset(rhs.m_refpar != nullptr ? rhs.m_refpar->clone() : nullptr);

      m_states.clear();
      for (const std::unique_ptr<GXFTrackState> & i : rhs.m_states) {
        m_states.push_back(std::make_unique<GXFTrackState>(*i));
      }

      m_scatteringangles = rhs.m_scatteringangles;
      m_scatteringsigmas = rhs.m_scatteringsigmas;
      m_brems = rhs.m_brems;
      m_res = rhs.m_res;
      m_errors = rhs.m_errors;
      m_weightresderiv = rhs.m_weightresderiv;
      m_totx0 = rhs.m_totx0;
      m_toteloss = rhs.m_toteloss;
      m_mass = rhs.m_mass;
      m_caloelossstate = nullptr;

      if (rhs.m_caloelossstate != nullptr) {
        for (auto & state : m_states) {
          conditionalSetCalorimeterEnergyLossState(state.get());
        }
      }
      m_upstreammat = rhs.m_upstreammat;
    }
    return *this;
  }

  void GXFTrajectory::conditionalSetCalorimeterEnergyLossState(GXFTrackState * state) {
    constexpr double perpThreshold = 1400;
    constexpr double zThreshold = 3700;

    const GXFMaterialEffects *meff = state->materialEffects();
    const TrackParameters *par = state->trackParameters();

    if (
      meff != nullptr &&
      par != nullptr &&
      meff->sigmaDeltaE() > 0 &&
      meff->sigmaDeltaPhi() == 0 &&
      (par->position().perp() > perpThreshold || std::abs(par->position().z()) > zThreshold)
    ) {
      m_caloelossstate = state;
    }
  }

  bool GXFTrajectory::addMeasurementState(std::unique_ptr<GXFTrackState> state, int index) {
    if (!m_states.empty() && (m_states.back()->measurement() != nullptr)) {
      const MeasurementBase *meas = m_states.back()->measurement();
      const MeasurementBase *meas2 = state->measurement();

      if (
        &meas->associatedSurface() == &meas2->associatedSurface() &&
        meas->localParameters().parameterKey() == meas2->localParameters().parameterKey() &&
        state->measurementType() != TrackState::MM
      ) {
        return false;
      }
    }

    int nmeas = 0;
    double *errors = state->measurementErrors();

    for (int i = 0; i < 5; i++) {
      if (errors[i] > 0) {
        nmeas++;
      }
    }

    if (state->getStateType(TrackStateOnSurface::Measurement)) {
      m_ndof += nmeas;
    } else {
      m_nmeasoutl += nmeas;
      m_noutl++;
    }

    if (state->measurementType() != TrackState::Pseudo) {
      m_nhits++;
    }

    if (state->measurementType() == TrackState::Pixel
        || state->measurementType() == TrackState::SCT) {
      m_nsihits++;
    }

    if (state->measurementType() == TrackState::TRT) {
      m_ntrthits++;
      if (errors[0]<1) m_ntrtprechits++;
      if (errors[0]>1) m_ntrttubehits++;
    }

    if (state->measurementType() == TrackState::Pseudo) {
      m_npseudo++;
    }

    if (index == -1) {
      m_states.push_back(std::move(state));
    } else {
      m_states.insert(m_states.begin() + index, std::move(state));
    }

    return true;
  }

  void GXFTrajectory::addMaterialState(std::unique_ptr<GXFTrackState> state, int index) {
    GXFMaterialEffects *meff = state->materialEffects();

    if (state->getStateType(TrackStateOnSurface::Scatterer)) {
      m_nscatterers++;

      if (meff->deltaE() == 0) {
        m_ncaloscatterers++;
      }
    }

    if (meff->sigmaDeltaE() > 0) {
      m_nbrems++;
      conditionalSetCalorimeterEnergyLossState(state.get());
    }

    m_toteloss += std::abs(meff->deltaE());
    m_totx0 += meff->x0();

    if (index == -1) {
      m_states.push_back(std::move(state));

      if (meff->sigmaDeltaE() > 0) {
        m_brems.push_back(meff->delta_p());
      }
    } else {
      m_states.insert(m_states.begin() + index, std::move(state));
      int previousbrems = 0;

      for (int i = 0; i < index; i++) {
        if ((m_states[i]->materialEffects() != nullptr)
            && m_states[i]->materialEffects()->sigmaDeltaE() > 0) {
          previousbrems++;
        }
      }

      if (meff->sigmaDeltaE() > 0) {
        m_brems.insert(m_brems.begin() + previousbrems, meff->delta_p());
      }
    }
  }

  void GXFTrajectory::addBasicState(std::unique_ptr<GXFTrackState> state, int index) {
    if (index == -1) {
      m_states.push_back(std::move(state));
    } else {
      m_states.insert(m_states.begin() + index, std::move(state));
    }
  }

  void GXFTrajectory::setReferenceParameters(std::unique_ptr<const TrackParameters> per) {
    if (m_refpar != nullptr) {
      m_refpar = std::move(per);
      return;
    }

    m_refpar = std::move(per);

    m_nupstreamstates = 0;
    m_nupstreamscatterers = 0;
    m_nupstreamcaloscatterers = 0;
    m_nupstreambrems = 0;

    for (const auto & state : m_states) {
      if (state->trackParameters() != nullptr) {
        const double inprod = m_refpar->momentum().dot(state->position() - m_refpar->position());

        if (inprod > 0) {
          return;
        }
      } else {
        const DistanceSolution distsol =
          state->associatedSurface().straightLineDistanceEstimate(m_refpar->position(), m_refpar->momentum().unit());

        double distance = 0;
        if (distsol.numberOfSolutions() == 1) {
          distance = distsol.first();
        } else if (distsol.numberOfSolutions() == 2) {
          distance =
            std::abs(distsol.first()) < std::abs(distsol.second()) ?
            distsol.first() :
            distsol.second();
        }

        if (distance > 0) {
          return;
        }
      }

      m_nupstreamstates++;

      const GXFMaterialEffects *meff = state->materialEffects();

      if (
        state->getStateType(TrackStateOnSurface::Scatterer) &&
        meff->sigmaDeltaTheta() != 0
      ) {
        m_nupstreamscatterers++;

        if (meff->deltaE() == 0) {
          m_nupstreamcaloscatterers++;
        }
      }

      if (meff != nullptr && meff->sigmaDeltaE() > 0) {
        m_nupstreambrems++;
      }
    }
  }

  const TrackParameters *GXFTrajectory::referenceParameters() {
    return m_refpar.get();
  }

  void GXFTrajectory::resetReferenceParameters() {
    m_refpar.reset(nullptr);
  }

  void GXFTrajectory::setNumberOfPerigeeParameters(int nperpar) {
    m_ndof -= nperpar;
    m_nperpars = nperpar;
  }

  void GXFTrajectory::setOutlier(int index, bool isoutlier) {
    if (isoutlier && m_states[index]->getStateType(TrackStateOnSurface::Outlier)) {
      return;
    }

    if (!isoutlier && m_states[index]->getStateType(TrackStateOnSurface::Measurement)) {
      return;
    }

    int nmeas = 0;
    double *errors = m_states[index]->measurementErrors();

    for (int i = 0; i < 5; i++) {
      if (errors[i] > 0) {
        nmeas++;
      }
    }

    if (isoutlier) {
      m_ndof -= nmeas;
      m_states[index]->resetStateType(TrackStateOnSurface::Outlier);
      m_nmeasoutl += nmeas;
      m_noutl++;
      m_states[index]->setFitQuality({});
    } else {
      m_ndof += nmeas;
      m_states[index]->resetStateType(TrackStateOnSurface::Measurement);
      m_nmeasoutl -= nmeas;
      m_noutl--;
    }
  }

  void GXFTrajectory::updateTRTHitCount(int index, float oldError) {
    const double error = (m_states[index]->measurementErrors())[0];
    if (m_states[index]->getStateType(TrackStateOnSurface::Outlier)) {
      if (oldError<1) { m_ntrtprechits--; }
      else {m_ntrttubehits--; }
    }
    if (error>1 && oldError<1) { // was precison, became tube
      m_ntrttubehits++;
      m_ntrtprechits--;
    }
    else if (error<1 && oldError>1) { // was tube, became precision
      m_ntrttubehits--;
      m_ntrtprechits++;
    }
  }

  void GXFTrajectory::setPrefit(int isprefit) {
    m_prefit = isprefit;
  }

  void GXFTrajectory::setConverged(bool isconverged) {
    m_converged = isconverged;
  }

  void GXFTrajectory::reset() {
    m_res.resize(0);
    m_weightresderiv.resize(0, 0);
    m_errors.resize(0);
    m_scatteringangles.clear();
    m_scatteringsigmas.clear();
    m_converged = false;
    m_refpar.reset(nullptr);
  }

  bool GXFTrajectory::converged() const {
    return m_converged;
  }

  int GXFTrajectory::prefit() const {
    return m_prefit;
  }

  int GXFTrajectory::numberOfHits() const {
    return m_nhits;
  }

  int GXFTrajectory::numberOfOutliers() const {
    return m_noutl;
  }

  int GXFTrajectory::numberOfSiliconHits() const {
    return m_nsihits;
  }

  int GXFTrajectory::numberOfTRTHits() const {
    return m_ntrthits;
  }

  int GXFTrajectory::numberOfTRTPrecHits() const {
    return m_ntrtprechits;
  }

  int GXFTrajectory::numberOfTRTTubeHits() const {
    return m_ntrttubehits;
  }

  int GXFTrajectory::numberOfPseudoMeasurements() const {
    return m_npseudo;
  }

  int GXFTrajectory::numberOfScatterers() const {
    if (m_prefit != 0) {
      return m_ncaloscatterers;
    }
    return m_nscatterers;
  }

  void GXFTrajectory::setNumberOfScatterers(int nscat) {
    m_nscatterers = nscat;
  }

  int  GXFTrajectory::numberOfBrems() const {
    return m_nbrems;
  }

  void GXFTrajectory::setNumberOfBrems(int nbrem) {
    m_nbrems = nbrem;
  }

  int GXFTrajectory::numberOfUpstreamStates() const {
    return m_nupstreamstates;
  }

  int GXFTrajectory::numberOfUpstreamScatterers() const {
    if (m_prefit == 0) {
      return m_nupstreamscatterers;
    }
    return m_nupstreamcaloscatterers;
  }

  int GXFTrajectory::numberOfUpstreamBrems() const {
    return m_nupstreambrems;
  }

  int GXFTrajectory::numberOfPerigeeParameters() const {
    return m_nperpars;
  }

  int GXFTrajectory::numberOfFitParameters() const {
    if (m_prefit == 1) {
      return m_nperpars + numberOfBrems() + numberOfScatterers();
    }
    return m_nperpars + numberOfBrems() + 2 * numberOfScatterers();
  }

  double GXFTrajectory::chi2() const {
    return m_chi2;
  }

  double GXFTrajectory::prevchi2() const {
    return m_prevchi2;
  }

  void GXFTrajectory::setChi2(double chi2) {
    m_chi2 = chi2;
  }

  void GXFTrajectory::setPrevChi2(double chi2) {
    m_prevchi2 = chi2;
  }

  int GXFTrajectory::nDOF() const {
    return m_ndof;
  }

  std::vector<std::pair<double, double>> & GXFTrajectory::scatteringAngles() {
    if (m_scatteringangles.empty() && numberOfScatterers() > 0) {
      m_scatteringsigmas.clear();
      m_scatteringsigmas.reserve(numberOfScatterers());
      m_scatteringangles.reserve(numberOfScatterers());
      for (auto & state : m_states) {
        if ((*state).getStateType(TrackStateOnSurface::Scatterer)
            && ((m_prefit == 0) || (*state).materialEffects()->deltaE() == 0)) {
          const double scatphi = (*state).materialEffects()->deltaPhi();
          const double scattheta = (*state).materialEffects()->deltaTheta();
          m_scatteringangles.emplace_back(scatphi, scattheta);
          const double sigmascatphi = (*state).materialEffects()->sigmaDeltaPhi();
          const double sigmascattheta = (*state).materialEffects()->sigmaDeltaTheta();
          m_scatteringsigmas.
            emplace_back(sigmascatphi, sigmascattheta);
        }
      }
    }
    return m_scatteringangles;
  }

  std::vector < std::pair < double, double >>&
    GXFTrajectory::scatteringSigmas() {
    if (m_scatteringsigmas.empty() && numberOfScatterers() > 0) {
      scatteringAngles();
    }
    return m_scatteringsigmas;
  }

  std::vector<double> & GXFTrajectory::brems() {
    return m_brems;
  }

  void

    GXFTrajectory::setScatteringAngles(std::vector < std::pair < double,
                                       double > >&scatteringangles) {
    m_scatteringangles = scatteringangles;
    int scatno = 0;
    for (auto & state : m_states) {
      if ((*state).getStateType(TrackStateOnSurface::Scatterer)
          && ((m_prefit == 0) || (*state).materialEffects()->deltaE() == 0)) {
        const double scatphi = scatteringangles[scatno].first;
        const double scattheta = scatteringangles[scatno].second;
        (*state).materialEffects()->setScatteringAngles(scatphi, scattheta);
        scatno++;
      }
    }
  }

  void
    GXFTrajectory::setBrems(std::vector<double> & brems) {
    // if (m_prefit==1) return;
    m_brems = brems;
    int bremno = 0;
    for (auto & state : m_states) {
      if (((*state).materialEffects() != nullptr)
          && (*state).materialEffects()->sigmaDeltaE() > 0) {
        (*state).materialEffects()->setdelta_p(m_brems[bremno]);
        bremno++;
      }
    }
  }

  const std::vector<std::unique_ptr<GXFTrackState>> & GXFTrajectory::trackStates() const {
    return m_states;
  }

  std::vector<std::unique_ptr<GXFTrackState>> & GXFTrajectory::trackStates() {
    return m_states;
  }

  Amg::VectorX & GXFTrajectory::residuals() {
    if (m_res.size() == 0) {
      m_res.setZero(numberOfBrems() + m_ndof + m_nperpars + m_nmeasoutl);
    }
    return m_res;
  }

  Amg::VectorX & GXFTrajectory::errors() {
    if (m_errors.size() == 0) {
      m_errors.setZero(numberOfBrems() + m_ndof + m_nperpars + m_nmeasoutl);
    }
    return m_errors;
  }

  Amg::MatrixX & GXFTrajectory::weightedResidualDerivatives() {
    if (m_weightresderiv.size() == 0) {
      m_weightresderiv.setZero(
        numberOfBrems() + m_ndof + m_nperpars + m_nmeasoutl,
        numberOfFitParameters()
      );
    }
    return m_weightresderiv;
  }

  double
    GXFTrajectory::totalX0() const {
    return m_totx0;
  }

  double
    GXFTrajectory::totalEnergyLoss() const {
    return m_toteloss;
  }

  double
    GXFTrajectory::mass() const {
    return m_mass;
  }

  void
    GXFTrajectory::setMass(double mass) {
    m_mass = mass;
  }

  GXFTrackState *GXFTrajectory::caloElossState() {
    return m_caloelossstate;
  }

  std::vector < std::pair < const Layer *, const Layer *>>&
    GXFTrajectory::upstreamMaterialLayers() {
    return m_upstreammat;
  }

  std::pair<GXFTrackState *, GXFTrackState *> GXFTrajectory::findFirstLastMeasurement(void) {
    GXFTrackState *firstmeasstate = nullptr;
    GXFTrackState *lastmeasstate = nullptr;

    for (std::unique_ptr<GXFTrackState> & hit : trackStates()) {
      if (hit->measurement() != nullptr) {
        if (firstmeasstate == nullptr) {
          firstmeasstate = hit.get();
        }
        lastmeasstate = hit.get();
      }
    }

    if (firstmeasstate == nullptr) {
      throw std::logic_error("no first measurement.");
    }

    return std::make_pair(firstmeasstate, lastmeasstate);
  }

  bool GXFTrajectory::hasKink(void) {
    for (auto & hit : trackStates()) {
      if (
        hit->measurementType() == TrackState::Pseudo &&
        hit->getStateType(TrackStateOnSurface::Outlier)
      ) {
        continue;
      }

      if (
        (hit->materialEffects() != nullptr) &&
        hit->materialEffects()->isKink()
      ) {
        return true;
      }
    }

    return false;
  }

  void GXFTrajectory::resetCovariances(void) {
    for (std::unique_ptr<GXFTrackState> & hit : trackStates()) {
      hit->setTrackCovariance(nullptr);
    }
  }

  std::unique_ptr<const FitQuality> GXFTrajectory::quality(void) const {
    return std::make_unique<const FitQuality>(chi2(), nDOF());
  }
}
