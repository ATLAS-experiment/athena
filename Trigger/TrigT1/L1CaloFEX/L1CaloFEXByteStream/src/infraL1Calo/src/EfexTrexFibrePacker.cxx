/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "infraL1Calo/EfexTrexFibrePacker.h"

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <map>


std::vector<FibrePackerBase::myDataWord> EfexTrexFibrePacker::getPackedData(const std::vector<myDataWord>& inFrame,
                                                             myDataWord bcNumber, 
                                                             InputDataFrameType frameType) const {

    std::vector<myDataWord> dataToLoad(FexDefs::num32BitWordsPerFibre(),0);
    auto supercells = inFrame;

    switch(frameType){
                    
        case InputDataFrameType::Normal:		     // Standard data pattern
            {
            for(auto& icell:supercells)
                icell = icell & 0x3ff;              // truncate to 10 bits 

            myDataWord bcId     = bcNumber    & 0x7f ;              // 7 bits from BCID
            myDataWord bcIdFull = bcNumber    & 0xfff;              // full 12 bit BCID
            myDataWord bcId02   = bcId & 0x7;
            myDataWord bcId34   = (bcId >> 3) & 0x3;
            myDataWord bcId56   = (bcId >> 5) & 0x3;

            dataToLoad.at(0) =  (bcId56 << 8)               | ((supercells.at(0)&0xff) <<10)   | 
                                ((supercells.at(1)&0xff) << 20)    | (bcId34 << 30);
            dataToLoad.at(1) =  (supercells.at(2)&0xff)            | ((supercells.at(3)&0xff) << 10)  | 
                                ((supercells.at(4)&0xff) << 20);
            dataToLoad.at(2) =  (supercells.at(5)&0xff)            | ((supercells.at(6)&0xff) << 10)  | 
                                ((supercells.at(7)&0xff) << 20);
            dataToLoad.at(3) =  (supercells.at(8)&0xff)            | ((supercells.at(9)&0xff) << 10)  | 
                                ((supercells.at(10)&0xff) << 20);
            dataToLoad.at(4) =  (supercells.at(11)&0xff)           | ((supercells.at(12)&0xff) << 10) | 
                                ((supercells.at(13)&0xff) << 20);
            dataToLoad.at(5) =  (supercells.at(14)&0xff)           | ((supercells.at(15)&0xff) << 10) ;
            dataToLoad.at(6) =  bcIdFull                    | bcId02 << 20;

            myDataWord tempCRC = crc9d32(dataToLoad,6l,true);               // calculate CRC32 on first 6 words
            myDataWord myCRCReminder = crc9d23(dataToLoad[6],tempCRC,true); // CRC23 on the last word

            dataToLoad.at(0) =  K_28_5         | dataToLoad.at(0) ;          // add K-character
            dataToLoad[6]    =  dataToLoad[6]  | ( myCRCReminder << 23) ;        // add CRC check

            break;
            }
        case InputDataFrameType::Alignement:		// Special pattern, LATOME alignement/mapping frame
            {

            myDataWord bcId = bcNumber   & 0xfff ;                          // 12 bits of BCID

            dataToLoad.at(6) =  bcId; 
            dataToLoad.at(0) =  (K_28_0 << 8)   | dataToLoad.at(0) ;          // add K_28_0 -character, used for CRC calculation

            myDataWord tempCRC = crc9d32(dataToLoad,6l,true);               // calculate CRC32 on first 6 words
            myDataWord myCRCReminder = crc9d23(dataToLoad[6],tempCRC,true); // CRC23 on the last word

            dataToLoad.at(0) =  K_28_5    | dataToLoad.at(0) ;          // add K-character
            dataToLoad[6]    =  dataToLoad[6]  | ( myCRCReminder << 23) ;        // add CRC check to the famous last word

            break;
            }
        }

    return dataToLoad; 

}    

std::vector<FibrePackerBase::myDataWord> EfexTrexFibrePacker::getPackedControl(const std::vector<myDataWord>& /*inFrame*/,
                                                                myDataWord /*bcNumber*/,
                                                                InputDataFrameType frameType) const {

    std::vector<myDataWord> controlWords(FexDefs::num32BitWordsPerFibre(),0);

    switch(frameType){
                    
        case InputDataFrameType::Normal:		            // one K character in first data word
            controlWords.at(0) = 0x1;
            break;

        case InputDataFrameType::Alignement:		        // two K characters in first data word
            controlWords.at(0) = 0x3;
            break;
        
    } 

    return controlWords;
}    

bool EfexTrexFibrePacker::checkCRC(const std::vector<myDataWord>& encodedData, 
                                    InputDataFrameType frameType) const {

    auto inputData = encodedData;
    myDataWord CRCCheck = 0;

    switch(frameType){
                    
        case InputDataFrameType::Normal:		                    // one K character in first data word
            inputData.at(0) = inputData.at(0) & 0xffffff00;
            CRCCheck = crc9d32(inputData,7l,true);               // calculate CRC32 on first 6 words
            break;

        case InputDataFrameType::Alignement:		                // two K characters in first data word, mask the first one
            inputData.at(0) = inputData.at(0) & 0xffffff00;
            CRCCheck = crc9d32(inputData,7l,true);               // calculate CRC32 on first 6 words
            break;
        
    } 

    return (CRCCheck == 0);
}

FibrePackerBase::myDataWord  EfexTrexFibrePacker::getBcNumber(const std::vector<myDataWord>& encodedData, 
                                               InputDataFrameType /*frameType*/) const {
    myDataWord BcNumber = encodedData.at(6) & 0xfff;
    return BcNumber;
}

FibrePackerBase::myDataWord  EfexTrexFibrePacker::getBcMask(InputDataFrameType /*frameType*/) const {
    // BC number is the full 12 bits from any frame.
    return 0xfff;
}

std::vector<FibrePackerBase::myDataWord>  EfexTrexFibrePacker::getUnpackedData(const std::vector<myDataWord>& encodedData, 
                                                                InputDataFrameType frameType) const {
    std::vector<myDataWord> unpackedData; 

    switch(frameType){
                    
        case InputDataFrameType::Normal:		                    
            {
            std::vector<myDataWord> supercells(EfexDefs::maxSuperCellsPerFibre(),0);

            supercells[0]  = (encodedData[0] >> 10) & 0x3ff;
            supercells[1]  = (encodedData[0] >> 20) & 0x3ff;
            supercells[2]  = (encodedData[1]      ) & 0x3ff;
            supercells[3]  = (encodedData[1] >> 10) & 0x3ff;
            supercells[4]  = (encodedData[1] >> 20) & 0x3ff;
            supercells[5]  = (encodedData[2]      ) & 0x3ff;
            supercells[6]  = (encodedData[2] >> 10) & 0x3ff;
            supercells[7]  = (encodedData[2] >> 20) & 0x3ff;
            supercells[8]  = (encodedData[3]      ) & 0x3ff;
            supercells[9]  = (encodedData[3] >> 10) & 0x3ff;
            supercells[10] = (encodedData[3] >> 20) & 0x3ff;
            supercells[11] = (encodedData[4]      ) & 0x3ff;
            supercells[12] = (encodedData[4] >> 10) & 0x3ff;
            supercells[13] = (encodedData[4] >> 20) & 0x3ff;
            supercells[14] = (encodedData.at(5)      ) & 0x3ff;
            supercells.at(15) = (encodedData[5] >> 10) & 0x3ff;

            unpackedData=std::move(supercells);    
            break;
            }
        case InputDataFrameType::Alignement:		                
            {
            break;              // no hyper data in the frame 
            }
    } 

    return unpackedData;

}    

