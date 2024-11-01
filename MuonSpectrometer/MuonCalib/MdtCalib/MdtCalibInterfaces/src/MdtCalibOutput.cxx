/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibInterfaces/MdtCalibOutput.h>

using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;

std::ostream& operator<<(std::ostream& ostr, const MdtCalibOutput& calibResult){
    ostr<<"drift radius: "<<calibResult.driftRadius()<<" pm "<<calibResult.driftRadiusUncert()<<", ";
    ostr<<"drift time: "<<calibResult.driftTime()<<", ";
    ostr<<"t0 "<<calibResult.tubeT0()<<", ";
    ostr<<"slewing time: "<<calibResult.slewingTime()<<", ";
    ostr<<"lorentz time: "<<calibResult.lorentzTime()<<", ";
    ostr<<"propagation time: "<<calibResult.signalPropagationTime()<<", ";
    ostr<<"calib status: "<<calibResult.status();
    return ostr;
}

double MdtCalibOutput::driftRadius() const { return m_driftR; }
double MdtCalibOutput::driftRadiusUncert() const { return m_driftUncert; }

void MdtCalibOutput::setDriftRadius(const double radius,
                                    const double uncert){
    m_driftR = radius;
    m_driftUncert = uncert;
}

double MdtCalibOutput::driftTime() const { return m_driftTime; }
void MdtCalibOutput::setDriftTime(const double driftTime) { m_driftTime = driftTime; }

double MdtCalibOutput::slewingTime() const { return m_slewingTime ;}
void MdtCalibOutput::setSlewingTime(const double slewTime) { m_slewingTime = slewTime; }

double MdtCalibOutput::lorentzTime() const { return m_lorentzTime; }
void MdtCalibOutput::setLorentzTime(const double time) { m_lorentzTime = time; }

double MdtCalibOutput::temperatureTime() const { return m_tempTime; }
void MdtCalibOutput::setTemperatureTime(const double tempTime) { m_tempTime = tempTime; }

MdtDriftCircleStatus MdtCalibOutput::status() const { return m_status; }
void MdtCalibOutput::setStatus(const MdtDriftCircleStatus stat) { m_status = stat; }

double MdtCalibOutput::tubeT0() const { return m_tubeT0; }
void MdtCalibOutput::setTubeT0(const double T0) { m_tubeT0 = T0; }

void MdtCalibOutput::setPropagationTime(const double propTime) { m_sigPropTime = propTime; }
double MdtCalibOutput::signalPropagationTime() const { return m_sigPropTime; }

void MdtCalibOutput::setMeanAdc(const double adc) { m_meanAdc = adc ;}
double MdtCalibOutput::meanAdc() const { return m_meanAdc; }

double MdtCalibOutput::backgroundTime() const{ return m_bkgTime; }
void MdtCalibOutput::setBackgroundTime(const double bkgTime) { m_bkgTime = bkgTime ;}

void MdtCalibOutput::setDriftUncertSigProp(const double uncert) { m_radUncertFromProp  = uncert; }
double MdtCalibOutput::driftUncertSigProp() const { return m_radUncertFromProp; }
