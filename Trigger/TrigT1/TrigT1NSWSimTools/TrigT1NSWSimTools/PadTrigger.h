//  -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NSWL1_PADDATA_H
#define NSWL1_PADDATA_H

#include "TrigT1NSWSimTools/TriggerTypes.h"
#include "TrigT1NSWSimTools/GeoUtils.h"
#include <ostream>
#include <vector>


namespace NSWL1 {
    class PadData;
    class SectorTriggerCandidate;
    /**
    @brief Holder of the pad trigger information
    
    Stores info about the pads that were used to compute this trigger,
    as well as the resulting trigger primitive.

    */
    class PadTrigger {//S.I make this a proper class // needs a wrap-up
    public:
        uint16_t m_bandid{0}; ///< see bandId below
        float m_eta{0};//eta of the centroid of the pad tower
        float m_phi{0}; // phi of the centroid of the pad tower
        float m_etamin{0};//bounding rectangles in eta-phi for the pad tower
        float m_etamax{0};
        float m_phimin{0};
        float m_phimax{0};
        int   m_eta_id{0};//meaningless
        int   m_phi_id{0};
        int   m_multiplet_id{0};//meaningless as theres no single wedge triggers
        int   m_isSmall{0};
        int m_moduleIdInner{0};
        int m_moduleIdOuter{0};
        std::vector<float> m_trglocalminYInner;
        std::vector<float> m_trglocalmaxYInner;
        std::vector<float> m_trglocalminYOuter;
        std::vector<float> m_trglocalmaxYOuter;
        std::vector<int> m_trgSelectedLayersInner;
        std::vector<int> m_trgSelectedLayersOuter;
        std::vector<int> m_trgSelectedBandsInner;
        std::vector<int> m_trgSelectedBandsOuter;
        std::vector<int> m_trgPadPhiIndicesInner;
        std::vector<int> m_trgPadPhiIndicesOuter;
        std::vector<int> m_trgPadEtaIndicesInner;
        std::vector<int> m_trgPadEtaIndicesOuter;        
        
        std::vector< std::shared_ptr<PadData>> m_pads; ///!< digits contributing to this trigger primitive
        std::vector< std::shared_ptr<PadData>> m_padsInner; ///!< digits contributing to this trigger primitive
        std::vector< std::shared_ptr<PadData>> m_padsOuter; ///!< digits contributing to this trigger primitive
        int m_triggerindex{0};
        PadTrigger() = default;

        std::shared_ptr<PadData> firstPad() const;
        std::shared_ptr<PadData> firstPadInner() const;
        std::shared_ptr<PadData> firstPadOuter() const;
        uint16_t bctag()     const;
        int index() const { return m_triggerindex;}
        int sideId()        const; ///! sideId of the first pad in the trigger
        int sectorId()      const; ///! sectorId of the first pad in the trigger
        int triggerSectorNumber() const;
        uint16_t bandId()   const { return m_bandid; }
        float eta()         const { return m_eta;    } ///! SectorTriggerCandidate eta center
        float phi()         const { return m_phi;    } ///! SectorTriggerCandidate phi center 
        float etaId()       const { return m_eta_id; } ///! SectorTriggerCandidate pad eta index 
        uint16_t phiId()       const { return m_phi_id; } ///! SectorTriggerCandidate pad phi index
        float multipletId() const { return m_multiplet_id; } ///! SectorTriggerCandidate pad multiplet index 
        //=== boundaries of triggger band in eta
        float etaMin()      const {return m_etamin;}
        float etaMax()      const {return m_etamax;}
        //========
        float phiMin()      const {return m_phimin;}
        float phiMax()      const {return m_phimax;}
        int isSmall ()      const {return m_isSmall;}
        int moduleIdInner() const { return m_moduleIdInner;}
        int moduleIdOuter() const { return m_moduleIdOuter;}

        const std::vector<float>& trglocalminYInner() const { return m_trglocalminYInner; }
        const std::vector<float>& trglocalmaxYInner() const { return m_trglocalmaxYInner; }
        const std::vector<float>& trglocalminYOuter() const { return m_trglocalminYOuter; }
        const std::vector<float>& trglocalmaxYOuter() const { return m_trglocalmaxYOuter; }
        const std::vector<int>& trgSelectedLayersInner()const{ return m_trgSelectedLayersInner;}
        const std::vector<int>& trgSelectedLayersOuter() const { return m_trgSelectedLayersOuter; }
        const std::vector<int>& trgSelectedBandsInner() const { return m_trgSelectedBandsInner;}
        const std::vector<int>& trgSelectedBandsOuter() const { return m_trgSelectedBandsOuter;}
        const std::vector<int>& trgPadPhiIndicesInner() const { return m_trgPadPhiIndicesInner;}
        const std::vector<int>& trgPadPhiIndicesOuter() const { return  m_trgPadPhiIndicesOuter;}
        const std::vector<int>& trgPadEtaIndicesInner() const { return m_trgPadEtaIndicesInner;}
        const std::vector<int>& trgPadEtaIndicesOuter() const { return m_trgPadEtaIndicesOuter;}         
        
        
    };

} // namespace NSWL1

/// \todo
// std::ostream& operator<<(std::ostream& stream, NSWL1::PadTrigger& pt);

#endif
