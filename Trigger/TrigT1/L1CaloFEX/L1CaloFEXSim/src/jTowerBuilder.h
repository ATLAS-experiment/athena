/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef JTOWERBUILDER_H
#define JTOWERBUILDER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjTowerBuilder.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXCond/jFEXDBCondData.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace LVL1 {

class jTowerBuilder: public AthAlgTool, virtual public IjTowerBuilder {

    public:
        jTowerBuilder(const std::string& type,const std::string& name,const IInterface* parent);
        virtual ~jTowerBuilder() = default;
        virtual StatusCode initialize() override;

        virtual void init(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const override ;
        virtual void execute(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const override ;
        virtual void reset() const override ;
        virtual StatusCode AssignPileupAndNoiseValues (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const override;



    private:
        void BuildEMBjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildTRANSjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildEMEjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildEMIEjTowers (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildFCALjTowers (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildHECjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildAllTowers   (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildSingleTower (std::unique_ptr<jTowerContainer> & jTowerContainerRaw,float eta, float phi, int key_eta, float keybase, int posneg, float centre_eta = 0.0, float centre_phi = 0.0, int fcal_layer = -1) const;

        static constexpr float m_TT_Size_phi = M_PI/32;
        static constexpr float m_TT_Size_phi_FCAL = M_PI/16;
        
        SG::ReadCondHandleKey<jFEXDBCondData> m_BDToolKey {this, "BDToolKey", "jFEXDBParams", "DB tool key"};


};

} // end of LVL1 namespace
#endif
