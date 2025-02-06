/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBSVC_MDTCALIBRATIONTOOL_H
#define MDTCALIBSVC_MDTCALIBRATIONTOOL_H


#include "MdtCalibInterfaces/IMdtCalibrationTool.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MdtCalibInterfaces/IShiftMapTools.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"
class MdtCalibHit;
class MdtCalibToolInput;

namespace MuonCalib {
  class MdtRtRelation;
}

/**
   @class MdtCalibrationTool
   the Mdt Calib Service provides, on request, the drift radius and its 
   error, computed applying the proper calibration, for any hit in Mdt chambers  
   @author Martin Woudstra, Niels van Eldik
*/



class MdtCalibrationTool : public extends<AthAlgTool, IMdtCalibrationTool> {
public:

  
  using CorrectionPtr =  MuonCalib::MdtFullCalibData::CorrectionPtr;
  using RtRelationPtr =  MuonCalib::MdtFullCalibData::RtRelationPtr;
  using TubeContainerPtr = MuonCalib::MdtFullCalibData::TubeContainerPtr;


  /** constructor */
  using base_class::base_class;
  /** destructor */
  virtual ~MdtCalibrationTool() = default;

  /** initialization */
  virtual StatusCode initialize() override final;

  /** Convert the raw MDT time (+charge) into a drift radius + error.
      It returns whether the conversion was successful.
       
      @param[in,out] hit Hit must have pointer set to the MdtDigit,
      as well as the global hit position (including the position along the tube!)
      @param[in] signedTracklength the track length from the 'triggerpoint' to the hit.
      It is used for the time-of-flight correction.
      This triggerpoint is the I.P. for ATLAS p-p collisions,
      typically scintillators in test-beam and cosmic teststands,
      and not clear yet what is it is for cosmics in ATLAS.
      The sign is for determining the sign of the time-of-flight correction.
      If a muon first passes the triggerpoint, and then the MDT tube, the sign
      should be positive (the case for ATLAS p-p and H8 test-beam).
      If a muon first passes the MDT tube, and then de triggerpoint, the sign
      should be negative (typically the case for cosmic-ray teststands).
      @param[in] triggerTime the time of the 'triggerpoint' in ns. This is the time (measured with
      the same clock as the MDT TDC's) when the muon passed a known point in
      space: the 'triggerpoint'. 
      For ATLAS this is 0.0 since the TDC's are synchonised w.r.t. the bunch-crossings.
      For H8 test-beam it is the trigger time, which is time when the muon passed
      the trigger scintillators. For cosmic-ray teststands it is usually also the
      time when the muon passed the trigger scintillators.
      For cosmics in ATLAS it is not clear yet.
      @param[in] resolFromRtrack indicates the method to provide the resolution as a function 
      of the distance of the reconstructed track from the anode wire instead of the drift
      radius
  */
  virtual MdtCalibOutput calibrate(const EventContext& ctx, 
                                   const MdtCalibInput& hit,
                                   bool resolFromRtrack=false) const override final;

  /** Convert the raw MDT times of two twin hits into a Twin position (coordinate along tube)
      It returns whether the conversion was successful. */
  virtual MdtCalibTwinOutput calibrateTwinTubes(const EventContext& ctx,
                                                MdtCalibInput&& hit, 
                                                MdtCalibInput&& twinHit) const override final;

  virtual double getResolutionFromRt(const EventContext& ctx,
                                     const Identifier& module,
                                     const double time) const override final;

  virtual ToolSettings getSettings() const override final;

  virtual const MuonCalib::MdtFullCalibData* 
          getCalibConstants(const EventContext& ctx,
                            const Identifier& channelId) const override final;
private:
  Muon::MdtDriftCircleStatus driftTimeStatus(double driftTime, 
                                             const MuonCalib::MdtRtRelation& rtRelation) const;
  
  Gaudi::Property<int> m_windowSetting{this, "TimeWindowSetting", timeWindowMode::Default};
  Gaudi::Property<double> m_timeWindowLowerBound{this, "TimeWindowLowerBound", 0.};
  Gaudi::Property<double> m_timeWindowUpperBound{this, "TimeWindowUpperBound", 0.}; 
  Gaudi::Property<bool> m_doTof{this, "DoTofCorrection", true};
  Gaudi::Property<bool> m_doProp{this, "DoPropagationCorrection",  true};
  Gaudi::Property<bool> m_doTemp{this, "DoTemperatureCorrection", false};
  Gaudi::Property<bool> m_doField{this,"DoMagneticFieldCorrection", false};
  Gaudi::Property<bool> m_doSlew{this, "DoSlewingCorrection", false};
  Gaudi::Property<bool> m_doBkg{this, "DoBackgroundCorrection", false};
  Gaudi::Property<bool> m_doPropUncert{this, "DoPropagationTimeUncert", false};
  /* T0 Shift tool -- Per-tube offsets of t0 value */
  ToolHandle<MuonCalib::IShiftMapTools> m_t0ShiftTool{this, "T0ShiftTool", ""};
  /* TMax Shift tool -- Per-tube offsets of Tmax */
  ToolHandle<MuonCalib::IShiftMapTools> m_tMaxShiftTool{this, "TShiftMaxTool", ""};

  // tools should only be retrieved if they are used
  Gaudi::Property<bool> m_doT0Shift{this, "DoT0Shift", false};
  Gaudi::Property<bool> m_doTMaxShift{this, "DoTMaxShift", false};

  Gaudi::Property<double> m_unphysicalHitRadiusUpperBound{this, "UpperBoundHitRadius", 20.};
  Gaudi::Property<double> m_unphysicalHitRadiusLowerBound{this, "LowerBoundHitRadius" , 0.};
  Gaudi::Property<double> m_resTwin{this, "ResolutionTwinTube" , 1.05, "Twin tube resolution"};


  SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_calibDbKey{this, "CalibDataKey", "MdtCalibConstants",
                                                                       "Conditions object containing the calibrations"};

  // Read handle for conditions object to get the field cache
  // If one wants to avoid that adding of this read handle here, then client tools/algs calling driftRadiusFromTime
  // must implement this to get the AtlasFieldCache which can then be passed through the call to driftRadiusFromTime
  // Note: a readhandle must be in a tool or an alg, and so it cannot be in the class Imp.)
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", 
                                                                             "Name of the Magnetic Field conditions object key"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};

#endif
