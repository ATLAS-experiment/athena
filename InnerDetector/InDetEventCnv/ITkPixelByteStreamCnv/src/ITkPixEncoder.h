/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2024
* Description: ITkPix* encoding base class
*/

#ifndef ITKPIXENCODER_H
#define ITKPIXENCODER_H


#include "ITkPixLayout.h"
#include <vector>
#include <cstdint>

class ITkPixEncoder{
    public:
        typedef ITkPixLayout<uint16_t> HitMap;
    
        ITkPixEncoder(const unsigned nCol = 400, const unsigned nRow = 384, 
          const unsigned nColInCCol = 8, const unsigned nRowInQRow = 2, 
          const unsigned nEventsPerStream = 16, const bool plainHitMap = false, 
          const bool dropToT = false);
        
        std::vector<uint32_t>& getWords(){return m_words;}
        
        void addBits64(const uint64_t value, const uint8_t length);

        void pushWords32();

        void encodeQCore(const unsigned nCCol, const unsigned nQRow);
        
        void encodeEvent();

        void streamTag(const uint8_t nStream);

        void intTag(const uint16_t nEvt);

        void scanHitMap();

        bool hitInQCore(const unsigned CCol, const unsigned QRow);

        void setHitMap(const HitMap& hitMap){m_hitMap = hitMap;}

        void setEventsPerStream(const unsigned nEventsPerStream = 16){m_nEventsPerStream = nEventsPerStream;}
    
    protected:
        // Chip geometry
        unsigned m_nCol{400}, m_nRow{384}, m_nCCol{50}, m_nQRow{192}, m_nColInCCol{8}, m_nRowInQRow{2};

        // Output
        std::vector<uint32_t> m_words;
        unsigned m_nEventsPerStream{}, m_currCCol{}, m_currQRow{}, m_currEvent{};//, m_lastQRow;
        uint8_t m_currStream{};

        // Encoding machinery
        uint64_t m_currBlock{};
        uint8_t  m_currBit{};
        std::vector<std::vector<bool>> m_hitQCores;
        std::vector<unsigned>  m_lastQRow;

        //Globals - could be replace with compile-time conditioning instead of run-time if performance is critical
        bool m_plainHitMap{}, m_dropToT{};

        // Input
        HitMap m_hitMap;


};


#endif