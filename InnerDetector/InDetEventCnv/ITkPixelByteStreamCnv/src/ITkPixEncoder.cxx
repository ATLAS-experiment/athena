/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*EXTERNAL CODE PORTED FROM YARR MINIMALLY ADAPTED FOR ATHENA*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2024
* Description: ITkPix* encoding base class
*/

#include "ITkPixEncoder.h"
#include "ITkPixQCoreEncodingLUT.h"
#include <bitset>
#include <limits>

//Constructor sets up the geometry for all future loops

ITkPixEncoder::ITkPixEncoder(const bool enableChipID, const unsigned nCol, const unsigned nRow, const unsigned nColInCCol, 
  const unsigned nRowInQRow, const unsigned nEventsPerStream, const bool plainHitMap, 
  const bool dropToT): m_nCol(nCol/nColInCCol), m_nRow(nRow/nRowInQRow), 
  m_nColInCCol(nColInCCol), m_nRowInQRow(nRowInQRow), m_nEventsPerStream(nEventsPerStream), m_enableChipID(enableChipID),
  m_plainHitMap(plainHitMap), m_dropToT(dropToT){
    if (m_enableChipID) m_bitsPerWord = 61;
    else m_bitsPerWord = 63;
}

void ITkPixEncoder::addBits64(const uint64_t value, const uint8_t length) const {

    //only called from mutex-protected function

    //This adds 'length' lowest bits to the current block. If the current
    //block gets filled, push it to the output and start a new block with
    //the rest of the bits that didn't make it. Need to keep track of the
    //remaining space in the current word. Also, we only have 63 bits for
    //the added data, as the first bit is EoS.
    //Case 1: there's enough space for the entire information to be added
    if (length <= (m_bitsPerWord - m_currBit)){
        
        //The position at which the new bits should be inserted into the block
        //is (length of the block - 1) - (currently last bit) - (length)
        //We need to keep the first bit for EoS, hence the -1
        m_currBlock |= (value << (m_bitsPerWord - m_currBit - length));
        m_currBit += length;
        return;
    }

    //Case 2: there's not enough space, so we put what we can, push the block
    //to the output, and the rest to a new block
    else {

        //How much space do we have?
        uint8_t remainingBits = m_bitsPerWord - m_currBit;

        //Add that many bits
        m_currBlock |= ((value >> (length - remainingBits)));

        //Push the current block to the output and reset it and the current
        //bit counter. The block needs to be split in 32-bit halves before the output
        pushWords32();

        //What are we left with?
        uint8_t leftoverBits = length - remainingBits;

        //Now we can add the remainder
        //Make sure that the remainder we're adding is really only the bits we've not yet
        //added, otherwise there can be a rogue "1" in the EoS bit, which was already added
        //in the previous block
        addBits64((value & (0xFFFFFFFFFFFFFFFF >> (64 - leftoverBits))), leftoverBits);
        
    }
}

void ITkPixEncoder::pushWords32() const{
    //whenever the current block is ready for output,
    //split it into two 32-bit words and push them to
    //the output container. Reset the current bloc/bit
    
    //only called from mutex-protected function
    if (m_enableChipID){
        m_currBlock |= ((0x0ULL | m_chipID) << 61);
    }
    uint32_t word1 = m_currBlock >> 32;
    uint32_t word2 = m_currBlock & 0xFFFFFFFF;
    m_words.push_back(word1);
    m_words.push_back(word2);
    m_currBlock = 0x0ULL;
    m_currBit   = 0;
}

void ITkPixEncoder::encodeQCore(const unsigned nCCol, const unsigned nQRow) const {

    //only called from mutex-protected function

    //produce hit map and ToTs
    //First, get the top-left pixel in the QCore
    unsigned col = nCCol * m_nColInCCol;
    unsigned row = nQRow * m_nRowInQRow;

    //now loop, store ToTs, and build index of the
    //compressed hit map in the LUT
    uint16_t lutIndex = 0x0000;
    std::vector<uint16_t> tots;
    tots.reserve(16);
    int pix = 0;
    for (unsigned pixRow = row; pixRow < row + m_nRowInQRow; pixRow++){
        for (unsigned pixCol = col; pixCol < col + m_nColInCCol; pixCol++){
            if (m_hitMap(pixCol, pixRow)){
                lutIndex |= 0x1 << pix;
                tots.push_back(m_hitMap(pixCol, pixRow) - 1);
            }
            pix++;
        }
    }

    //now add the binary-tree encoded & compressed map
    //from the LUT to the stream. If, instead, the plain
    //hit map is requested, add the index (which is the
    //plain hit map in fact)
    m_plainHitMap ? addBits64(lutIndex, 16) : addBits64(ITkPixEncoding::ITkPixV2QCoreEncodingLUT_Tree[lutIndex], ITkPixEncoding::ITkPixV2QCoreEncodingLUT_Length[lutIndex]);

    //if dropToT is requested, we can return here
    if (m_dropToT) return;

    //and add the four-bit ToT information for each hit
    for (auto& tot : tots){
        addBits64(tot, 4);
    }
}

bool ITkPixEncoder::hitInQCore(const unsigned CCol, const unsigned QRow) const {
    //Was there a hit in this QCore?

    unsigned col = CCol * m_nColInCCol;
    unsigned row = QRow * m_nRowInQRow;

    for (unsigned pixRow = row; pixRow < row + m_nRowInQRow; pixRow++){
        for (unsigned pixCol = col; pixCol < col + m_nColInCCol; pixCol++){
            if (m_hitMap(pixCol, pixRow)) return true;
        }
    }

    return false;
}

void ITkPixEncoder::scanHitMap() const {

    //only called from mutex-protected function

    //Fill in a helper map of hit QCores and a vector of last qrow in each ccol
    m_hitQCores = std::vector<std::vector<bool>>(m_nCCol, std::vector<bool>(m_nQRow, false));
    m_lastQRow  = std::vector<unsigned> (m_nCCol, 0);

    for (unsigned CCol = 0; CCol < m_nCCol; CCol++){
        for (unsigned QRow = 0; QRow < m_nQRow; QRow++){
            //if there's a hit in the qcore, flag the helper map
            m_hitQCores[CCol][QRow] = hitInQCore(CCol, QRow);
            
            //and keep track of the last qrow in each CCol, so that we can
            //easily set the isLast bit. Numbering starts at 1, in order to
            //keep the m_lastQRow[CCol] == 0 case denoting "no hit" in the CCol
            //to save some looping/ifs later on
            if (m_hitQCores[CCol][QRow]) m_lastQRow[CCol] = QRow + 1;
        }
    }

}

void ITkPixEncoder::encodeEvent() const{

    //only called from mutex-protected function

    //This produces the bits for one event.
    //First, scan the map and produce helpers
    scanHitMap();

    for (unsigned CCol = 0; CCol < m_nCCol; CCol++){
        //if there are no hits in this CCol, continue
        if (m_lastQRow[CCol] == 0) continue;
        //add the 6-bit (CCol + 1) address
        addBits64(CCol + 1, 6);    

        unsigned previousQRow = std::numeric_limits<unsigned>::max();
        for (unsigned QRow = 0; QRow < m_nQRow; QRow++){
            //if there's no hit in this row, continue
            if (!m_hitQCores[CCol][QRow]) continue;            
            
            //add the isLast bit
            QRow + 1 == m_lastQRow[CCol] ? addBits64(0x1, 1) : addBits64(0x0, 1);

            //add the isNeighbor bit. If false, add the QRow address as well.
            if (previousQRow != std::numeric_limits<unsigned>::max() && QRow == previousQRow + 1) {
                addBits64(0x1, 1);
            } else {
                addBits64(0x0, 1);
                addBits64(QRow, 8);
            };

            //add the map and ToT
            encodeQCore(CCol, QRow);

            //update the previous QRow
            previousQRow = QRow;
        }
    }    
}

void ITkPixEncoder::streamTag(const uint8_t nStream) const {

    //only called from mutex-protected function

    //this adds the 8-bit 'global' stream tag
    addBits64(nStream, 8);
}

void ITkPixEncoder::intTag(const uint16_t nEvt) const {

    //only called from mutex-protected function

    //this adds 11 bits of interal tagging between events.
    //does the tag always need to start with 111?
    uint16_t tag = nEvt | (0b111 << 8);
    addBits64(tag, 11);
}

void ITkPixEncoder::setChipID(const uint8_t& chipID){
    m_chipID = chipID;
}

void ITkPixEncoder::clear() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_words.clear();
}