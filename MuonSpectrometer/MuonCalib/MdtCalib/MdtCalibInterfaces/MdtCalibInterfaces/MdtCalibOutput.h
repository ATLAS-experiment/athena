/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDTCALIBINTEFACES_MDTCALIBOUTPUT_H
#define MDTCALIBINTEFACES_MDTCALIBOUTPUT_H

#include <MuonPrepRawData/MdtDriftCircleStatus.h>
#include <iostream>

class MdtCalibOutput {
public:
    MdtCalibOutput() = default;
    /// Returns the drift radius of the calibrated object
    double driftRadius() const;
    /// Returns the uncertainty on the drift radius
    double driftRadiusUncert() const;
    /// Sets the charge drift radius and its associated uncertainty
    void setDriftRadius(const double radius,
                        const double uncert);
    /// Returns the drift time inside the tube
    double driftTime() const;
    /// Sets the drift time 
    void setDriftTime(const double driftTime);

    /// Returns the point in time where the muon typically enters the chamber
    double tubeT0() const;
    /// Sets the tube T0
    void setTubeT0(const double T0);
    
    /// Sets the signal propagation time in the tube wire
    void setPropagationTime(const double T0);
    /// Returns the signal propagation time
    double signalPropagationTime() const;


    /// Returns the slewing time (Needed time that the accumulated charge 
    /// passes the electronic threshold to trigger the tdc counter)
    double slewingTime() const;
    /// Sets the slewing time
    void setSlewingTime(const double slewTime);
    /// Returns the time corrections from the signal propgation inside a magnetic field
    double lorentzTime() const;
    /// Sets the Lorentz time
    void setLorentzTime(const double time);
    /// Returns the time corrections stemming from temperature & pressure corrections
    double temperatureTime() const;
    /// Sets the temperature time correction
    void setTemperatureTime(const double tempTime);
    /// Sets the mean tube adc
    void setMeanAdc(const double adc);
    /// Returns the mean tube adc
    double meanAdc() const;
    /// Return the time correction arising from background processes
    double backgroundTime() const;
    /// Sets the background time correction
    void setBackgroundTime(const double bkgTime);

    // Returns the time correction arising from a wire sagging in the tube
    double saggingTime() const;
    /// Sets the sagging time
    void setSaggingTime(const double sagTime);

    using  MdtDriftCircleStatus = Muon::MdtDriftCircleStatus;    
    /// Status of the calibration
    MdtDriftCircleStatus status() const;
    void setStatus(const MdtDriftCircleStatus stat);
private:
    double m_driftR{0.};
    double m_dRdt{0.};
    double m_driftUncert{0.};
    double m_tubeT0{0.};
    double m_sigPropTime{0.};
    double m_driftTime{0.};
    double m_slewingTime{0.};
    double m_lorentzTime{0.};
    double m_tempTime{0.};
    double m_meanAdc{0.};
    double m_bkgTime{0.};
    double m_sagTime{0.};
    MdtDriftCircleStatus m_status{MdtDriftCircleStatus::MdtStatusUnDefined};
};

std::ostream& operator<<(std::ostream& ostr, const MdtCalibOutput& calibResult);
#endif

