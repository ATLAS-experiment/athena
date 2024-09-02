/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBSVC_IMDTCALIBRATIONTOOL_H
#define MDTCALIBSVC_IMDTCALIBRATIONTOOL_H

#include <bitset>
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>
#include <MdtCalibInterfaces/MdtCalibInput.h>
#include <MdtCalibInterfaces/MdtCalibOutput.h>
#include <MdtCalibInterfaces/MdtCalibTwinOutput.h>


/**
   @class IMdtCalibrationTool
   the Mdt Calib Service provides, on request, the drift radius and its 
   error, computed applying the proper calibration, for any hit in Mdt chambers  
   @author Martin Woudstra, Niels van Eldik
*/

class IMdtCalibrationTool : virtual public IAlgTool {
public:

  enum timeWindowMode {
      UserDefined = 0,    /// User can configure the window
      Default = 1,        /// 1000, 2000
      CollisionG4 = 2,    /// 20, 30
      CollisionData = 3,  /// 10, 30
      CollisionFitT0 = 4, /// 50, 100
  };
  /// Conversion to go from tdc counts -> drift Time
  static constexpr double tdcBinSize = 0.78125;  //25/32; exact number: (1000.0/40.079)/32.0

  /** destructor */
  virtual ~IMdtCalibrationTool() = default;

  /** implements IInterface */
  DeclareInterfaceID(IMdtCalibrationTool, 1, 0);
 
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
                                   bool resolFromRtrack=false) const = 0;

  /** Convert the raw MDT times of two twin hits into a Twin position (coordinate along tube)
      It returns whether the conversion was successful. */
  virtual MdtCalibTwinOutput calibrateTwinTubes(const EventContext& ctx,
                                                MdtCalibInput&& primHit, 
                                                MdtCalibInput&& twinHit) const = 0;
  
  virtual double getResolutionFromRt(const EventContext& ctx,
                                     const Identifier& module,
                                     const double time) const  = 0;

  virtual double getdRdtFromRt(const EventContext& ctx,
                               const Identifier& module,
                               const double time) const  = 0;
  struct ToolSettings {
      enum class Property {
        TofCorrection = 0,
        PropCorrection,
        TempCorrection,
        MagFieldCorrection,
        WireSagTimeCorrection,
        SlewCorrection,
        BackgroundCorrection,
        NumSettings
      };
   
      void setBit(const Property prop, const bool value){
         m_mask.set(static_cast<unsigned int>(prop), value);
      }
      bool isActive(const Property prop) const {
        return m_mask.test(static_cast<unsigned int>(prop));
      }
      timeWindowMode window{timeWindowMode::UserDefined};
    private:
      using bitmask = std::bitset<static_cast<unsigned>(Property::NumSettings)>;
      bitmask m_mask{0};

  };
  virtual ToolSettings getSettings() const = 0;

};

#endif