/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1RPClogic/decodeSL.h"

namespace{
  /*
   * Note implicit sign extension: field with type unsigned short (16 bits, unsigned) 
   * is promoted in (unsigned short)field << posit to type int (32 bits, signed), 
   * then sign-extended to type unsigned long (64 bits, unsigned). If 
   * (unsigned short)field << posit is greater than 0x7FFFFFFF, the upper bits of the 
   * result will all be 1.
  */
  unsigned short
  code(unsigned long int slword, unsigned short int field, unsigned short int posit){
    return ((slword&((unsigned short int) field<<posit))>>posit);
  }
}

namespace TriggerRPC{

//****************************************************************************//
unsigned short int moreThan2(unsigned long int slword) {
//
// returns 1 if there are more than 2 candidates in a sector
// returns 0 otherwise
//
 unsigned short int field=0x1;
 unsigned short int posit=0;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short int ROI1(unsigned long int slword) {
//
// returns the ROI identifier of the highest pT ROI
//
 unsigned short int field=0x1f;
 unsigned short int posit=1;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short int OVL1(unsigned long int slword) {
//
// returns the Overlap Flag of the highest pT ROI
//
 unsigned short int field=0x3;
 unsigned short int posit=8;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short int ROI2(unsigned long int slword) {
//
// returns the ROI identifier of the second highest pT ROI
//
 unsigned short int field=0x1f;
 unsigned short int posit=10;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short int OVL2(unsigned long int slword) {
//
// returns the Overlap Flag of the second highest pT ROI
//
 unsigned short int field=0x3;
 unsigned short int posit=17;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short int PT1(unsigned long int slword) {
//
// returns the highest pt code
//
 unsigned short int field=0x7;
 unsigned short int posit=19;
 unsigned short int ptcode = code(slword, field, posit);
 if(!ptcode) {
  return 0;
 } else {
  if(ptcode==7) ptcode=0;
 }
 return ptcode;
}
//****************************************************************************//
unsigned short int PT2(unsigned long int slword) {
//
// returns the second highest pt code
//
 unsigned short int field=0x7;
 unsigned short int posit=22;
 unsigned short int ptcode = code(slword, field, posit);
 if(!ptcode) {
  return 0;
 } else {
  if(ptcode==7) ptcode=0;
 }
 return ptcode;
}
//****************************************************************************//
unsigned short moreThan1_1(unsigned long int slword) {
//
// returns 1 if there is more than 1 candidate in ROI1
// returns 0 otherwise
//
 unsigned short int field=0x1;
 unsigned short int posit=25;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short moreThan1_2(unsigned long int slword) {
//
// returns 1 if there is more than 1 candidate in ROI2
// returns 0 otherwise
//
 unsigned short int field=0x1;
 unsigned short int posit=26;
 return code(slword, field, posit);
}
//****************************************************************************//
unsigned short BunchXID(unsigned long int slword) {
//
// returns the Bunch Crossing identifier of this trigger
//
 unsigned short int field=0x7;
 unsigned short int posit=27;
 return code(slword, field, posit);
}


}
