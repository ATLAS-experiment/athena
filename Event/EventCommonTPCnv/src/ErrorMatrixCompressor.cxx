/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 *  ErrorMatrixCompressor.cxx
 *
 *  Created by Dmitry Emeliyanov on 12/11/08.
 *  <Dmitry.Emeliyanov@cern.ch>
 *
 */
#include "EventCommonTPCnv/ErrorMatrixCompressor.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <format>


void DecoderFloat_IEEE754::print()
{
 std::cout << std::format("{:032b}\n", m_data);
}

short int DecoderFloat_IEEE754::getExponent()
{
  std::uint32_t mask = 0x7f800000u;
  return ((m_data & mask) >> 23);
}

unsigned int DecoderFloat_IEEE754::getMantissa()
{
  std::uint32_t mask = 0x007fffffu;
  return (m_data & mask);
}


void DecoderFloat_IEEE754::setExponent(short int ex)
{
  std::uint32_t buf=ex;
  buf=buf<<23;
  std::uint32_t mask=0x7F800000u;
  m_data = m_data | (buf & mask);
}

void DecoderFloat_IEEE754::setSign(int s)
{
  if(s>0)
    {
      m_data = m_data & 0x7FFFFFFFu;
    }
  else
    {
      m_data= m_data | 0x80000000u;
    }
}


ErrorMatrixCompressor::ErrorMatrixCompressor(int b)
{
  short int biasArray[6] = {116,120,118,116,116,102};


  double T[5]={0.2,0.5,100.0,100.0,20000.0};

  m_bitStrip=b;
  for(int i=0;i<6;i++)
    {
      m_biases[i]=biasArray[i];
    }
  for(int i=0;i<5;i++)
    {
      m_scales[i]=T[i];
    }
  m_limits[0]=15;
  m_limits[1]=31;
  m_srcMasks[0]= 0x80000000;
  m_srcMasks[1]= 0xC0000000; 
  m_srcMasks[2]= 0xE0000000; 
  m_srcMasks[3]= 0xF0000000;
  m_srcMasks[4]= 0xF8000000; 
  m_srcMasks[5]= 0xFC000000;
  m_srcMasks[6]= 0xFE000000;
  m_srcMasks[7]= 0xFF000000;
  m_srcMasks[8]= 0xFF800000;
  m_srcMasks[9]= 0xFFC00000;
  m_srcMasks[10]=0xFFE00000;
  m_srcMasks[11]=0xFFF00000;
  m_srcMasks[12]=0xFFF80000;
  m_srcMasks[13]=0xFFFC0000;
  m_srcMasks[14]=0xFFFE0000;
  m_srcMasks[15]=0xFFFF0000;
  m_tripVec.clear();
  m_tripVec.push_back(Triplet(0,1,3));
  m_tripVec.push_back(Triplet(2,4,6));
  m_tripVec.push_back(Triplet(5,7,8));
  m_tripVec.push_back(Triplet(9,10,11));
  m_tripVec.push_back(Triplet(14,12,13));
}

void ErrorMatrixCompressor::setBiases(const int b[6])
{
  for(int i=0;i<6;i++)
    {
      m_biases[i]=b[i];
    }
}

void ErrorMatrixCompressor::setUpperLimits(const int l[2])
{
  for(int i=0;i<2;i++)
    {
      m_limits[i]=l[i];
    }
}


bool ErrorMatrixCompressor::CholeskyDecomposition(double a[5][5], double L[5][5])
{

  int i{},j{},k{};
  double sum{};
  double p[5]{};

  for(i=0;i<5;i++)
    {
      for(j=i;j<5;j++)
	{
	  sum=a[i][j];
	  for(k=i-1;k>=0;k--)
	    sum-=a[i][k]*a[j][k];
	  if(i==j)
	    {
	      if(sum<=0.0)
		{
		  return false;
		}
	      p[i]=std::sqrt(sum);
	      L[i][i]=p[i];
	    }
	  else
	    { 
	      a[j][i]=sum/p[i];
	      L[j][i]=a[j][i];
	    }
	}
    }
  return true;
}

bool ErrorMatrixCompressor::compress(const std::vector<double>& src, std::vector<unsigned int>& dest)
{
  int i{},j{};
  double C0[5][5]{},C[5][5]{};
 

  dest.clear();

  int idx=0;
  for (i=0;i<5;i++)
    for(j=0;j<=i;j++)
      {
	C[i][j]=src[idx];
	C[j][i]=C0[i][j]=C0[j][i]=C[i][j];
	idx++;
      }
  double L[5][5]{};

  for(i=0;i<5;i++)
    for(j=0;j<=i;j++) 
      {
	C[i][j]*=(m_scales[i]*m_scales[j]);
	C[j][i]=C[i][j];
      }
  if(!CholeskyDecomposition(C,L)) return false;
  //
  float S[5][5]{};
  for(i=0;i<5;i++)
    for(j=0;j<=i;j++) S[i][j]=L[i][j];

  std::vector<FloatRep> vecFR;
  vecFR.clear();

  for(i=0;i<5;i++)
    {
      for(j=0;j<=i;j++) 
	{
	  m_decoder.setF(S[i][j]);
	  char sign=(S[i][j]<0)?1:0;
	  std::uint32_t mant=m_decoder.getMantissa();
	  unsigned short int ex=m_decoder.getExponent();
	  vecFR.push_back(FloatRep(sign,ex,mant));
	}
    }
  std::vector<unsigned short> vShorts;
  vShorts.clear();
  if(!compressFR(vecFR,vShorts)) return false;

  //re-packing as ints

  int nBits=0;
  std::uint32_t buffer = 0x00000000u;

  for(std::vector<unsigned short>::iterator it = vShorts.begin(); it != vShorts.end();++it) {
    // printf("short = 0x%X\n",(*it));
    if(nBits==2) {
      dest.push_back(buffer);
      nBits=0;
      buffer = 0x00000000;
    }
    if(nBits==0) {
      buffer = (*it);
      buffer = buffer << 16;
      nBits++;
    }
    else {
      buffer = buffer | (*it);
      nBits++;
    }
  }
  dest.push_back(buffer);
  return true;
}
  
bool ErrorMatrixCompressor::restore(const std::vector<unsigned int>& src, std::vector<double>& dest)
{
  int i{},j{};
  double C[5][5]{};

  dest.clear();

  std::vector<FloatRep> vecFR;
  vecFR.clear();

  std::vector<unsigned short> vShorts;

  vShorts.clear();

  for (unsigned int ii : src) {
    unsigned short s1,s2;

    s1 = (unsigned short)((0xFFFF0000 & ii) >> 16);
    s2 = (unsigned short)(0x0000FFFF & ii);
    vShorts.push_back(s1);
    //if(!((s2==0) && ((it+1)==src.end()))) //do not store last zero
    vShorts.push_back(s2);
  }

  if(!restoreFR(vShorts,vecFR)) return false;

  std::vector<FloatRep>::iterator fIt(vecFR.begin());
  float S[5][5]{};
  for(i=0;i<5;i++)
    for(j=0;j<=i;j++)
      {
	if(fIt==vecFR.end()) break;
	S[i][j]=(*fIt).restore();
        ++fIt;
      }

  double L[5][5]{};
  for(i=0;i<5;i++)
    for(j=0;j<=i;j++) L[i][j]=S[i][j];
  for(i=0;i<5;i++)  
    for(j=i;j<5;j++)
      {
	C[i][j]=0.0;
	for(int k=0;k<5;k++)
	  C[i][j]+=L[i][k]*L[j][k];
	C[j][i]=C[i][j];
      }
  for(i=0;i<5;i++)  
    for(j=0;j<=i;j++)
      {
	C[i][j]/=(m_scales[i]*m_scales[j]);
	C[j][i]=C[i][j];
      }
  for(i=0;i<5;i++) for(j=0;j<=i;j++) dest.push_back(C[i][j]);
  return true;
}


bool ErrorMatrixCompressor::compressFR(const std::vector<FloatRep>& src, std::vector<unsigned short>& dest)
{
  unsigned short buf=0x0000;
  dest.clear();
  int nMantLength=23-m_bitStrip;
  if(nMantLength<8)
    {
      std::cout<<"Requested mantissa reduction is too large: 23->"<<nMantLength<<std::endl;
      return false;
    }
  std::vector<FloatRep>::const_iterator fIt;
  
  // 1. Check limits
  fIt=src.begin();
  int i,j;
  for (i=0;i<5;i++)
    for (j=0;j<=i;j++)
      {
	unsigned short int ex=(*fIt).exponent();
	std::uint32_t mant=(*fIt).mantissa();
	int bias,limit;
	if(i==j)
	  {
	    bias=m_biases[i];
	    limit=m_limits[0];
	  }
	else
	  {
	    bias=m_biases[5];
	    limit=m_limits[1];
	  }
	if((ex!=0)&&(mant!=0))
	  {
	    if((ex<bias)||(ex-bias>limit))
	      {
		return false;
	      }
	  }
	++fIt;
      }
  // 2. Pack exponents: diag e + non-diag s1 + non-diag e1 + non-diag s2 + non-diag e2
	
  i=0;
  for(std::vector<Triplet>::iterator trIt=m_tripVec.begin();trIt!=m_tripVec.end();++trIt)
    {
      int i1,i2[2];
      i1=(*trIt).m_d;
      i2[0]=(*trIt).m_nd1;i2[1]=(*trIt).m_nd2;
      //printf("%d %d %d\n",i1,i2[0],i2[1]);
      buf=0x0000;
      unsigned short e;
      
      e=src[i1].exponent()-m_biases[i];
      buf = buf | ((e<<12) & 0xF000);      
      if((src[i2[0]].exponent()==0)&&(src[i2[0]].mantissa()==0))
	{
	  e=src[i2[0]].exponent();
	  buf = buf | 0x0800;//-0
	}
      else
	{
	  if(src[i2[0]].sign())
	    buf = buf | 0x0800;
	  e=src[i2[0]].exponent()-m_biases[5];
	}
      buf = buf | ((e<<6) & 0x07C0);      
      if((src[i2[1]].exponent()==0)&&(src[i2[1]].mantissa()==0))
	{
	  buf = buf | 0x0020;//-0
	  e=src[i2[1]].exponent();
	}
      else
	{
	  e=src[i2[1]].exponent()-m_biases[5];
	  if(src[i2[1]].sign())
	    buf = buf | 0x0020;
	}
      buf = buf | (e & 0x001F);
      //      printf("Exponent 0x%X\n",buf);
      dest.push_back(buf);
      i++;
    }
  // 3. Pack reduced mantissas 
  fIt=src.begin();
  unsigned int nPacked=0;
  int nFreeBits=0,nBitsToStore=0,nBufferLength=0;
  std::uint32_t srcBuffer=0x00000000u;
  //  printf("L=%d\n",nMantLength);
  while (nPacked<=src.size()+1)
    {
      if(nFreeBits==0)
	{
	  if(nBufferLength!=0)
	    {
	      dest.push_back(buf);
	      //      printf("Storing 0x%X\n",buf);
	    }
	  buf=0x0000;
	  nFreeBits=16;
	  nBufferLength++;
	}
      if(nBitsToStore==0)
	{
	  if(nPacked!=0)
	    ++fIt;
	  if(fIt==src.end()) break;
	  nPacked++;
	  srcBuffer=((*fIt).mantissa()<<9);
	  // printf("Packing 0x%X = %ud\n",(*fIt).mantissa(),(*fIt).mantissa());
	  nBitsToStore=nMantLength;
	}
      int Np=(nBitsToStore>nFreeBits)?nFreeBits:nBitsToStore;
      std::uint32_t mask=m_srcMasks[Np-1];
      std::uint32_t slice = srcBuffer & mask;
      slice = (slice >> (32-nFreeBits)) & 0x0000FFFF;
      std::uint32_t tmp = slice;
      //	printf("Np=%d Tmp=0x%X\n",Np,tmp);
      buf =  buf | tmp;
      srcBuffer = srcBuffer << Np;
      //	printf("dest=0x%X src=0x%X\n",buf,srcBuffer);
      nFreeBits-=Np;
      nBitsToStore-=Np;
    }
  dest.push_back(buf);
  return true;
}
  
bool ErrorMatrixCompressor::restoreFR(const std::vector<unsigned short>& src, std::vector<FloatRep>& dest)
{
  int i,nRestored,nFreeBits,nBitsToStore;
  unsigned short buf=0x0000;
  std::uint32_t destBuffer=0x00000000;
  std::vector<unsigned short>::const_iterator uIt(src.begin());
  dest.clear();

  for(i=0;i<15;i++)
    {
      dest.push_back(FloatRep(0,0,0));
    }

  i=0;
  for(;uIt!=src.end();++uIt)
    {
      int i0=m_tripVec[i].m_d;
      int i1=m_tripVec[i].m_nd1;
      int i2=m_tripVec[i].m_nd2;
      buf=(*uIt);
      char s=((buf & 0x0800)==0)?0:1;
      dest[i1].sign(s);
      s=((buf & 0x0020)==0)?0:1;
      dest[i2].sign(s);
      unsigned short e = ((buf & 0xF000) >> 12);
      dest[i0].exponent(e+m_biases[i]);
      e = ((buf & 0x07C0) >> 6);
      dest[i1].exponent(e+m_biases[5]);
      e = (buf & 0x001F);
      dest[i2].exponent(e+m_biases[5]);
      i++;
      if(i==5) break;
    }
  if(i<5) return false;
  nRestored=0;
  nBitsToStore=0;
  nFreeBits=0;
  int nMantLength=23-m_bitStrip;
  while(nRestored<=15)
    {	
      if(nFreeBits==0)
	{
	  if(nRestored!=0)
	    {
	      //printf("Dest 0x%X ",destBuffer);
	      destBuffer = destBuffer << m_bitStrip;
	      //printf("<< 0x%X = %lu\n",destBuffer,destBuffer);
	      dest[nRestored-1].mantissa(destBuffer);
	    }
	  ++nRestored;
	  destBuffer=0x00000000;
	  nFreeBits=nMantLength;
	}
      if(nBitsToStore==0)
	{
          if(uIt==src.end()) break;
	  ++uIt;
          if(uIt==src.end()) 
	    {
	      //printf("Breaking ... nR=%d\n",nRestored);
	      break;
	    }
	  nBitsToStore=16;buf=(*uIt);//printf("Source 0x%X\n",buf);
	}
      int Np=(nFreeBits>nBitsToStore) ? nBitsToStore : nFreeBits;
      std::uint32_t tmp = buf;
      tmp = tmp >> (16-Np);
      buf = ((buf << Np) & 0x0000FFFFu);
      //printf("F=%d TS=%d Np=%d 0x%X 0x%X\n",nFreeBits,nBitsToStore,Np,tmp,buf);
      nBitsToStore-=Np;
      //printf("Copy 0x%X ",destBuffer);
      destBuffer = destBuffer << Np;
      //printf("<< 0x%X ",destBuffer);
      destBuffer = destBuffer | tmp;
      //printf(" | 0x%X\n",destBuffer);
      nFreeBits-=Np;
      //printf("nR=%d\n",nRestored);
    }
  return true;
}

