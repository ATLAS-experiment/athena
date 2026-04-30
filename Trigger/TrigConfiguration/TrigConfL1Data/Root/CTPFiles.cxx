/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfL1Data/CTPFiles.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace std;

unsigned int TrigConf::CTPFiles::ALL_CTPCORELUT_SIZE = 0;     
unsigned int TrigConf::CTPFiles::ALL_CTPCORECAM_SIZE = 0;
unsigned int TrigConf::CTPFiles::ALL_CTPINMONSEL_SIZE = 0;
unsigned int TrigConf::CTPFiles::ALL_CTPINMONDEC_SIZE = 0;
unsigned int TrigConf::CTPFiles::CTPMON_MUX_OUTPUT_NUMBER = 0;
unsigned int TrigConf::CTPFiles::CTPMON_ADDRESS_SELECTOR_NUMBER = 0;
unsigned int TrigConf::CTPFiles::ALL_CTPMONSELECTOR_SIZE = 0;
unsigned int TrigConf::CTPFiles::ALL_CTPMONDECODER_SIZE = 0;


TrigConf::CTPFiles::CTPFiles() : 
   L1DataBaseclass(),
   m_Lvl1MasterTableId(0),
   m_smxId(0),      
   m_LoadCtpcoreFiles(true),
   m_LoadCtpinFiles(true),
   m_LoadCtpmonFiles(true)
{}

void 
TrigConf::CTPFiles::setFileSizes(unsigned int run) {
   if(run==1) {
      ALL_CTPCORELUT_SIZE = 49664;      
      ALL_CTPCORECAM_SIZE =  8192;
      
      ALL_CTPINMONSEL_SIZE = 124;
      ALL_CTPINMONDEC_SIZE = 4096;

      CTPMON_MUX_OUTPUT_NUMBER = 9;
      CTPMON_ADDRESS_SELECTOR_NUMBER = 24;
      ALL_CTPMONSELECTOR_SIZE = CTPMON_MUX_OUTPUT_NUMBER * CTPMON_ADDRESS_SELECTOR_NUMBER;
      ALL_CTPMONDECODER_SIZE = 6656;
   } else {
      // https://svnweb.cern.ch/trac/atlastdaq/browser/LVL1/ctp/CtpcorePlusModule/trunk/CtpcorePlusModule/CtpcorePlusModule.h
      ALL_CTPCORELUT_SIZE = 725248; // 0xb1100
      ALL_CTPCORECAM_SIZE =  55296;
      
      ALL_CTPINMONSEL_SIZE = 124;
      ALL_CTPINMONDEC_SIZE = 4096;

      CTPMON_MUX_OUTPUT_NUMBER = 9;
      CTPMON_ADDRESS_SELECTOR_NUMBER = 24;
      ALL_CTPMONSELECTOR_SIZE = CTPMON_MUX_OUTPUT_NUMBER * CTPMON_ADDRESS_SELECTOR_NUMBER;
      ALL_CTPMONDECODER_SIZE = 6656;
   }
}


// get the files 
const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpcoreLUT() const {
   return m_ctpcoreLUT;
}

void
TrigConf::CTPFiles::setCtpcoreLUT( const std::vector<u_int>& vec)
{
   if(vec.size() != ALL_CTPCORELUT_SIZE) 
      throw std::runtime_error( "CTPFiles: LUT size incorrect" );
   m_ctpcoreLUT = vec;
}     

const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpcoreCAM() const {
   return m_ctpcoreCAM;
}

void
TrigConf::CTPFiles::setCtpcoreCAM( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPCORECAM_SIZE) 
      throw std::runtime_error( "CTPFiles: CAM size incorrect" );
   m_ctpcoreCAM = vec;
}

// get the different decoder & selection files 
const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonSelectorSlot7() const {
   return m_ctpinMonSelectorSlot7;
}

void
TrigConf::CTPFiles::setCtpinMonSelectorSlot7( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPINMONSEL_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 7 incorrect" );
   m_ctpinMonSelectorSlot7 = vec;
}

const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonSelectorSlot8() const {
   return m_ctpinMonSelectorSlot8;
}

void
TrigConf::CTPFiles::setCtpinMonSelectorSlot8( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPINMONSEL_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 8 incorrect" );
   m_ctpinMonSelectorSlot8 = vec;
}


const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonSelectorSlot9() const {
   return m_ctpinMonSelectorSlot9;
}

void
TrigConf::CTPFiles::setCtpinMonSelectorSlot9( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPINMONSEL_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 9 incorrect" );
   m_ctpinMonSelectorSlot9 = vec;
}


const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpmonSelector() const {
   return m_ctpmonSelector;
}


void
TrigConf::CTPFiles::setCtpmonSelector( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPMONSELECTOR_SIZE) 
      throw std::runtime_error( "CTPFiles: CTPMON Selector size incorrect" );
   m_ctpmonSelector = vec;
}
 

const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonDecoderSlot7() const {
   return m_ctpinMonDecoderSlot7;
}

void TrigConf::CTPFiles::setCtpinMonDecoderSlot7( const std::vector<u_int>& vec ) {
   if(vec.size() != ALL_CTPINMONDEC_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 7 incorrect" );
   m_ctpinMonDecoderSlot7 = vec;
}

const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonDecoderSlot8() const {
   return m_ctpinMonDecoderSlot8;
}

void
TrigConf::CTPFiles::setCtpinMonDecoderSlot8(const std::vector<u_int>& vec) {
   if(vec.size() != ALL_CTPINMONDEC_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 8 incorrect" );
   m_ctpinMonDecoderSlot8 = vec;
}


const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpinMonDecoderSlot9() const {
   return m_ctpinMonDecoderSlot9;
}

void
TrigConf::CTPFiles::setCtpinMonDecoderSlot9(const std::vector<u_int>& vec) {
   if(vec.size() != ALL_CTPINMONDEC_SIZE) 
      throw std::runtime_error( "CTPFiles: MonSel size slot 9 incorrect" );
   m_ctpinMonDecoderSlot9 = vec;
}

const std::vector<uint32_t> &
TrigConf::CTPFiles::ctpmonDecoder() const {
   return m_ctpmonDecoder;
}


void
TrigConf::CTPFiles::setCtpmonDecoder(const std::vector<u_int>& vec) {
   if(vec.size() != ALL_CTPMONDECODER_SIZE) 
      throw std::runtime_error( "CTPFiles: CTPMON Decoder size incorrect" );
   m_ctpmonDecoder = vec;
}
 
// get files stop 

void
TrigConf::CTPFiles::print(const std::string& indent, unsigned int /*detail*/) const {
   const std::ios::fmtflags oldFlags = cout.flags();
   const char oldFill = cout.fill();
   //
   cout << indent << "CTPFiles: \n"; 
   cout << indent << "id:\t\t\t"                 << id() << "\n"; 
   cout << indent << "Name: \t\t\t"               << name() << "\n";
   cout << indent << "Version: \t\t"            << version() << "\n";
   cout << indent << "Print the LUT\n";

   for (unsigned int i=0; i<(ALL_CTPCORELUT_SIZE); i++) 
      cout << indent << i << "\t:t0x" << setw(8) << setfill('0') 
           << hex << m_ctpcoreLUT[i] << dec << "\n";

   cout << indent << "Print the CAM\n";
   for (unsigned int i=0; i<(ALL_CTPCORECAM_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpcoreCAM[i] << dec << "\n";
    
   cout << indent << "Print the CTPIN mon decoders slot 7\n";
   for (unsigned int i=0; i<(ALL_CTPINMONDEC_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonDecoderSlot7[i] << dec << "\n";
   cout << indent << "Print the CTPIN mon decoders slot 8\n";
   for (unsigned int i=0; i<(ALL_CTPINMONDEC_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonDecoderSlot8[i] << dec << "\n";
   cout << indent << "Print the CTPIN mon decoders slot 9\n";
   for (unsigned int i=0; i<(ALL_CTPINMONDEC_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonDecoderSlot9[i] << dec << "\n";

   cout << indent << "Print the CTPIN mon selectors slot 7\n";
   for (unsigned int i=0; i<(ALL_CTPINMONSEL_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonSelectorSlot7[i] << dec << "\n";
   cout << indent << "Print the CTPIN mon selectors slot 8\n";
   for (unsigned int i=0; i<(ALL_CTPINMONSEL_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonSelectorSlot8[i] << dec << "\n";
   cout << indent << "Print the CTPIN mon selectors slot 9\n";
   for (unsigned int i=0; i<(ALL_CTPINMONSEL_SIZE); i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpinMonSelectorSlot9[i] << dec << "\n";
    
   cout << indent << "Print the CTPMON selector\n" ;
   for (unsigned int i=0; i < ALL_CTPMONSELECTOR_SIZE; i++) 
      cout << indent << i << "\t:\t" << setw(2) << setfill(' ') 
           << m_ctpmonSelector[i] << "\n";
   cout << indent << "Print the CTPMON decoder\n";
   for (unsigned int i=0; i < ALL_CTPMONDECODER_SIZE; i++) 
      cout << indent << i << "\t:\t0x" << setw(8) << setfill('0') 
           << hex << m_ctpmonDecoder[i] << dec << "\n";

   cout << indent << "SMX no " << m_smxId << " '" << m_smxName << "'\n" ;
   cout << indent << "VHDL slot 7 : \n";
   cout << m_ctpinSmxVhdlSlot7 << "\n";
   cout << indent << "VHDL slot 8 : \n";
   cout << m_ctpinSmxVhdlSlot8 << "\n";
   cout << indent << "VHDL slot 9 : \n";
   cout << m_ctpinSmxVhdlSlot9 << "\n";

   cout << indent << "SVFI slot 7 : \n";
   cout << m_ctpinSmxSvfiSlot7 << "\n";
   cout << indent << "SVFI slot 8 : \n";
   cout << m_ctpinSmxSvfiSlot8 << "\n";
   cout << indent << "SVFI slot 9 : \n";
   cout << m_ctpinSmxSvfiSlot9 << "\n";
      
   cout << indent << "SMX Output file: \n";
   cout << m_ctpinSmxOutput << endl;
   cout.flags(oldFlags);
   cout.fill(oldFill);
} 
