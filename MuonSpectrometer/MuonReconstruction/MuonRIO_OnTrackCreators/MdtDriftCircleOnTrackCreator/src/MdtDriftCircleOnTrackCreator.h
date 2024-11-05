/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MDTDRIFTCIRCLEONTRACKCREATOR_H
#define MUON_MDTDRIFTCIRCLEONTRACKCREATOR_H

#include <bitset>
#include <memory>
#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonDriftCircleErrorStrategy.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSpaceTimePoint/SpaceTimePoint.h"

class MdtCalibToolInput;

namespace Muon {

class MdtPrepData;

/**Interface for the reconstruction to MDT calibration and alignment
corrections. It should be used by reconstruction and pattern recognition to
create Muon::MdtDriftCircleOnTrack (s).

   It offers several interfaces:
   - Create new Muon::MdtDriftCircleOnTrack from a Trk::MdtPrepData and a
predicted Trk::TrackParameter.
     @code const MdtRotPtr correct ( const Trk::PrepRawData& prd,
const Trk::TrackParameters& tp) const @endcode
   - Create new Muon::MdtDriftCircleOnTrack from a Trk::MdtPrepData and a
prediction of the global position of the point of closest approach of the track
to the wire. Knowing the second coordinate is sufficient as the side on which
the track passed the wire is not determined.
     @code createRIO_OnTrack(const MdtPrepData& prd, const Trk::GlobalPosition&
globalPos) const @endcode
   - Update the side (sign) of the  Muon::MdtDriftCircleOnTrack.
     @code updateSign(const MdtDriftCircleOnTrack& rot, const
Trk::DriftCircleSide si) const @endcode

   The tool is capable of handling serveral different timing cases (click links
to see definitions):
    - normal time of flight corrections assuming IP + light speed to be used for
simulated data and collision data
    - cosmics taken with scintilator trigger or cosmic simulation without TOF
    - cosmics taken with scintilator trigger which is read out so the offset
with respect to the clock is known. This mode is not completely functional yet
as the way to access the trigger time is not known at the moment
    - normal cosmic data with rpc trigger or simulation including TOF.
It uses the MuonCosmicTofTool to calculate the correct timing with respect to
the MuonTriggerPoint
    - testbeam running. Currently not implemented, propose not to change
anything for the moment for testbeam The modes can be selected setting the
TimingMode in the job options.

   JobOptions Flags:
   - doMDT: switch on/off ROT creation (default = true)
   - TimingMode: select timing mode (default = ATLTIME)
   - DoWireSag: Flag to turn on application of geometrical wire sagging
correstions (default = false)
   - CreateTubeHit: Flag to turn on the creation of tube hits (default = false)
*/
class MdtDriftCircleOnTrackCreator: public AthAlgTool, public IMdtDriftCircleOnTrackCreator {
   public:
    enum TimingMode {
        //!< normal time of flight corrections assuming IP + light speed
        //!< to be used for simulated data and collision data
        ATLTIME =0, 
        //!< special case for cosmics taken with scintilator trigger or
        //!< cosmic simulation without TOF
        NO_CORRECTIONS = 1,
        //!<  special case for cosmics taken with scintilator trigger
        //!<  which is read out so the offset with respect to the clock
        //!<  is known. This mode is not completely functional yet as
        //!<  the way to access the trigger time is not known at the moment
        COSMICS_TRIGGERTIME = 2,  
        //!< case for normal cosmic data with rpc trigger or simulation
        //!< including TOF. It uses the MuonCosmicTofTool to calculate
        //!< the correct timing with respect to the MuonTriggerPoint
        COSMICS_TOF =3,  
               
        NumberOfTimingModes
    };

   public:
    MdtDriftCircleOnTrackCreator(const std::string&, const std::string&, const IInterface*);
    virtual ~MdtDriftCircleOnTrackCreator() = default;
    
    virtual StatusCode initialize() override final;

    /** @brief Calibrate a MdtPrepData object. The result is stored in a new
    MdtDriftCircleOnTrack object. Included calibrations:
     - Conversion t->r using MdtCalibrationSvc
     - Wire sag + chamber deformations (if available)
     - Special treatment for cosmics if switched on
    @param prd  MdtPrepData object
    @param globalPos GlobalPosition (including second coordinate along the tube)
    @param gdir GlobalDirection of track
    @param strategy optional drift circle error strategy to override the default
    @return Fully calibrated MdtDriftCircleOnTrack (the user must delete this
    object when it is no longer needed)
    */
    virtual MdtRotPtr createRIO_OnTrack(const MdtPrepData& prd, 
                                        const Amg::Vector3D& globalPos,
                                        const Amg::Vector3D* gdir = nullptr, 
                                        const double t0Shift = 0.,
                                        const MuonDriftCircleErrorStrategy* strategy = nullptr, 
                                        const double beta = 1,
                                        const double tTrack = 1) const override final;

    /** @brief Update of the sign of the drift radius. The method creates a new
    MdtDriftCircleOnTrack, the old input MdtDriftCircleOnTrack is not deleted.
    The user should take care of the memory managment of both
    MdtDriftCircleOnTracks.
    @param rot reference to the Muon::MdtDriftCircleOnTrack of which the sign
    should be updated.
    @param si  Trk::DriftCircleSide indicating whether the muon passed on the
    left or right side of the wire.
    */
    virtual void updateSign(MdtDriftCircleOnTrack& rot,
                            const Trk::DriftCircleSide si) const override final;

    /** @brief Update error of a ROT without changing the drift radius
        @param DCT reference to the Muon::MdtDriftCircleOnTrack of which the
    sign should be updated.
        @param pars track prediction at DCT used when using the track prediction
    to update the error
    @param strategy optional drift circle error strategy to override the default
        @return New ROT with updated error. (the user must delete this object
    when it is no longer needed).
    */
    virtual MdtRotPtr updateError(const MdtDriftCircleOnTrack& DCT, 
                                  const Trk::TrackParameters* pars = nullptr,
                                  const MuonDriftCircleErrorStrategy* strategy = nullptr) const override;

    
    /** @brief Base class method for correct. */
    virtual  Trk::RIO_OnTrack* correct(const Trk::PrepRawData& prd, 
                                       const Trk::TrackParameters& tp, const EventContext& /*ctx*/) const override;

    /** @brief Returns calibrated MdtDriftCircleOnTrack.
    Implementation of IRIO_OnTrackCreator method
    @param prd Reference to a Trk::PrepRawData object (which should always be a
    Muon::MdtPrepData in this case)
    @param tp Reference to the extrapolated/predicted TrackParameters at this
    MdtPrepData
    @return calibrated MdtDriftCircleOnTrack. Memory management is passed to
    user.
    */
    virtual MdtRotPtr correct(const MdtPrepData& prd, 
                             const Trk::TrackParameters& tp,
                             const MuonDriftCircleErrorStrategy* strategy, 
                             const double beta,
                             const double tTrack) const override final;

    /** @brief Returns the default error strategy object */
    virtual const MuonDriftCircleErrorStrategy& errorStrategy() const override {
        return m_errorStrategy;
    };
    
    /** struct to hold output of calibration */
    struct CalibrationOutput {
        CalibrationOutput(Trk::LocalParameters lp,  //!< localParameters
                          Amg::MatrixX le,          //!< localErrorMatrix
                          double t,                 //!< drift time
                          bool ok)
            : locPars(lp), locErr(le), driftTime(t), calibOk(ok) {}
        Trk::LocalParameters locPars;
        Amg::MatrixX locErr;
        double driftTime;
        bool calibOk;
    };



   private:
   
    double timeOfFlight(const Amg::Vector3D& pos, 
                        const double beta,
                        const double tTrack, 
                        const double tShift) const;

    /** preform the mdt calibration */
    CalibrationOutput getLocalMeasurement(const EventContext& ctx,
                                          const MdtPrepData& DC, 
                                          const MdtCalibInput& calibInput,
                                          const MuonDriftCircleErrorStrategy& strategy) const;

    /** currently returns 0. */
    double getTriggerTime() const { return 0.; }

    // parametrised error function
    static double parametrisedSigma(double r);

    double mooreErrorStrategy(const MuonDriftCircleErrorStrategy& myStrategy,
                              double sigmaR, 
                              const Identifier& id) const;
    
    double mooreErrorStrategyMC(const MuonDriftCircleErrorStrategy& myStrategy,
                                double sigmaR, 
                                const Identifier& id) const;
    
    double mooreErrorStrategyLoose(const MuonDriftCircleErrorStrategy& myStrategy, 
                                   double sigmaR,
                                   const Identifier& id) const;
    
    double mooreErrorStrategyTight(const MuonDriftCircleErrorStrategy& myStrategy, 
                                   double sigmaR,
                                   const Identifier& id) const;

    double muonErrorStrategy(const MuonDriftCircleErrorStrategy& myStrategy,
                             double sigmaR, 
                             const Identifier& id) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ToolHandle<IMdtCalibrationTool> m_mdtCalibrationTool{this, "CalibrationTool", ""};

    // Configuration variables
    Gaudi::Property<bool> m_doMdt{this, "doMDT", true};  //!< Process MDT ROTs
    //!< Defined in TimingMode enum.
    Gaudi::Property<int> m_timeCorrectionType{this, "TimingMode", 0};
    // Constants for use during calculations
    //!< Error used when m_doFixed error =true or m_scaleErrorManually = true
    Gaudi::Property<double> m_fixedError{this, "FixedError", 1.};

    Gaudi::Property<double> m_globalToLocalTolerance{this, "GlobalToLocalTolerance", 1000.,
                                                     "Tolerance used for the Surface::globalToLocal"};

    // Member variables used to fill the default error strategy
    //!< Error strategy for created ROTs    
    MuonDriftCircleErrorStrategy m_errorStrategy{MuonDriftCircleErrorStrategyInput{}};  
   
    //!< Default error strategy for the error strategy object
    Gaudi::Property<std::string> m_defaultStrategy{this, "DefaultErrorStrategy", "Muon",
                        "Default error strategy to be used in calculating errors"};

    //!< if set to true, the ROT creator create 'tube' hits with a local
    //!< position of 0 and an error of tube radius/sqrt(12)
    Gaudi::Property<bool> m_createTubeHits{this, "CreateTubeHit", false};
    //!< Scale ROTs depending on local alignment (i.e. location in detector)
    Gaudi::Property<bool> m_scaleMdtCov{this, "DoErrorScaling", true};
    //!< Fixed error (not tube radius)
    Gaudi::Property<bool> m_doFixedError{this, "DoFixedError", true};  
    //!< Use parameterised errors
    Gaudi::Property<bool> m_useErrorParametrisation{this, "UseParametrisedError", false};
    //!< Use the predicted track position to correct the Error. See
    //!< Muon::MdtDriftCircleOnTrack::ErrorAtPredictedPosition The error will be
    //!< adjusted to be that corresponding to the predicted position. This is
    //!< useful to fix problems with tracks very close to the wire.
    Gaudi::Property<bool> m_errorAtPredictedPosition{this, "UseErrorAtPredictedPosition", false};
    //!< Add a term to the error to account for very poorly aligned stations
    Gaudi::Property<bool> m_stationError{this, "DoStationError", false};
    //!< Add a special error to account for the T0 refit
    Gaudi::Property<bool> m_t0Refit{this, "T0RefitError", false};
    //!< Use error strategy for segments by default
    Gaudi::Property<bool> m_doSegments{this, "DoSegmentErrors", true};
    //!< Deweight individual chambers
    Gaudi::Property<bool> m_doIndividualChamberReweights{this, "DeweightIndividualChambers", true};
    //!< toggle between MC and data alignment errors (to be removed in rel. 21!)
    Gaudi::Property<bool> m_isMC{this, "IsMC", false};
    //!< toggle whether the time of flight is included in the t0 shifts
    Gaudi::Property<bool> m_applyToF{this, "ApplyToF", true};
    //!< toggle between loose errors (initial before alignment) and tight after
    //!< alignment
    Gaudi::Property<bool> m_looseErrors{this, "UseLooseErrors", false,
                                        "Use error strategy for MC"};
    Gaudi::Property<bool> m_wasConfigured{this, "WasConfigured", false,
                    "This tool is too complicated to rely on defaults. Will fail if not configured."};

    static constexpr double s_inverseSpeedOfLight{1. / Gaudi::Units::c_light};

    int m_BME_idx{-1};
    
};

}  // namespace Muon

#endif
