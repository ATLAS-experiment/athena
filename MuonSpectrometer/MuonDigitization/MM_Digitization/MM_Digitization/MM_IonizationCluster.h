/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_DIGITIZATION_MM_IONIZATIONCLUSTER_H
#define MM_DIGITIZATION_MM_IONIZATIONCLUSTER_H
//
// MM_IonizationCluster.cxx
//     Primary Ionization Cluster made up of primary electrons
//

#include <memory>
#include <vector>
#include "GeoPrimitives/GeoPrimitives.h"
#include "MM_Digitization/MM_Electron.h"

class MM_IonizationCluster {
public:
    MM_IonizationCluster() = default;
    MM_IonizationCluster(float HitX, float IonizationX, float IonizationY);
    void createElectrons(int nElectrons);
    void propagateElectrons(float lorentzAngle, float driftVel);
    std::vector<std::unique_ptr<MM_Electron>>& getElectrons();
    float getHitX() const { return m_HitX; }
    const Amg::Vector2D& getIonizationStart() const { return m_IonizationStart; }

private:
    // Members supplied by user
    std::vector<std::unique_ptr<MM_Electron>> m_Electrons;
    float m_HitX{0.0F};
    Amg::Vector2D m_IonizationStart{Amg::Vector2D::Zero()};
};

#endif
