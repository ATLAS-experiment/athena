/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MM_Digitization/MM_Electron.h"

MM_Electron::MM_Electron(float x, float y) : m_initialPosition(x, y) {}

void MM_Electron::setOffsetPosition(float x, float y) { m_offsetPosition = Amg::Vector2D{x, y}; }

void MM_Electron::propagateElectron(float lorentzAngle, float driftVel) {
    float tanLorentzAngle = std::tan(lorentzAngle);
    if (tanLorentzAngle == tanLorentzAngle)  // checking that it's not NAN
        m_offsetPosition = Amg::Vector2D(m_offsetPosition.x() + tanLorentzAngle * (m_offsetPosition.y() + m_initialPosition.y()),
                                         m_offsetPosition.y() + m_initialPosition.y());

    if (driftVel > 0.)
        m_time = m_offsetPosition.mag() / driftVel;
    else
        m_time = -1.;
}

void MM_Electron::setTime(float Time) { m_time = Time; }
void MM_Electron::setCharge(float Charge) { m_charge = Charge; }

const Amg::Vector2D& MM_Electron::getOffsetPosition() const { return m_offsetPosition; }
float MM_Electron::getCharge() const { return m_charge; }
float MM_Electron::getTime() const { return m_time; }
float MM_Electron::getX() const { return m_offsetPosition.x() + m_initialPosition.x(); }
float MM_Electron::getY() const { return m_offsetPosition.y() + m_initialPosition.y(); }
float MM_Electron::getInitialX() const { return m_initialPosition.x(); }
float MM_Electron::getInitialY() const { return m_initialPosition.y(); }
