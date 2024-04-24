/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           FEXAlgoSpaceDefs.h  -
//                              -------------------
//     begin                : 08 02 2021
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef FEXAlgoSpaceDefs_H
#define FEXAlgoSpaceDefs_H

namespace LVL1 {

  //Doxygen class description below:
  /** The FEXAlgoSpaceDefs class defines the size of a single jFEX algorithm window, for use in jFEXFPGA
  */

  class FEXAlgoSpaceDefs {

  public:

    constexpr static int jFEX_wide_algoSpace_width = 45;
    constexpr static int jFEX_thin_algoSpace_width = 24;
    constexpr static int jFEX_algoSpace_height = 32; 

 
//Array breakdown of jFEX_wide_algoSpace_width matric to indicate different eta and phi regions

    constexpr static int jFEX_FCAL1_start =  700000; 
    constexpr static int jFEX_FCAL2_start =  900000; 
    constexpr static int jFEX_FCAL3_start = 1100000;

    //Pileup bitshifts
    constexpr static unsigned int pu_AreaINV = 6; 
    constexpr static unsigned int pu_Area    = 11; 
    constexpr static unsigned int pu_rhoLUT  = 20; 
    
    
//A side (jFEX Module 5):

    //Eta space for core area 
    constexpr static int jFEX_algoSpace_A_EMB_eta = 8;
    constexpr static int jFEX_algoSpace_A_EMIE_eta = 17;
    constexpr static int jFEX_algoSpace_A_FCAL_start_eta = 21; 
    constexpr static int jFEX_algoSpace_A_FCAL_end_eta = 33;
 
    //Eta space for core and overlaps, EM only
    constexpr static int jFEX_algoSpace_A_lowerEM_eta = 0;
    constexpr static int jFEX_algoSpace_A_upperEM_eta = 33;
    
    //Eta space for core and overlaps, FCAL 2 (33-40) and 3 (41-44)
    constexpr static int jFEX_algoSpace_A_lowerFCAL_eta = 33;
    constexpr static int jFEX_algoSpace_A_upperFCAL2_eta = 41;
    constexpr static int jFEX_algoSpace_A_upperFCAL_eta = 45;

//C side (jFEX module 0):
//there are more values on the C side as the matrix cannot be flipped in the bitwise framework.

   //Eta space for core area 
    constexpr static int jFEX_algoSpace_C_EMB_start_eta = 28;
    constexpr static int jFEX_algoSpace_C_EMB_end_eta = 37;
    constexpr static int jFEX_algoSpace_C_EMIE_start_eta = 24;
    constexpr static int jFEX_algoSpace_C_EMIE_end_eta = 28;
    constexpr static int jFEX_algoSpace_C_FCAL_start_eta = 12;
    constexpr static int jFEX_algoSpace_C_FCAL_end_eta = 24;
 
   //Eta space for core and overlaps, EM only
    constexpr static int jFEX_algoSpace_C_lowerEM_eta = 12;
    constexpr static int jFEX_algoSpace_C_upperEM_eta = 45;
    
    //Eta space for core and overlaps, FCAL 2 (4-11) and 3 (0-3)
    constexpr static int jFEX_algoSpace_C_lowerFCAL_eta = 0;
    constexpr static int jFEX_algoSpace_C_lowerFCAL2_eta = 4;
    constexpr static int jFEX_algoSpace_C_upperFCAL_eta = 12;

    //First and second FCAL 1st layer  eta bins
    constexpr static int jFEX_algoSpace_FCAL1_2nd =  22;
    constexpr static int jFEX_algoSpace_A_FCAL1_1st =  21;
    constexpr static int jFEX_algoSpace_C_FCAL1_1st =  23;
    
    //Lowest/highest  eta for jFEX Electrons
    constexpr static int jFEX_algoSpace_A_FwdEl_start =  14;
    constexpr static int jFEX_algoSpace_C_FwdEl_start =  30;

    //Phi space breakdown
    constexpr static int jFEX_algoSpace_EMB_start_phi =  8;    
    constexpr static int jFEX_algoSpace_EMB_end_phi  = 24; 
    constexpr static int jFEX_algoSpace_EMIE_start_phi =  4;  
    constexpr static int jFEX_algoSpace_EMIE_end_phi =  12;
    constexpr static int jFEX_algoSpace_FCAL_start_phi =  2;
    constexpr static int jFEX_algoSpace_FCAL_end_phi =  6;
    
// JFEX bit shifting in the TOB words
    // jFEX Taus (jTau)
      // Data locations within word
      constexpr static int jTau_isoBit = 21;
      constexpr static int jTau_etBit  = 10;
      constexpr static int jTau_etaBit = 5;
      constexpr static int jTau_phiBit = 1;
      constexpr static int jTau_satBit = 0;

      // jFEX Forward Electrons (jEM)
      // Data locations within word
      constexpr static int jEM_resBit  = 27;
      constexpr static int jEM_emf2Bit = 25;
      constexpr static int jEM_emf1Bit = 23;
      constexpr static int jEM_isoBit  = 21;
      constexpr static int jEM_etBit   = 10;
      constexpr static int jEM_etaBit  = 5;
      constexpr static int jEM_phiBit  = 1;
      constexpr static int jEM_satBit  = 0;
      
    // jFEX SRJets (jJ)
      // Data locations within word
      constexpr static int jJ_resBit = 21;
      constexpr static int jJ_etBit  = 10;
      constexpr static int jJ_etaBit = 5;
      constexpr static int jJ_phiBit = 1;
      constexpr static int jJ_satBit = 0;
      
    // jFEX LRJets (jLJ)
      // Data locations within word
      constexpr static int jLJ_resBit = 23;
      constexpr static int jLJ_etBit  = 10;
      constexpr static int jLJ_etaBit = 5;
      constexpr static int jLJ_phiBit = 1;
      constexpr static int jLJ_satBit = 0; 
      
    // jFEX MET (jXE)
      // Data locations within word
      constexpr static int jXE_ResBit   = 31;
      constexpr static int jXE_Ey_Bit   = 16;
      constexpr static int jXE_Ex_Bit   = 1;
      constexpr static int jXE_SatBit   = 0;
      
    // jFEX SumET (jTE)
      // Data locations within word
      constexpr static int jTE_Sat_upperBit = 31;
      constexpr static int jTE_Et_upperBit  = 16;
      constexpr static int jTE_Et_lowerBit  = 1; 
      constexpr static int jTE_Sat_lowerBit = 0; 


   //define constants needed by gFEX Jet algorithm
    constexpr static int centralNphi = 32;
    constexpr static int forwardNphi = 16;
    constexpr static int centralNeta = 12;
    constexpr static int forwardNeta = 6;
    constexpr static int totalNeta = 40;

    constexpr static int n_partial = 4;
    constexpr static int ABCrows = 32;
    constexpr static int ABcolumns = 12;
    constexpr static int ABCcolumnsEng = 6;
    constexpr static int gJetMax = 0x00000FFF; //Corresponds to the maximum value with 12 bits (4095)
    constexpr static int gBlockMax = 0x00000FFF; //Corresponds to the maximum value with 12 bits (4095)
    constexpr static int jetThr = 0x001E;
    
    constexpr static bool ENABLE_INTER_AB = true;
    constexpr static bool ENABLE_INTER_C = true;
    constexpr static bool ENABLE_INTER_ABC = false;
    constexpr static int gJetTOBfib = 6;
    constexpr static int BTOBFIB = 6;
    constexpr static bool ENABLE_PUC = true;
    constexpr static bool APPLY_TRUNC = false;
    //define constants needed by gFEX JwoJ algorithm
    constexpr static bool ENABLE_JWOJ_C = false;

    constexpr static int PS_UPPER_AB =  2047;
    constexpr static int PS_LOWER_AB = -2048;
    constexpr static int PS_SHIFT_AB = 3;
    constexpr static int PS_UPPER_C  =  511;
    constexpr static int PS_LOWER_C  = -511;
    constexpr static int PS_SHIFT_C  = 3;

    constexpr static int fineCeiling = 255;
    constexpr static int fineFloor   = -256; 

    constexpr static std::array<unsigned int, 385> inv19 = {
      0x00000,0x45000,0x22800,0x17000,0x11400,0x0DCC8,0x0B800,0x09DB0,
      0x08A00,0x07AA8,0x06E60,0x06458,0x05C00,0x054E8,0x04ED8,0x04998,
      0x04500,0x040F0,0x03D50,0x03A18,0x03730,0x03490,0x03228,0x03000,
      0x02E00,0x02C28,0x02A70,0x028E0,0x02768,0x02610,0x024C8,0x02398,
      0x02280,0x02170,0x02078,0x01F88,0x01EA8,0x01DD0,0x01D08,0x01C48,
      0x01B98,0x01AE8,0x01A48,0x019A8,0x01910,0x01888,0x01800,0x01778,
      0x01700,0x01680,0x01610,0x015A0,0x01538,0x014D0,0x01470,0x01410,
      0x013B0,0x01358,0x01308,0x012B0,0x01260,0x01218,0x011C8,0x01180,
      0x01140,0x010F8,0x010B8,0x01078,0x01038,0x01000,0x00FC0,0x00F88,
      0x00F50,0x00F18,0x00EE8,0x00EB8,0x00E80,0x00E50,0x00E20,0x00DF8,
      0x00DC8,0x00DA0,0x00D70,0x00D48,0x00D20,0x00CF8,0x00CD0,0x00CB0,
      0x00C88,0x00C60,0x00C40,0x00C20,0x00C00,0x00BD8,0x00BB8,0x00B98,
      0x00B80,0x00B60,0x00B40,0x00B20,0x00B08,0x00AE8,0x00AD0,0x00AB0,
      0x00A98,0x00A80,0x00A68,0x00A50,0x00A38,0x00A20,0x00A08,0x009F0,
      0x009D8,0x009C0,0x009A8,0x00998,0x00980,0x00968,0x00958,0x00940,
      0x00930,0x00918,0x00908,0x008F8,0x008E0,0x008D0,0x008C0,0x008B0,
      0x008A0,0x00888,0x00878,0x00868,0x00858,0x00848,0x00838,0x00828,
      0x00818,0x00808,0x00800,0x007F0,0x007E0,0x007D0,0x007C0,0x007B8,
      0x007A8,0x00798,0x00788,0x00780,0x00770,0x00768,0x00758,0x00748,
      0x00740,0x00730,0x00728,0x00718,0x00710,0x00708,0x006F8,0x006F0,
      0x006E0,0x006D8,0x006D0,0x006C0,0x006B8,0x006B0,0x006A0,0x00698,
      0x00690,0x00688,0x00678,0x00670,0x00668,0x00660,0x00658,0x00648,
      0x00640,0x00638,0x00630,0x00628,0x00620,0x00618,0x00610,0x00608,
      0x00600,0x005F0,0x005E8,0x005E0,0x005D8,0x005D0,0x005C8,0x005C0,
      0x005C0,0x005B8,0x005B0,0x005A8,0x005A0,0x00598,0x00590,0x00588,
      0x00580,0x00578,0x00574,0x00570,0x00568,0x00560,0x00558,0x00550,
      0x0054C,0x00548,0x00540,0x00538,0x00530,0x0052C,0x00528,0x00520,
      0x00518,0x00514,0x00510,0x00508,0x00500,0x004FC,0x004F8,0x004F0,
      0x004EC,0x004E8,0x004E0,0x004D8,0x004D4,0x004D0,0x004C8,0x004C4,
      0x004C0,0x004B8,0x004B4,0x004B0,0x004AC,0x004A8,0x004A0,0x0049C,
      0x00498,0x00490,0x0048C,0x00488,0x00484,0x00480,0x0047C,0x00478,
      0x00470,0x0046C,0x00468,0x00464,0x00460,0x0045C,0x00458,0x00454,
      0x00450,0x00448,0x00444,0x00440,0x0043C,0x00438,0x00434,0x00430,
      0x0042C,0x00428,0x00424,0x00420,0x0041C,0x00418,0x00414,0x00410,
      0x0040C,0x00408,0x00405,0x00402,0x00400,0x003FC,0x003F8,0x003F4,
      0x003F0,0x003EC,0x003E8,0x003E4,0x003E0,0x003DD,0x003DA,0x003D8,
      0x003D4,0x003D0,0x003CC,0x003C8,0x003C5,0x003C2,0x003C0,0x003BC,
      0x003B8,0x003B5,0x003B2,0x003B0,0x003AC,0x003A8,0x003A5,0x003A2,
      0x003A0,0x0039D,0x0039A,0x00398,0x00394,0x00390,0x0038D,0x0038A,
      0x00388,0x00385,0x00382,0x00380,0x0037D,0x0037A,0x00378,0x00375,
      0x00372,0x00370,0x0036D,0x0036A,0x00368,0x00365,0x00362,0x00360,
      0x0035D,0x0035A,0x00358,0x00355,0x00352,0x00350,0x0034D,0x0034A,
      0x00348,0x00345,0x00342,0x00340,0x0033D,0x0033A,0x00338,0x00336,
      0x00334,0x00332,0x00330,0x0032D,0x0032A,0x00328,0x00326,0x00324,
      0x00322,0x00320,0x0031D,0x0031A,0x00318,0x00316,0x00314,0x00312,
      0x00310,0x0030E,0x0030C,0x0030A,0x00308,0x00306,0x00304,0x00302,
      0x00300,0x002FD,0x002FA,0x002F8,0x002F6,0x002F4,0x002F2,0x002F0,
      0x002EE,0x002EC,0x002EA,0x002E8,0x002E6,0x002E5,0x002E3,0x002E1,
      0x002E0}; 


  };

} // end of namespace

#endif
