/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawEvent/LArFebHeader.h"

LArFebHeader::LArFebHeader() {}

//Constructor
LArFebHeader::LArFebHeader(const HWIdentifier febid):m_FEBId(febid) {
  
}

int LArFebHeader::degray(unsigned int x)
{ // like explained in: http://mathworld.wolfram.com/GrayCode.html

  int i,j,sum;

  if ( x==138 ) return 141;
  else if ( x== 136 )  return 142;
  else if ( x== 128 )  return 143;
  else {
    for (i=0; i<32; i++)
      {
        sum=0;
        for (j=i+1; j<32; j++)
          {
            sum += (x>>j)&0x1;
          }
        //      printf("x=%8x sum=%d\n",x,sum );

        if (sum%2) {
          if (x&(1<<i))
            {
              unsigned int tmp1,tmp2=0;
              if (i < 31)
                tmp1 = (x>>(i+1))<<(i+1);
              else
                tmp1 = 0;
              if (i>0)
                tmp2 = x & ((1u<<i)-1);
              //    printf("tmp1 =%x tmp2=%x\n",tmp1,tmp2 );

              x=tmp1|tmp2;
            }
          else
            x |= (1<<i);
        }
      }
    return x;
  }
}
