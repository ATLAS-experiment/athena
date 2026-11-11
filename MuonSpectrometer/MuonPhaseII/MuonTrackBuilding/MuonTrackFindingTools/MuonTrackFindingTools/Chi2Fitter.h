/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_FITTER_H
#define MUONTRACKFINDINGTOOLS_FITTER_H

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackProxyConcept.hpp"
#include "Acts/EventData/Types.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/detail/CorrectedTransformationFreeToBound.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Material/Interactions.hpp"
#include "Acts/Propagator/DirectNavigator.hpp"
#include "Acts/Propagator/PropagatorOptions.hpp"
#include "Acts/Propagator/PropagatorState.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/detail/PointwiseMaterialInteraction.hpp"
#include "Chi2FitterError.h"
#include "Acts/TrackFitting/detail/VoidFitterComponents.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/TrackHelpers.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/MathHelpers.hpp"

#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>

using namespace Acts;
using namespace Acts::UnitLiterals;
namespace MuonExperimental {

/// Forward declaration
struct Gx2fSystem;
/// @addtogroup track_fitting
/// @{


/// @brief A container to store material properties for each material surface
///
/// This struct holds the scattering angles, the inverse covariance of the
/// material, and a validity flag indicating whether the material is valid for
/// the scattering process.
struct Gx2fMaterialProperties {
  /// Auxiliary struct to include the measured energy loss at a surface. If the
  /// fitter reaches a surface
  /// with material a delegate is called to evaluate the measured energy loss at
  /// the surface. If the struct is valid the measured loss and its associated
  /// uncertainty are plugged into the chi2 system. Otherwise, the energy loss
  /// from the tracking geometry material is taken.
  struct ELossAtSurface {
    /// Is the measured energy loss valid
    /// @returns Flag indicating the energy loss validity
    constexpr bool isValid() const { return m_isValid; }

    /// The amount of lost energy (Bethe bloch / measurement)
    /// @returns The lost energy at the surface
    constexpr double lostEnergy() const { return m_lostEnergy; }
    /// The uncertainty on the energy loss
    /// @return  Uncertainty on the lost energy at the surface
    constexpr double lostSigma() const { return m_lossSigma; }

    /// Default constructor to construct an invalid energy loss
    ELossAtSurface() = default;

    /// Constructor taking the measured energy loss and its uncertainty
    /// @param energy: The measured lost energy at the corresponding surface
    /// @param sigma: The uncertainty on the measured lost energy
    explicit ELossAtSurface(const double energy, const double sigma) noexcept
        : m_isValid{std::min(energy, sigma) > Acts::s_epsilon},
          m_lostEnergy{energy},
          m_lossSigma{sigma} {}

    /// Absorb the energy loss from another object.
    ELossAtSurface& operator+=(const ELossAtSurface& other) noexcept {
      if (&other != this && other.isValid()) {
        m_lostEnergy += other.m_lostEnergy;
        m_lossSigma = Acts::fastHypot(m_lossSigma, other.lostSigma());
        m_isValid = true;
      }
      return *this;
    }

    /// Function that satisfies the @ref eLossAccumulator delegate pattern and the produces an invalid energy loss
    /// at any passed material surface
    /// @param geoContext:The geometry context to align the material surface
    /// @param calibContext: The calibration context to retrieve the detector conditions to apply the calibration on the
    ///                      contributing calorimeter cells
    /// @param pos: Current position along the trajectory (global frame)
    /// @param dir:  Trajectory direction at the current position (glboal frame)
    /// @param matSurface: The material surface at which the propagtions stops
    /// @return An invalid EnergyLoss such that the loss will be estimated from the Bethe-Bloch formula
    static ELossAtSurface invalidELoss(const GeometryContext& geoContext,
                                       const CalibrationContext& calibContext,
                                       const Vector3& pos, const Vector3& dir,
                                       const Surface& matSurface);

   private:
    /// Flag toggling whether the parameters are valid
    bool m_isValid{false};
    /// The amount of lost energy at a given surface
    double m_lostEnergy{0.};
    /// The uncertainty on the lost energy
    double m_lossSigma{0.};
  };

  /// Auxiliary struct to describe the material scattering at a surface
  struct ScatteringAtSurface {
    /// Is the scattering uncertainty valid
    constexpr bool isValid() const { return m_isValid; }
    /// Uncertainty on the scattering angle
    constexpr double sigma() const { return m_sigma; }
    /// Default constructor to construct an invalid scatterer
    ScatteringAtSurface() = default;
    /// Constructor taking the width of the scatterer
    /// @param sigma: The width of the scatterer
    explicit ScatteringAtSurface(const double sigma)
        : m_isValid{sigma > Acts::s_epsilon}, m_sigma{sigma} {}

    ScatteringAtSurface& operator+=(const ScatteringAtSurface& other) {
      if (&other != this && other.isValid()) {
        m_sigma += other.sigma();
        m_isValid = true;
      }

      return *this;
    }
    /// Flag toggling whether the scatterer is valid
    bool m_isValid{false};
    /// The inverse uncertainty on the scattering angles calculated
    /// using e.g. the Highland formula
    double m_sigma{0.};
  };

  /// @brief Constructor to initialize material properties.
  ///
  /// @param scatteringAngles_ The vector of scattering angles.
  /// @param invCovarianceMaterial_ The inverse covariance of the material.
  /// @param materialIsValid_ A boolean flag indicating whether the material is valid.
  explicit Gx2fMaterialProperties(ScatteringAtSurface&& scatterer,
                                  ELossAtSurface&& eLoss)
      : m_scatterer{std::move(scatterer)},
        m_eloss{std::move(eLoss)},
        m_lostEnergy{m_eloss.isValid() ? m_eloss.lostEnergy() : 0.} {}

  /// @brief Accessor for the material validity flag
  /// @return True if material is valid for scattering calculations, false for vacuum or zero thickness
  constexpr bool materialIsValid() const {
    return m_scatterer.isValid() || m_eloss.isValid();
  }

  /// Number of dimensions from the material scatterer
  /// @return Number of dimensions from the material scatterer to the parameter vector
  constexpr std::size_t nDim() const {
    return (m_scatterer.isValid() ? 2ul : 0ul) +
           (m_eloss.isValid() ? 1ul : 0ul);
  }

  const ScatteringAtSurface& scatterer() const { return m_scatterer; }

  const ELossAtSurface& energyLoss() const { return m_eloss; }

  constexpr double deltaTheta() const { return m_scatTheta; }

  constexpr double deltaPhi() const { return m_scatPhi; }

  constexpr double lostEnergy() const { return m_lostEnergy; }

  constexpr double qOverPderiv() const { return m_qOverPderiv; }

  constexpr double qOverPratio() const { return m_qOverPratio; }

  /// Update the track parameters with the scattering angles and
  //  subtract the energy loss from the q/p parameters
  /// @param trackPars The bound track parameters fetched from the stepper which
  /// will be fed back to the stepper to adjust the trajectory position
  void updateTrackParameters(BoundTrackParameters& trackPars);

  void contributionToGx2fSums(Gx2fSystem& extendedSystem, const double theta,
                              const std::size_t firstParamIdx,
                              const Logger& logger) const;

  void updateParameters(const Eigen::VectorXd& deltaParamsExtended,
                        const std::size_t firstParamIdx);

 private:
  /// Description of the scattering angles
  ScatteringAtSurface m_scatterer{};
  /// Description of the energy loss
  ELossAtSurface m_eloss{};

  ///
  /// Fitted parameters
  ///

  /// Fitted energy loss at the surface
  double m_lostEnergy{0.};
  /// Fitted delta theta scattering
  double m_scatTheta{0.};
  /// Fitted delta phi scattering
  double m_scatPhi{0.};
  /// qOverP derivative with respect to eloss
  double m_qOverPderiv{0.};
  // qoverP ratio after and before the energy loss
  double m_qOverPratio{1.};
};

/// Extension struct which holds delegates to customise the GX2F behaviour
template <typename track_container_t>
struct Gx2FitterExtensions {

  using TrackStateContainerBackend = typename track_container_t::TrackStateContainerBackend;
  /// Type alias for mutable track state proxy from multi-trajectory
  using TrackStateProxy = typename TrackStateContainerBackend::TrackStateProxy;
  /// Type alias for const track state proxy from multi-trajectory
  using ConstTrackStateProxy = typename TrackStateContainerBackend::ConstTrackStateProxy;
  /// Type alias for track parameters from track state proxy
  using Parameters = typename TrackStateProxy::Parameters;
  /// Type alias for the track proxy backend
  using TrackProxy = typename track_container_t::TrackProxy;
  /// Type alias for the ConstTrack proxy backend
  using ConstTrackProxy = typename track_container_t::ConstTrackProxy;
   

  /// Type alias for calibrator delegate to process measurements
  using Calibrator =
      Delegate<void(const GeometryContext&, const CalibrationContext&,
                    const SourceLink&, TrackStateProxy)>;

  /// Type alias of a delegate to retrieve the measured energy loss at a given
  /// surface e.g. from accumulating the calorimeter cell energies around the
  /// trajectory
  /// @param geoContext: The geometry context to align the surface in space
  /// @param calibContext: Calibration context to access the detector conditions
  /// @param pos: The position along the trajectory (global frame)
  /// @param dir: Track direction at the current position (global frame)
  /// @param matSurf: Reference to the material surface
  using ELossAccumulator = Delegate<Gx2fMaterialProperties::ELossAtSurface(
      const GeometryContext& geoContext, const CalibrationContext& calibContext,
      const Vector3& pos, const Vector3& dir, const Surface& matSurf)>;

  /// Type alias of a delegate which inspects the track created in the current
  /// iteration. If it returns true, the fit is aborted (e.g. if all hits of the
  /// track originate from a single muon station).
  /// @param track: Trajectory holding the track states of the current iteration.
  ///               The states carry the uncalibrated source links of the measurements.
  using FitAbortChecker =  Delegate<bool(const Acts::CalibrationContext& cctx,
                                         const ConstTrackProxy track)>;

  /// Default abort checker which never aborts the fit
  static bool neverAbort(const CalibrationContext&,
                        const ConstTrackProxy /*track*/) {
    return false;
  }

  /// The elossAcumulator is the hook to use the measured energy loss instead of
  /// the estimated energy loss from the tracking geometry. The accumulator is
  /// called at every material surface. If the accumulator returns a valid
  /// ELossAtSurface object i.e. the lost energy and the associated uncertainty
  /// need to be greater than zero, this value is taken instead of the
  /// parameterized energy loss from the Highland formula
  ELossAccumulator elossAccumulator;
  /// The Calibrator is a dedicated calibration algorithm that allows
  /// to calibrate measurements using track information, this could be
  /// e.g. sagging for wires, module deformations, etc.
  Calibrator calibrator;
  /// The abort checker is called in each iteration with the hits entering the
  /// fit. If it returns true, the fit is aborted with
  /// GlobalChiSquareFitterError::FitAbortedByChecker
  FitAbortChecker abortChecker;
  /// Retrieves the associated surface from a source link
  SourceLinkSurfaceAccessor surfaceAccessor;

  /// Default constructor which connects the default void components
  Gx2FitterExtensions() {
    calibrator.template connect<&Acts::detail::voidFitterCalibrator<TrackStateContainerBackend>>();
    surfaceAccessor.connect<&Acts::detail::voidSurfaceAccessor>();
    elossAccumulator.connect<&Gx2fMaterialProperties::ELossAtSurface::invalidELoss>();
    abortChecker.template connect<&Gx2FitterExtensions::neverAbort>();
  }
};


/// Combined options for the Global-Chi-Square fitter.
///
/// @tparam traj_t The trajectory type
template <TrackContainerFrontend track_container_t>
struct Gx2FitterOptions {
   /// PropagatorOptions with context.
  ///
  /// @param gctx The geometry context for this fit
  /// @param mctx The magnetic context for this fit
  /// @param cctx The calibration context for this fit
  /// @param extensions_ The global chi2 fitter extensions
  /// @param pOptions The plain propagator options
  Gx2FitterOptions(const GeometryContext& gctx,
                   const MagneticFieldContext& mctx,
                   const CalibrationContext& cctx,
                   Gx2FitterExtensions<track_container_t> extensions_,
                   const PropagatorPlainOptions& pOptions)
      : geoContext{gctx},
        magFieldContext{mctx},
        calibrationContext{cctx},
        extensions{std::move(extensions_)},
        propagatorPlainOptions{pOptions}{}


  /// Context object for the geometry
  std::reference_wrapper<const GeometryContext> geoContext;
  /// Context object for the magnetic field
  std::reference_wrapper<const MagneticFieldContext> magFieldContext;
  /// context object for the calibration
  std::reference_wrapper<const CalibrationContext> calibrationContext;

  /// Extensions for calibration and outlier finding
  Gx2FitterExtensions<track_container_t> extensions;

  /// The trivial propagator options
  PropagatorPlainOptions propagatorPlainOptions;

  /// Whether to consider multiple scattering
  bool doMultipleScattering = false;

  /// Whether to consider energy loss
  bool doEnergyLoss = false;

  /// Flag to toggle whether material is fitted simultaneously with
  /// the track parameters or whether the material update is just
  /// a small adjustment to the track parameters
  bool includeMaterialInIter = true;

  /// Cutoff value on the scattering uncertainty to fine tune the number of
  /// scattering angles in the fit. Instead of creating scattering states at
  /// each material surface, a scatterer is only created if the uncertainy on
  /// the theta scattering angle exceeds the threshold. Otherwise the material
  /// is accumulated
  double scatteringCutOff = 0.;
  /// Cutoff value on the energy loss to fine tune the number of energy loss
  /// parameters in the fit
  double elossCutOff = 0.;

  /// Whether to include non-linear correction during global to local
  /// transformation
  FreeToBoundCorrection freeToBoundCorrection{false};

  /// Max number of iterations during the fit (abort condition)
  std::uint32_t nUpdateMax = 5;

  /// Check for convergence (abort condition). Set to 0 to skip.
  double relChi2changeCutOff = 1e-5;
  /// Convergence criterion on the reduced chi2. The fit 
  /// is covnerged if the reduced chi2 falls below the threshold
  double goodChi2{0.1};
  /// Cut on the minimal momentum. If the momentum falls below
  /// the threshold the fit is aborted
  double momentumCutOff{100._MeV};
  
  /// Abort the fit if one of its propagations reverses the direction in more
  /// than this number of consecutive steps (back and forth across a surface
  /// without progress). Set to 0 to skip.
  std::uint32_t maxDirectionFlips{10};
};

/// Result container for a global chi-square fit.
template <MutableMultiTrajectoryBackend traj_t>
struct Gx2FitterResult {
  /// Fitted states that the actor has handled.
  traj_t* fittedStates{nullptr};

  /// The materialMap stores for each visited surface their scattering
  /// properties
  std::unordered_map<GeometryIdentifier, Gx2fMaterialProperties> materialMap{};

  /// Accumulated energy loss to cache the energy loss which is below the
  /// threshold
  Gx2fMaterialProperties::ELossAtSurface accEloss{};
  /// Accumulated scattering
  Gx2fMaterialProperties::ScatteringAtSurface accScatterer{};

  /// This is the index of the 'tip' of the track stored in multitrajectory.
  /// This corresponds to the last measurement state in the multitrajectory.
  /// Since this KF only stores one trajectory, it is unambiguous.
  /// Acts::TrackTraits::kInvalid is the start of a trajectory.
  std::size_t lastMeasurementIndex = Acts::kTrackIndexInvalid;

  /// This is the index of the 'tip' of the states stored in multitrajectory.
  /// This corresponds to the last state in the multitrajectory.
  /// Since this KF only stores one trajectory, it is unambiguous.
  /// Acts::TrackTraits::kInvalid is the start of a trajectory.
  std::size_t lastTrackIndex = Acts::kTrackIndexInvalid;

  /// Counter for states with non-outlier measurements
  std::uint32_t measurementStates{0ul};

  /// Counter for measurements holes
  /// A hole correspond to a surface with an associated detector element with no
  /// associated measurement. Holes are only taken into account if they are
  /// between the first and last measurements.
  std::size_t measurementHoles{0ul};

  /// Index of the next material parameter
  std::vector<std::size_t> materialParamIndices{eBoundSize};

  /// Indicator if track fitting has been done
  bool finished = false;

  /// Measurement surfaces without hits
  std::vector<const Surface*> missedActiveSurfaces;

  /// Protection against wiggling propagators
  /// Signed path length after the previous step
  double lastPathLength = 0.;
  /// Signed length of the previous step
  double lastStepLength = 0.;
  /// Number of consecutive steps that reversed the direction
  std::uint32_t nDirectionFlips = 0;

};

/// @brief A container to manage all properties of a gx2f system
///
/// This struct manages the mathematical infrastructure for the gx2f. It
/// initializes and maintains the extended aMatrix and extended bVector.
struct Gx2fSystem {
  /// @brief Constructor to initialize matrices and vectors to zero based on specified dimensions.
  ///
  /// @param nDims Number of dimensions for the extended matrix and vector.
  explicit Gx2fSystem(std::size_t nDims) : m_nDims{nDims} {}

  /// @brief Accessor for the number of dimensions of the extended system
  /// @return Number of dimensions for the aMatrix and bVector (bound parameters + scattering angles)
  std::size_t nDims() const { return m_nDims; }

  /// @brief Accessor for the accumulated chi-squared value (const version)
  /// @return Current sum of chi-squared contributions from measurements and material
  double chi2() const { return m_chi2; }

  /// @brief Accessor for the accumulated chi-squared value (mutable version)
  /// @return Mutable reference to chi-squared sum for modification during fitting
  double& chi2() { return m_chi2; }

  /// @brief Accessor for the extended system matrix (const version)
  /// @return Const reference to the aMatrix containing measurement and material contributions
  const Eigen::MatrixXd& aMatrix() const { return m_aMatrix; }

  /// @brief Accessor for the extended system matrix (mutable version)
  /// @return Mutable reference to the aMatrix for adding measurement and material contributions
  Eigen::MatrixXd& aMatrix() { return m_aMatrix; }

  /// @brief Accessor for the extended system vector (const version)
  /// @return Const reference to the bVector containing measurement and material contributions
  const Eigen::VectorXd& bVector() const { return m_bVector; }

  /// @brief Accessor for the extended system vector (mutable version)
  /// @return Mutable reference to the bVector for adding measurement and material contributions
  Eigen::VectorXd& bVector() { return m_bVector; }

  /// @brief Accessor for the number of degrees of freedom (const version)
  /// @return Current number of degrees of freedom from processed measurements
  std::size_t ndf() const { return m_ndf; }

  /// @brief Accessor for the number of degrees of freedom (mutable version)
  /// @return Mutable reference to NDF counter for incrementing during measurement processing
  std::size_t& ndf() { return m_ndf; }

  /// @brief Determines the minimum number of degrees of freedom required for the fit
  ///
  /// Automatically deduces the required NDF based on the system configuration.
  /// We have only 3 cases, because we always have l0, l1, phi, theta:
  /// - 4: no magnetic field -> q/p is empty
  /// - 5: no time measurement -> time is not fittable
  /// - 6: full fit with all parameters
  ///
  /// @return Required NDF based on which parameters can be fitted
  std::size_t findRequiredNdf() {
    std::size_t ndfSystem = 0;
    if (m_aMatrix(4, 4) == 0) {
      ndfSystem = 4;
    } else if (m_aMatrix(5, 5) == 0) {
      ndfSystem = 5;
    } else {
      ndfSystem = 6;
    }

    return ndfSystem;
  }

  /// @brief Checks if the system has sufficient degrees of freedom for fitting
  /// @return True if NDF exceeds the minimum required for the parameter configuration
  bool isWellDefined() { return m_ndf > findRequiredNdf(); }

 private:
  /// Number of dimensions of the (extended) system
  std::size_t m_nDims;

  /// Sum of chi-squared values.
  double m_chi2 = 0.;

  /// Extended matrix for accumulation.
  Eigen::MatrixXd m_aMatrix{Eigen::MatrixXd::Zero(m_nDims, m_nDims)};

  /// Extended vector for accumulation.
  Eigen::VectorXd m_bVector{Eigen::VectorXd::Zero(m_nDims)};

  /// Number of degrees of freedom of the system
  std::size_t m_ndf = 0u;
};

/// @brief Adds a measurement to the GX2F equation system in a modular backend function.
///
/// This function processes measurement data and integrates it into the GX2F
/// system.
///
/// @param extendedSystem All parameters of the current equation system to update.
/// @param jacobianFromStart The Jacobian matrix from the start to the current state.
/// @param covarianceMeasurement The covariance matrix of the measurement.
/// @param predicted The predicted state vector based on the track state.
/// @param measurement The measurement vector.
/// @param projector The projection matrix.
/// @param logger A logger instance.
///
/// @note The dynamic Eigen matrices are suboptimal. We could think of
/// templating again in the future on kMeasDims. We currently use dynamic
/// matrices to reduce the memory during compile time.
void addMeasurementToGx2fSumsBackend(
    Gx2fSystem& extendedSystem,
    const std::vector<BoundMatrix>& jacobianFromStart,
    const std::vector<std::size_t>& materialIndices,
    const std::vector<double>& qOverPderiv,
    const Eigen::MatrixXd& covarianceMeasurement, const BoundVector& predicted,
    const Eigen::VectorXd& measurement, const Eigen::MatrixXd& projector,
    const Logger& logger);

/// @brief Process measurements and fill the aMatrix and bVector
///
/// The function processes each measurement for the GX2F Actor fitting process.
/// It extracts the information from the track state and adds it to aMatrix,
/// bVector, and chi2sum.
///
/// @tparam kMeasDim Number of dimensions of the measurement
/// @tparam track_state_t The type of the track state
///
/// @param extendedSystem All parameters of the current equation system to update
/// @param jacobianFromStart The Jacobian matrix from start to the current state
/// @param trackState The track state to analyse
/// @param logger A logger instance
template <std::size_t kMeasDim, TrackStateProxyConcept track_state_t>
void addMeasurementToGx2fSums(Gx2fSystem& extendedSystem,
                              const std::vector<BoundMatrix>& jacobianFromStart,
                              const std::vector<std::size_t>& materialIndices,
                              const std::vector<double>& qOverPderiv,
                              const track_state_t& trackState,
                              const Logger& logger) {
  const SquareMatrix<kMeasDim> covarianceMeasurement =
      trackState.template calibratedCovariance<kMeasDim>();

  const BoundVector predicted = trackState.smoothed();

  const Vector<kMeasDim> measurement =
      trackState.template calibrated<kMeasDim>();

  const Matrix<kMeasDim, eBoundSize> projector =
      trackState.template projectorSubspaceHelper<kMeasDim>().projector();

  addMeasurementToGx2fSumsBackend(
      extendedSystem, jacobianFromStart, materialIndices, qOverPderiv,
      covarianceMeasurement, predicted, measurement, projector, logger);
}

/// @brief Fill the GX2F system with data from a track
///
/// This function processes a track proxy and updates the aMatrix, bVector, and
/// chi2 values for the GX2F fitting system. It considers material only if
/// multiple scattering is enabled.
///
/// @tparam track_proxy_t The type of the track proxy
///
/// @param track A constant track proxy to inspect
/// @param extendedSystem All parameters of the current equation system
/// @param multipleScattering Flag to consider multiple scattering in the calculation
/// @param materialMap Map of geometry identifiers to scattering properties,
///        containing scattering angles and validation status
/// @param logger A logger instance
template <TrackProxyConcept track_proxy_t>
void fillGx2fSystem(
    const track_proxy_t track, Gx2fSystem& extendedSystem,
    const std::unordered_map<GeometryIdentifier, Gx2fMaterialProperties>&
        materialMap,
    const std::vector<std::size_t>& materialParamIndices,
    std::vector<GeometryIdentifier>& geoIdVector, const Logger& logger) {
  std::vector<BoundMatrix> jacobianFromStart;
  jacobianFromStart.emplace_back(BoundMatrix::Identity());
  geoIdVector.reserve(track.nTrackStates());
  std::vector<double> qOverPderiv;

  for (const auto& trackState : track.trackStates()) {
    // Get and store geoId for the current surface
    const GeometryIdentifier geoId = trackState.referenceSurface().geometryId();
    ACTS_DEBUG("Start to investigate trackState on surface " << geoId);
    const auto typeFlags = trackState.typeFlags();

    // update all Jacobians from start for every track state
    for (auto& jac : jacobianFromStart) {
      jac = trackState.jacobian() * jac;
    }

    // Skip the hole states
    if (!typeFlags.hasMeasurement() && !typeFlags.hasMaterial()) {
      ACTS_DEBUG("    Skip state.");
      continue;
    }

    // Handle measurement
    if (typeFlags.hasMeasurement()) {
      ACTS_DEBUG("    Handle measurement. ");

      const auto measDim = trackState.calibratedSize();

      if (measDim < 1 || 6 < measDim) {
        ACTS_ERROR("Can not process state with measurement with "
                   << measDim << " dimensions.");
        throw std::domain_error(
            "Found measurement with less than 1 or more than 6 dimension(s).");
      }

      extendedSystem.ndf() += measDim;

      visit_measurement(measDim, [&](auto N) {
        addMeasurementToGx2fSums<N>(extendedSystem, jacobianFromStart,
                                    materialParamIndices, qOverPderiv,
                                    trackState, logger);
      });
    }

    // Handle material
    if (typeFlags.hasMaterial()) {
      ACTS_DEBUG("    Handle material");
      const auto materialMapId = materialMap.find(geoId);
      if (materialMapId == materialMap.end()) {
        ACTS_ERROR("No material properties found for surface " << geoId);
        throw std::runtime_error("No material properties found for surface.");
      }
      /// Invalid material state
      if (materialMapId->second.nDim() == 0ul) {
        continue;
      }
      if (materialMapId->second.energyLoss().isValid()) {
        ACTS_VERBOSE("The material surface carries energy loss");
      }
      const std::size_t deltaPosition =
          materialParamIndices.at(geoIdVector.size());
      geoIdVector.push_back(geoId);

      materialMapId->second.contributionToGx2fSums(
          extendedSystem, trackState.smoothed()[eBoundTheta], deltaPosition,
          logger());

      // After an energy loss, the q/p of the state changes as
      // (q/p)' = g(q/p, E). By the chain rule, the q/p row of *every*
      // accumulated Jacobian (the one of the start parameters and the ones of
      // all previous material surfaces) needs to be scaled by d(q/p)'/d(q/p),
      // such that the sensitivity of the later measurements to an upstream
      // parameter is propagated through this loss. The Jacobian of the current
      // surface is appended afterwards and starts from the updated q/p.
      if (materialMapId->second.energyLoss().isValid()) {
        ACTS_DEBUG("    Update Jacobian for q/p after material surface "
                   << geoId << " with ratio " << materialMapId->second.qOverPratio());
        for (auto& jac : jacobianFromStart) {
          jac.row(eBoundQOverP) *= materialMapId->second.qOverPratio();
        }
      }
      // Add for this material a new Jacobian, starting from this surface.
      jacobianFromStart.emplace_back(BoundMatrix::Identity());
      qOverPderiv.push_back(materialMapId->second.qOverPderiv());
    }
  }
}

/// @brief Solve the gx2f system to get the delta parameters for the update
///
/// This function computes the delta parameters for the GX2F Actor fitting
/// process by solving the linear equation system [a] * delta = b. It uses the
/// column-pivoting Householder QR decomposition for numerical stability.
///
/// @param extendedSystem All parameters of the current equation system
/// @return Delta parameters for the GX2F update
Eigen::VectorXd computeGx2fDeltaParams(const Gx2fSystem& extendedSystem);

/// @brief Update parameters (and scattering angles if applicable)
///
/// @param params Parameters to be updated
/// @param deltaParamsExtended Delta parameters for bound parameter and scattering angles
/// @param materialMap Map of geometry identifiers to scattering properties,
///        containing all scattering angles and covariances
/// @param geoIdVector Vector of geometry identifiers corresponding to material surfaces
void updateGx2fParams(
    BoundTrackParameters& params, const Eigen::VectorXd& deltaParamsExtended,
    std::unordered_map<GeometryIdentifier, Gx2fMaterialProperties>& materialMap,
    const std::vector<GeometryIdentifier>& geoIdVector);

/// @brief Calculate and update the covariance of the fitted parameters
///
/// This function calculates the covariance of the fitted parameters using
/// cov = inv([a])
/// It then updates the first square block of size ndfSystem. This ensures,
/// that we only update the covariance for fitted parameters. (In case of
/// no qop/time fit)
///
/// @param fullCovariancePredicted The covariance matrix to update
/// @param extendedSystem All parameters of the current equation system
void updateGx2fCovarianceParams(BoundMatrix& fullCovariancePredicted,
                                Gx2fSystem& extendedSystem);

/// Global Chi Square fitter (GX2F) implementation.
///
/// @tparam propagator_t Type of the propagation class
///
/// TODO Write description
template <typename propagator_t, TrackContainerFrontend track_container_t>
class Gx2Fitter {
  /// The navigator type
  using Gx2fNavigator = typename propagator_t::Navigator;

  using TrackStateContainerBackend = typename track_container_t::TrackStateContainerBackend;


  static constexpr auto kInvalid = kTrackIndexInvalid;

 public:
  /// @brief Constructor for the Global Chi-Square Fitter
  ///
  /// Initializes the fitter with a propagator and optional logger.
  /// The fitter uses iterative fitting with a linear equation system
  /// to minimize chi-squared including multiple scattering effects.
  ///
  /// @param pPropagator The propagator instance for track propagation
  /// @param _logger Logger instance for debugging output (optional)
  explicit Gx2Fitter(const propagator_t& pPropagator,
                     std::unique_ptr<const Logger> log =
                         getDefaultLogger("Gx2Fitter", Logging::INFO))
      : m_propagator(pPropagator), m_logger{std::move(log)} {}

  /// Name of the column storing the number of iterations needed to converge 
  static constexpr std::string_view s_gx2fnUpdateColumn ="Gx2fnUpdateColumn";
  /// Name of the column storing the theta scattering angles
  static constexpr std::string_view s_gx2fThetaScatteringColumn = "Gx2fThetaScatteringColumn";
  /// Name of the column storing the phi scattering angles
  static constexpr std::string_view s_gx2fPhiScatteringColumn = "Gx2fPhiScatteringColumn";
  /// Name of the column storing the energy loss 
  static constexpr std::string_view s_gx2fEnergyLossColumn = "Gx2fEnergyLossColumn";
  private:

/// Mask for the track states. We don't need Predicted and Filtered
  static constexpr TrackStatePropMask s_trackStateMask = TrackStatePropMask::Smoothed |
                                              TrackStatePropMask::Jacobian |
                                              TrackStatePropMask::Calibrated;
  /// The propagator for the transport and material update
  propagator_t m_propagator;

  /// The logger instance
  std::unique_ptr<const Logger> m_logger;
  const Logger& logger() const { return *m_logger; }

  /// @brief Propagator Actor plugin for the GX2F
  ///
  /// @tparam parameters_t The type of parameters used for "local" parameters.
  /// @tparam calibrator_t The type of calibrator
  /// @tparam outlier_finder_t Type of the outlier finder class
  ///
  /// The GX2F Actor does not rely on the measurements to be sorted along the
  /// track.
  class GX2FActor {
   public:
    /// Broadcast the result_type
    using result_type = Gx2FitterResult<TrackStateContainerBackend>;


    /// Allows retrieving measurements for a surface
    const std::unordered_map<const Surface*, SourceLink>* inputMeasurements{};

      /// Switch to toggle whether material effects should be taken into account
    bool doMaterial{false};

    /// Pointer to the fit options steering the fit 
    const Gx2FitterOptions<track_container_t>* options{nullptr};
    /// Start parameters of the track fit. The hypothesis is needed
    /// to estimate the material effects
    const BoundTrackParameters* startParameters = nullptr;

    /// @brief Gx2f actor operation
    ///
    /// @tparam propagator_state_t is the type of Propagator state
    /// @tparam stepper_t Type of the stepper
    /// @tparam navigator_t Type of the navigator
    ///
    /// @param state is the mutable propagator state object
    /// @param stepper The stepper in use
    /// @param navigator The navigator in use
    /// @param result is the mutable result state object
    template <typename propagator_state_t, typename stepper_t,
              typename navigator_t>
    Result<void> act(propagator_state_t& state, const stepper_t& stepper,
                     const navigator_t& navigator, result_type& result,
                     const Logger& logger) const {
      assert(result.fittedStates && "No MultiTrajectory set");
      assert(options && "No options are set");

      
      
      if (stepper.absoluteMomentum(state.stepping) < options->momentumCutOff) {
      ACTS_DEBUG("Absolute momentum "
                 << stepper.absoluteMomentum(state.stepping)
                 << " below the cut-off of " << options->momentumCutOff
                 << ". Abort the propagation.");
      return Result<void>::failure(
          GlobalChiSquareFitterError::MomentumBelowCutOff);
    }
    if (options->maxDirectionFlips > 0 && state.stage == Acts::PropagatorStage::postStep) {
       const double stepLength = state.pathLength - result.lastPathLength;
      result.lastPathLength = state.pathLength;
      result.nDirectionFlips = stepLength * result.lastStepLength < 0.
                                 ? result.nDirectionFlips + 1
                                 : 0;
      result.lastStepLength = stepLength;

      if (result.nDirectionFlips > options->maxDirectionFlips) {
        ACTS_DEBUG("The last " << result.nDirectionFlips
                             << " steps reversed the direction of the "
                                "previous step. Abort the propagation.");
        return Result<void>::failure(GlobalChiSquareFitterError::DirectionFlipLimitReached);
      }
    }
      
      // Check if we can stop to propagate
      if (result.measurementStates == inputMeasurements->size()) {
        ACTS_DEBUG("Actor: finish: All measurements have been found.");
        result.finished = true;
      } else if (state.navigation.navigationBreak) {
        ACTS_DEBUG("Actor: finish: navigationBreak.");
        result.finished = true;
      }

      // End the propagation and return to the fitter
      if (result.finished) {
        // Remove the missing surfaces that occur after the last measurement
        if (result.measurementStates > 0) {
          result.missedActiveSurfaces.resize(result.measurementHoles);
        }

        return Result<void>::success();
      }

      // We are only interested in surfaces. If we are not on a surface, we
      // continue the navigation
      const Surface* surface = navigator.currentSurface(state.navigation);
      if (surface == nullptr) {
        return Result<void>::success();
      }

      const GeometryIdentifier geoId = surface->geometryId();
      ACTS_DEBUG("Surface "
                 << geoId << ", " << surface->bounds() << " detected. "
                 << "Sensitive: " << (surface->isSensitive() ? "yes" : "no")
                 << ", has material: "
                 << (surface->hasMaterial() ? "yes" : "no"));

      auto matFillResult = fillMaterialMap(state, stepper, navigator, result, logger);
      if (!matFillResult.ok()) {
        return matFillResult.error();
      }

      auto trackStateResult = makeTrackState(state, stepper, navigator, result, logger);
      if (!trackStateResult.ok()) {
        return trackStateResult.error();
      }
      // Here we handle all measurements
      if (auto sourceLinkIt = inputMeasurements->find(surface);
          sourceLinkIt != inputMeasurements->end()) {

        result.lastMeasurementIndex = result.lastTrackIndex;

        // TODO check for outlier first
        // We count the state with measurement
        ++result.measurementStates;

        // Update the number of holes count only when encountering a
        // measurement
        result.measurementHoles = result.missedActiveSurfaces.size();

        return Result<void>::success();
      }

      ACTS_DEBUG("    The surface contains no measurement/material/hole.");
      return Result<void>::success();
    }

    /// Create a track state on the current surface if it is sensitive or 
    /// carries material. If the surface has a non-zero energy loss value,
    /// the momentum is corrected accordingly
    /// @tparam propagator_state_t 
    /// @tparam stepper_t 
    /// @tparam navigator_t 
    /// @param state 
    /// @param stepper 
    /// @param navigator 
    /// @param result 
    /// @param logger 
    /// @return  
    template <typename propagator_state_t, typename stepper_t,
              typename navigator_t>
    Result<void> makeTrackState(propagator_state_t& state,
                                const stepper_t& stepper,
                                const navigator_t& navigator,
                                result_type& result,
                                const Logger& logger) const {
      const Surface* surface = navigator.currentSurface(state.navigation);

      const GeometryIdentifier geoId = surface->geometryId();

      auto sourceLinkIt = inputMeasurements->find(surface);
      auto materialItr = result.materialMap.find(geoId);
      const bool isSensitive =
          surface->isSensitive() || sourceLinkIt != inputMeasurements->end();
      const bool hasMaterial = surface->hasMaterial() &&
                               materialItr != result.materialMap.end() &&
                               materialItr->second.materialIsValid();
      /// The surface is neither sensitive or it carries material
      if (!isSensitive && !hasMaterial) {
        return Result<void>::success();
      }

      auto& fittedStates = *result.fittedStates;

      // Add a <trackStateMask> TrackState entry multi trajectory. This
      // allocates storage for all components, which we will set later.
      TrackStateProxy trackStateProxy = fittedStates.makeTrackState(
          s_trackStateMask, result.lastTrackIndex);

      result.lastTrackIndex = trackStateProxy.index();

      auto typeFlags = trackStateProxy.typeFlags();
      typeFlags.setHasParameters();

      // Transport the covariance to the surface
      auto transportRes = stepper.transportToBound(
          state.stepping, *surface, options->freeToBoundCorrection);
      if (!transportRes.ok()) {
        return transportRes.error();
      }

      // Set the trackStateProxy components with the state from the ongoing
      // propagation
      trackStateProxy.setReferenceSurface(surface->getSharedPtr());

      // Not const since, we might need to update with scattering angles
      auto res = stepper.boundParameters(state.stepping, *surface);
      if (!res.ok()) {
        return res.error();
      }
      // Not const since, we might need to update with scattering angles
      auto& boundParams = *res;

      trackStateProxy.smoothedCovariance() = stepper.covariance(state.stepping).value();
      trackStateProxy.jacobian() = *transportRes;
      trackStateProxy.pathLength() = stepper.pathLength(state.stepping);

      /// There is no material update
      if (hasMaterial) {
        typeFlags.setHasMaterial();

        // track
        ACTS_DEBUG("    Update parameters with material effects.");
        ACTS_VERBOSE("        boundParams before the update: "
                     << boundParams.parameters().transpose());

        materialItr->second.updateTrackParameters(boundParams);
#ifdef ACTIVATE_LATER
        if (options->doEnergyLoss){
          trackStateProxy.template component<double>(Acts::hashString(s_gx2fEnergyLossColumn)) = materialItr->second.lostEnergy();
        }
        if (options->doMultipleScattering){
          trackStateProxy.template component<double>(Acts::hashString(s_gx2fThetaScatteringColumn)) = materialItr->second.deltaTheta();
          trackStateProxy.template component<double>(Acts::hashString(s_gx2fPhiScatteringColumn)) = materialItr->second.deltaPhi();
        }
#endif       
        ACTS_DEBUG("        boundParams after the update: "<< boundParams.parameters().transpose());

        stepper.update(state.stepping,
                       transformBoundToFreeParameters(
                           trackStateProxy.referenceSurface(), state.geoContext,
                           boundParams.parameters()),
                       boundParams.parameters(),
                       trackStateProxy.smoothedCovariance(), *surface);
      }

      // Fill the track state
      trackStateProxy.smoothed() = boundParams.parameters();
      if (!isSensitive) {
        return Result<void>::success();
      }

      if (sourceLinkIt == inputMeasurements->end()) {
        typeFlags.setIsHole();
        return Result<void>::success();
      }
      typeFlags.setIsMeasurement();
      trackStateProxy.setUncalibratedSourceLink(SourceLink{sourceLinkIt->second});

      // We have smoothed parameters, so calibrate the uncalibrated input
      // measurement
      options->extensions.calibrator(state.geoContext, options->calibrationContext,
                                     sourceLinkIt->second, trackStateProxy);

      return Result<void>::success();
    }
    ///
    /// @tparam propagator_state_t is the type of Propagator state
    /// @tparam stepper_t Type of the stepper
    /// @tparam navigator_t Type of the navigator

    /// @param state is the mutable propagator state object
    /// @param stepper The stepper in use
    /// @param navigator The navigator in use
    /// @param result is the mutable result state object
    /// @param logger: The ACTS logging object
    template <typename propagator_state_t, typename stepper_t,
              typename navigator_t>
    Result<void> fillMaterialMap(propagator_state_t& state,
                                 const stepper_t& stepper,
                                 const navigator_t& navigator,
                                 result_type& result,
                                 const Logger& logger) const {
      if ((!options->doMultipleScattering && !options->doEnergyLoss) || !doMaterial) {
        ACTS_DEBUG(
            __func__
            << " | "
            << "Multiple scattering and energy loss are disabled. Returning");
        return Result<void>::success();
      }
      const Surface* surface = navigator.currentSurface(state.navigation);
      if (!surface->hasMaterial()) {
        return Result<void>::success();
      }

      // Found material - add a scatteringAngles entry if not done yet and
      // energy loss if enabled. Handling will happen later
      ACTS_DEBUG("    The surface contains material ...");

      const GeometryIdentifier geoId = surface->geometryId();
      auto materialMapId = result.materialMap.find(geoId);
      /// Entry is already present. Nothing to be done

      if (materialMapId != result.materialMap.end()) {
        ACTS_VERBOSE("   ....  entry is already in the material map.");
        if (materialMapId->second.nDim() == 0ul) {
          ACTS_VERBOSE(" Material surface "
                       << geoId
                       << " has zero dimension - do not keep the index.");
          return Result<void>::success();
        }
        result.materialParamIndices.push_back(
            result.materialParamIndices.back() + materialMapId->second.nDim());
        return Result<void>::success();
      }

      ACTS_DEBUG("    ... create entry in material map.");

      const Result<MaterialSlab> slabResult =
          Acts::detail::evaluateMaterialSlab(
              state, stepper, *surface,
              Acts::detail::determineMaterialUpdateMode(
                  state, navigator, MaterialUpdateMode::FullUpdate));
      if (!slabResult.ok()) {
        ACTS_DEBUG(__func__ << " | "
                            << "Failed to evaluate material slab: "
                            << slabResult.error());
        return Result<void>::failure(slabResult.error());
      }
      const MaterialSlab& slab = *slabResult;
      const bool goodSlab = !slab.isVacuum();

      using ScatteringAtSurface = Gx2fMaterialProperties::ScatteringAtSurface;
      using ELossAtSurface = Gx2fMaterialProperties::ELossAtSurface;

      ELossAtSurface eLoss{};
      ScatteringAtSurface scatterer{};

      if (goodSlab) {
        const auto& particle = startParameters->particleHypothesis();

        const double sigma =
            static_cast<double>(Acts::computeMultipleScatteringTheta0(
                slab, particle.absolutePdg(), particle.mass(),
                static_cast<float>(startParameters->parameters()[eBoundQOverP]),
                particle.absoluteCharge()));
        ACTS_VERBOSE("        The Highland formula gives sigma = " << sigma);

        scatterer = ScatteringAtSurface{sigma};

      } else {
        ACTS_VERBOSE("        Material slab is not valid.");
      }

      if (options->doEnergyLoss) {
        eLoss = options->extensions.elossAccumulator(
            state.geoContext, options->calibrationContext,
            stepper.position(state.stepping), stepper.direction(state.stepping),
            *surface);
        /// There is measurement of the energy loss but the surface material
        /// is good. Calculate the expected energy loss from Bethe-Bloch.
        if (!eLoss.isValid() && goodSlab) {
          const auto& particle = startParameters->particleHypothesis();

          const double qOverP = stepper.qOverP(state.stepping);
          ACTS_VERBOSE("The particle has momentum "
                       << (1. / qOverP) * particle.absoluteCharge()
                       << " and charge " << particle.absoluteCharge());

          eLoss = ELossAtSurface{
              computeEnergyLossMean(slab, particle.absolutePdg(),
                                    particle.mass(), qOverP,
                                    particle.absoluteCharge()),
              /// Energy loss uncertainty
              static_cast<double>(Acts::computeEnergyLossLandauSigma(
                  slab, particle.mass(), static_cast<float>(qOverP),
                  particle.absoluteCharge()))};

          ACTS_VERBOSE("        The Bethe-Bloch formula gives energy loss "
                       << eLoss.lostEnergy() << " with uncertainty "
                       << eLoss.lostSigma());
        }
      }

      /// Accumulate the material across surfaces
      scatterer += result.accScatterer;
      eLoss += result.accEloss;
      if (scatterer.sigma() < options->scatteringCutOff) {
        result.accScatterer = std::move(scatterer);
        scatterer = ScatteringAtSurface{};
      } else {
        result.accScatterer = ScatteringAtSurface{};
      }

      if (eLoss.lostEnergy() < options->elossCutOff) {
        result.accEloss = std::move(eLoss);
        eLoss = ELossAtSurface{};
      } else {
        result.accEloss = ELossAtSurface{};
      }
      ACTS_DEBUG("    ... add entry in material map with scattering "
                 << scatterer.sigma() << " and energy loss "
                 << eLoss.lostEnergy());
      materialMapId =
          result.materialMap
              .emplace(geoId, Gx2fMaterialProperties{std::move(scatterer),
                                                     std::move(eLoss)})
              .first;
      if (materialMapId->second.nDim() == 0ul) {
        ACTS_VERBOSE(" Material surface "
                     << geoId
                     << " has zero dimension - do not keep the index.");
        return Result<void>::success();
      }
      result.materialParamIndices.push_back(result.materialParamIndices.back() +
                                            materialMapId->second.nDim());

      return Result<void>::success();
    }

    template <typename propagator_state_t, typename stepper_t,
              typename navigator_t, typename result_t>
    bool checkAbort(propagator_state_t& /*state*/, const stepper_t& /*stepper*/,
                    const navigator_t& /*navigator*/, const result_t& result,
                    const Logger& /*logger*/) const {
      return result.finished;
    }
  };

 public:
  /// Fit implementation
  ///
  /// @tparam source_link_iterator_t Iterator type used to pass source links
  /// @tparam track_container_t Type of the track container backend
  /// @tparam holder_t Type defining track container backend ownership
  ///
  /// @param it Begin iterator for the fittable uncalibrated measurements
  /// @param end End iterator for the fittable uncalibrated measurements
  /// @param sParameters The initial track parameters
  /// @param gx2fOptions Gx2FitterOptions steering the fit
  /// @param trackContainer Input track container storage to append into
  /// @note The input measurements are given in the form of @c SourceLink s.
  /// It's the calibrators job to turn them into calibrated measurements used in
  /// the fit.
  ///
  /// @return the output as an output track
  template <typename source_link_iterator_t>
  Result<typename track_container_t::TrackProxy> fit(
      source_link_iterator_t it, source_link_iterator_t end,
      const BoundTrackParameters& sParameters,
      const Gx2FitterOptions<track_container_t>& gx2fOptions,
      track_container_t& trackContainer) const
    requires(!std::is_same_v<Gx2fNavigator, DirectNavigator>)
  {
    // Preprocess Measurements (SourceLinks -> map)
    // To be able to find measurements later, we put them into a map.
    // We need to copy input SourceLinks anyway, so the map can own them.
    ACTS_VERBOSE("Preparing " << std::distance(it, end)
                              << " input measurements");
    std::unordered_map<const Surface*, SourceLink> inputMeasurements{};

    for (; it != end; ++it) {
      inputMeasurements.try_emplace(gx2fOptions.extensions.surfaceAccessor(*it),
                                    *it);
    }

    using GX2FResult = typename GX2FActor::result_type;
    using Actors = Acts::ActorList<GX2FActor>;

    using PropagatorOptions = typename propagator_t::template Options<Actors>;

    BoundTrackParameters params = sParameters;
    params.covariance() = BoundMatrix::Identity();
    // params.
    double chi2sum = 0;
    double oldChi2sum = std::numeric_limits<double>::max();

    // We need to create a temporary track container. We create several times a
    // new track and delete it after updating the parameters. However, if we
    // would work on the externally provided track container, it would be
    // difficult to remove the correct track, if it contains more than one.
    TrackStateContainerBackend tempTrajectory{};
    typename track_container_t::TrackContainerBackend tempTrajBackend{};
    track_container_t trackContainerTemp{std::move(tempTrajBackend), std::move(tempTrajectory)};

#ifdef ACTIVATE_LATER
    if (gx2fOptions.doMultipleScattering) {
      trackContainerTemp.trackStateContainer().template addColumn<double>(s_gx2fThetaScatteringColumn);
      trackContainerTemp.trackStateContainer().template addColumn<double>(s_gx2fPhiScatteringColumn);
    }
    if (gx2fOptions.doEnergyLoss) {
      trackContainerTemp.trackStateContainer().template addColumn<double>(s_gx2fEnergyLossColumn);
    }
#endif
    trackContainerTemp.template addColumn<std::uint32_t>(s_gx2fnUpdateColumn);
    
    
    // Create an index of the 'tip' of the track stored in multitrajectory. It
    // is needed outside the update loop. It will be updated with each iteration
    // and used for the final track
    std::size_t tipIndex = kInvalid;

    // The materialMap stores for each visited surface their scattering
    // properties
    std::unordered_map<GeometryIdentifier, Gx2fMaterialProperties> materialMap;

    // This will be filled during the updates with the final covariance of the
    // track parameters.
    BoundMatrix fullCovariancePredicted = BoundMatrix::Identity();

    ACTS_VERBOSE("Initial parameters: " << params.parameters().transpose());

    /// Actual Fitting /////////////////////////////////////////////////////////
    ACTS_DEBUG("Start to iterate");

    // Iterate the fit and improve result. Abort after n steps or after
    // convergence.
    // nUpdate is initialized outside to save its state for the track.
    std::size_t nUpdate = 0;
    for (nUpdate = 0; nUpdate < gx2fOptions.nUpdateMax; ++nUpdate) {
      ACTS_DEBUG("nUpdate = " << nUpdate + 1 << "/" << gx2fOptions.nUpdateMax);

      // set up propagator and co
      PropagatorOptions propagatorOptions{gx2fOptions.propagatorPlainOptions};

      // Add the measurement surface as external surface to the navigator.
      // We will try to hit those surface by ignoring boundary checks.
      for (const auto& [surface, _] : inputMeasurements) {
        propagatorOptions.navigation.registerMeasurementSurface(*surface);
      }

      auto& gx2fActor = propagatorOptions.actorList.template get<GX2FActor>();
      gx2fActor.inputMeasurements = &inputMeasurements;
      gx2fActor.options= &gx2fOptions;
      gx2fActor.doMaterial = gx2fOptions.includeMaterialInIter;
      gx2fActor.startParameters = &params;

      auto propagatorState = m_propagator.makeState(propagatorOptions);

      auto propagatorInitResult =
          m_propagator.initialize(propagatorState, params, nullptr);
      if (!propagatorInitResult.ok()) {
        ACTS_DEBUG("Propagation initialization failed: "
                   << propagatorInitResult.error());
        return propagatorInitResult.error();
      }

      auto& r = propagatorState.template get<GX2FResult>();
      r.fittedStates = &trackContainerTemp.trackStateContainer();
      r.materialMap = std::move(materialMap);

      // Clear the track container. It could be more performant to update the
      // existing states, but this needs some more thinking.
      trackContainerTemp.clear();

      // Run the fitter
      auto propagationResult = m_propagator.propagate(propagatorState);

      auto result =
          m_propagator.makeResult(std::move(propagatorState), propagationResult,
                                  propagatorOptions, false);

      if (!result.ok()) {
        ACTS_DEBUG("Propagation failed: " << result.error());
        return result.error();
      }

      // TODO Improve Propagator + Actor [allocate before loop], rewrite
      // makeMeasurements
      auto& propRes = *result;
      GX2FResult gx2fResult = std::move(propRes.template get<GX2FResult>());
      /// Move back the material map
      materialMap = std::move(gx2fResult.materialMap);

      auto track = trackContainerTemp.makeTrack();
      tipIndex = gx2fResult.lastMeasurementIndex;

      // It could happen, that no measurements were found. Then the track would
      // be empty and the following operations would be invalid. Usually, this
      // only happens during the first iteration, due to bad initial parameters.
      if (tipIndex == kInvalid) {
        ACTS_INFO("Did not find any measurements in nUpdate "
                  << nUpdate + 1 << "/" << gx2fOptions.nUpdateMax);
        return GlobalChiSquareFitterError::NotEnoughMeasurements;
      }

      track.tipIndex() = tipIndex;
      track.linkForward();

      // Let the user decide whether the track of this iteration is
      // acceptable to continue the fit
      using ConstTrackProxy = typename track_container_t::ConstTrackProxy;
      if (gx2fOptions.extensions.abortChecker(gx2fOptions.calibrationContext, 
                                               ConstTrackProxy{track})) {
        ACTS_INFO("Abort checker requested to abort the fit in nUpdate "
                   << nUpdate + 1 << "/" << gx2fOptions.nUpdateMax);
        return GlobalChiSquareFitterError::FitAbortedByChecker;
      }

      // System that we fill with the information gathered by the actor and
      // evaluate later
      Gx2fSystem extendedSystem{gx2fResult.materialParamIndices.back()};

      // This vector stores the IDs for each visited material in order. We use
      // it later for updating the scattering angles. We cannot use
      // materialMap directly, since we cannot guarantee, that we will visit
      // all stored material in each propagation.
      std::vector<GeometryIdentifier> geoIdVector;

      fillGx2fSystem(track, extendedSystem, materialMap,
                     gx2fResult.materialParamIndices, geoIdVector, logger());

      chi2sum = extendedSystem.chi2();

      // This check takes into account the evaluated dimensions of the
      // measurements. To fit, we need at least NDF+1 measurements. However, we
      // count n-dimensional measurements for n measurements, reducing the
      // effective number of needed measurements. We might encounter the case,
      // where we cannot use some (parts of a) measurements, maybe if we do not
      // support that kind of measurement. This is also taken into account here.
      // We skip the check during the first iteration, since we cannot guarantee
      // to hit all/enough measurement surfaces with the initial parameter
      // guess.
      // We skip the check during the first iteration, since we cannot guarantee
      // to hit all/enough measurement surfaces with the initial parameter
      // guess.
      if ((nUpdate > 0) && !extendedSystem.isWellDefined()) {
        ACTS_INFO("Not enough measurements. Require "
                  << extendedSystem.findRequiredNdf() + 1 << ", but only "
                  << extendedSystem.ndf() << " could be used.");
        return GlobalChiSquareFitterError::NotEnoughMeasurements;
      }

      Eigen::VectorXd deltaParamsExtended =
          computeGx2fDeltaParams(extendedSystem);

      ACTS_VERBOSE("aMatrix:\n"
                   << extendedSystem.aMatrix() << "\n"
                   << "bVector:\n"
                   << extendedSystem.bVector() << "\n"
                   << "deltaParamsExtended:\n"
                   << deltaParamsExtended << "\n"
                   << "oldChi2sum = " << oldChi2sum << "\n"
                   << "chi2sum = " << extendedSystem.chi2());

      if (nUpdate > 0){
        const double redChi2 = extendedSystem.chi2() / std::max(extendedSystem.ndf(), 1ul);
        if (redChi2 < gx2fOptions.goodChi2 || 
            std::abs(extendedSystem.chi2() / oldChi2sum - 1) < gx2fOptions.relChi2changeCutOff) {
          ACTS_DEBUG("Abort with relChi2changeCutOff after "
                     << nUpdate + 1 << "/" << gx2fOptions.nUpdateMax
                     << " iterations.");
          updateGx2fCovarianceParams(fullCovariancePredicted, extendedSystem);
          break;
        }
      }

      if (extendedSystem.chi2() > oldChi2sum + 1e-5) {
        ACTS_DEBUG("chi2 not converging monotonically in update " << nUpdate);
      }

      // If this is the final iteration, update the covariance and break.
      // Otherwise, we would update the scattering angles too much.
      if (nUpdate == gx2fOptions.nUpdateMax - 1) {
        // Since currently most of our tracks converge in 4-5 updates, we want
        // to set nUpdateMax higher than that to guarantee convergence for most
        // tracks. In cases, where we set a smaller nUpdateMax, it's because we
        // want to investigate the behaviour of the fitter before it converges,
        // like in some unit-tests.
        if (gx2fOptions.nUpdateMax > 5) {
          ACTS_INFO("Did not converge in " << gx2fOptions.nUpdateMax
                                           << " updates.");
          return GlobalChiSquareFitterError::DidNotConverge;
        }

        updateGx2fCovarianceParams(fullCovariancePredicted, extendedSystem);
        break;
      }

      updateGx2fParams(params, deltaParamsExtended, materialMap, geoIdVector);
      ACTS_VERBOSE("Updated parameters: " << params.parameters().transpose());

      oldChi2sum = extendedSystem.chi2();
    }
    ACTS_DEBUG("Iterations finished");
    ACTS_VERBOSE("Final parameters: " << params.parameters().transpose());
    ACTS_VERBOSE("Final scattering angles and energy loss:");
    for (const auto& [key, value] : materialMap) {
      if (!value.materialIsValid()) {
        continue;
      }
      ACTS_VERBOSE("    ( " << value.deltaTheta() << " | " << value.deltaPhi()
                            << "|" << value.lostEnergy() << " )");
    }
    /// Finish Fitting /////////////////////////////////////////////////////////

    /// Actual MATERIAL Fitting ////////////////////////////////////////////////
    if (!gx2fOptions.includeMaterialInIter &&
        (gx2fOptions.doEnergyLoss || gx2fOptions.doMultipleScattering)) {
      ACTS_DEBUG("Start to evaluate material");

      // Set up the propagator
      PropagatorOptions propagatorOptions{gx2fOptions.propagatorPlainOptions};

      // Add the measurement surface as external surface to the navigator.
      // We will try to hit those surface by ignoring boundary checks.
      for (const auto& [surface, _] : inputMeasurements) {
        propagatorOptions.navigation.registerMeasurementSurface(*surface);
      }

      auto& gx2fActor = propagatorOptions.actorList.template get<GX2FActor>();
      gx2fActor.inputMeasurements = &inputMeasurements;
      gx2fActor.doMaterial = true;
      gx2fActor.options = &gx2fOptions;
      gx2fActor.startParameters = &params;
     

      auto propagatorState = m_propagator.makeState(propagatorOptions);

      auto propagatorInitResult =
          m_propagator.initialize(propagatorState, params, nullptr);
      if (!propagatorInitResult.ok()) {
        ACTS_DEBUG("Propagation initialization failed: "
                   << propagatorInitResult.error());
        return propagatorInitResult.error();
      }

      auto& r = propagatorState.template get<GX2FResult>();
      r.fittedStates = &trackContainerTemp.trackStateContainer();
      r.materialMap = std::move(materialMap);

      // Clear the track container. It could be more performant to update the
      // existing states, but this needs some more thinking.
      trackContainerTemp.clear();

      // Run the fitter
      auto propagationResult = m_propagator.propagate(propagatorState);

      auto result =
          m_propagator.makeResult(std::move(propagatorState), propagationResult,
                                  propagatorOptions, false);

      if (!result.ok()) {
        ACTS_DEBUG("Propagation failed: " << result.error());
        return result.error();
      }

      // TODO Improve Propagator + Actor [allocate before loop], rewrite
      // makeMeasurements
      auto& propRes = *result;
      GX2FResult gx2fResult = std::move(propRes.template get<GX2FResult>());
      materialMap = std::move(gx2fResult.materialMap);
      auto track = trackContainerTemp.makeTrack();
      tipIndex = gx2fResult.lastMeasurementIndex;

      // It could happen, that no measurements were found. Then the track would
      // be empty and the following operations would be invalid. Usually, this
      // only happens during the first iteration, due to bad initial parameters.
      if (tipIndex == kInvalid) {
        ACTS_INFO("Did not find any measurements in material fit.");
        return GlobalChiSquareFitterError::NotEnoughMeasurements;
      }

      track.tipIndex() = tipIndex;
      track.linkForward();

      // System that we fill with the information gathered by the actor and
      // evaluate later
      Gx2fSystem extendedSystem{gx2fResult.materialParamIndices.back()};

      // This vector stores the IDs for each visited material in order. We use
      // it later for updating the scattering angles. We cannot use
      // materialMap directly, since we cannot guarantee, that we will visit
      // all stored material in each propagation.
      std::vector<GeometryIdentifier> geoIdVector;

      fillGx2fSystem(track, extendedSystem, materialMap,
                     gx2fResult.materialParamIndices, geoIdVector, logger());

      chi2sum = extendedSystem.chi2();

      // This check takes into account the evaluated dimensions of the
      // measurements. To fit, we need at least NDF+1 measurements. However, we
      // count n-dimensional measurements for n measurements, reducing the
      // effective number of needed measurements. We might encounter the case,
      // where we cannot use some (parts of a) measurements, maybe if we do not
      // support that kind of measurement. This is also taken into account here.
      // We skip the check during the first iteration, since we cannot guarantee
      // to hit all/enough measurement surfaces with the initial parameter
      // guess.
      if ((nUpdate > 0) && !extendedSystem.isWellDefined()) {
        ACTS_INFO("Not enough measurements. Require "
                  << extendedSystem.findRequiredNdf() + 1 << ", but only "
                  << extendedSystem.ndf() << " could be used.");
        return GlobalChiSquareFitterError::NotEnoughMeasurements;
      }

      Eigen::VectorXd deltaParamsExtended =
          computeGx2fDeltaParams(extendedSystem);

      ACTS_VERBOSE("aMatrix:\n"
                   << extendedSystem.aMatrix() << "\n"
                   << "bVector:\n"
                   << extendedSystem.bVector() << "\n"
                   << "deltaParamsExtended:\n"
                   << deltaParamsExtended << "\n"
                   << "oldChi2sum = " << oldChi2sum << "\n"
                   << "chi2sum = " << extendedSystem.chi2());

      chi2sum = extendedSystem.chi2();

      updateGx2fParams(params, deltaParamsExtended, materialMap, geoIdVector);
      ACTS_VERBOSE("Updated parameters: " << params.parameters().transpose());

      updateGx2fCovarianceParams(fullCovariancePredicted, extendedSystem);

      ACTS_DEBUG("Finished to evaluate material");
      ACTS_VERBOSE("Final parameters after material: "
                   << params.parameters().transpose());
      /// Finish MATERIAL Fitting
      /// ////////////////////////////////////////////////

      ACTS_VERBOSE("Final scattering angles and energy loss:");
      for (const auto& [key, value] : materialMap) {
        if (!value.materialIsValid()) {
          continue;
        }
        ACTS_VERBOSE("    ( " << value.deltaTheta() << " | " << value.deltaPhi()
                              << "|" << value.lostEnergy() << " )");
      }
    }
    ACTS_VERBOSE("Final covariance:\n" << fullCovariancePredicted);

    // Prepare track for return on the temporary container
    {
      auto track = trackContainerTemp.makeTrack();
      track.tipIndex() = tipIndex;
      track.parameters() = params.parameters();
      track.covariance() = fullCovariancePredicted;
      track.setReferenceSurface(params.referenceSurface().getSharedPtr());
      ACTS_DEBUG("Add nUpdate to track");
      track.template component<std::uint32_t>(s_gx2fnUpdateColumn) = nUpdate;
      // TODO write test for calculateTrackQuantities
      calculateTrackQuantities(track);

      // Set the chi2sum for the track summary manually, since we don't calculate
      // it for each state
      track.chi2() = chi2sum;

      track.linkForward();
    }
    ACTS_DEBUG("Copy track to the output. Tracks in temporary container "
            <<trackContainerTemp.size());
    trackContainer.ensureDynamicColumns(trackContainerTemp);
    auto track  = trackContainer.makeTrack();

    track.copyFrom(trackContainerTemp.getTrack(trackContainerTemp.size() - 1u));
    
    ACTS_DEBUG("Fit completed");
    // Return the converted Track
    return track;
  }
};

/// @}

}
#endif
