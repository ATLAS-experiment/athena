/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*EXTERNAL CODE PORTED FROM YARR MINIMALLY ADAPTED FOR ATHENA*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2024
* Description: ITkPix* encoding base class
*/

#ifndef ITKPIXENCODER_H
#define ITKPIXENCODER_H


#include "ITkPixLayout.h"
#include "CxxUtils/checker_macros.h"
#include <vector>
#include <cstdint>
#include <mutex>

class ITkPixEncoder{
    public:
        typedef ITkPixLayout<uint16_t> HitMap;
    
        ITkPixEncoder(const unsigned nCol = 400, const unsigned nRow = 384, 
          const unsigned nColInCCol = 8, const unsigned nRowInQRow = 2, 
          const unsigned nEventsPerStream = 16, const bool plainHitMap = false, 
          const bool dropToT = false);
        
        std::vector<uint32_t>& getWords(){return m_words;}
        
        void addBits64(const uint64_t value, const uint8_t length) const;

        void setHitMap(const HitMap& hitMap) const {m_hitMap = hitMap;}

        void setEventsPerStream(const unsigned nEventsPerStream = 16){m_nEventsPerStream = nEventsPerStream;}

        void clear() const;
    
    protected:

        void pushWords32() const;

        void encodeQCore(const unsigned nCCol, const unsigned nQRow) const;
        
        void encodeEvent() const;

        void streamTag(const uint8_t nStream) const;

        void intTag(const uint16_t nEvt) const;

        void scanHitMap() const;

        bool hitInQCore(const unsigned CCol, const unsigned QRow) const;

        //Access to all mutables is always mutex-protected
        // Chip geometry
        unsigned m_nCol{400}, m_nRow{384}, m_nCCol{50}, m_nQRow{192}, m_nColInCCol{8}, m_nRowInQRow{2};

        // Output
        mutable std::vector<uint32_t> m_words ATLAS_THREAD_SAFE; //only accessed in mutex-protected function
        unsigned m_nEventsPerStream;
        mutable unsigned m_currCCol ATLAS_THREAD_SAFE {}, m_currQRow ATLAS_THREAD_SAFE {}, m_currEvent ATLAS_THREAD_SAFE {};//
        mutable uint8_t m_currStream ATLAS_THREAD_SAFE {};

        // Encoding machinery
        mutable uint64_t m_currBlock ATLAS_THREAD_SAFE {};
        mutable uint8_t  m_currBit ATLAS_THREAD_SAFE {};
        mutable std::vector<std::vector<bool>> m_hitQCores ATLAS_THREAD_SAFE; //only accessed in mutex-protected function
        mutable std::vector<unsigned>  m_lastQRow ATLAS_THREAD_SAFE; //only accessed in mutex-protected function

        //Globals - could be replace with compile-time conditioning instead of run-time if performance is critical
        bool m_plainHitMap{}, m_dropToT{};

        // Input
        mutable HitMap m_hitMap ATLAS_THREAD_SAFE; //only accessed in mutex-protected function

        mutable std::mutex m_mutex;


};


#endif