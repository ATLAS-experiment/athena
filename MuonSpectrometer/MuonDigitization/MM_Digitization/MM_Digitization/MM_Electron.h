/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_DIGITIZATION_MM_ELECTRON_H
#define MM_DIGITIZATION_MM_ELECTRON_H
//
// MM_Electron.cxx
//     Primary electron objects that are diffused, propagated and avalanched
//

#include <GeoPrimitives/GeoPrimitives.h>

#include <cfloat>
#include <memory>


class MM_Electron {
public:
    MM_Electron() = default;
    MM_Electron(float x, float y);

    void setOffsetPosition(float x, float y);
    void propagateElectron(float lorentzAngle, float driftVel);
    void setTime(float Time);
    void setCharge(float Charge);

    const Amg::Vector2D& getOffsetPosition() const;
    float getCharge() const;
    float getTime() const;
    float getX() const;
    float getY() const;
    float getInitialX() const;
    float getInitialY() const;

private:
    Amg::Vector2D m_initialPosition{Amg::Vector2D::Zero()};
    Amg::Vector2D m_offsetPosition{Amg::Vector2D::Zero()};

    float m_time{-FLT_MAX};
    float m_charge{-FLT_MAX};
};

#endif
