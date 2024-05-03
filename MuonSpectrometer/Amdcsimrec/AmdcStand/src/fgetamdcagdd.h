/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef fgetamdcagdd_H
#define fgetamdcagdd_H
extern "C" void fgetagddcharnumber_(int& Nb);
extern "C" void fgetamdccharnumber_(int& Nb);
extern "C" void fgetagddcharstring_(int& Nb,char* ChAr, long ChAr_len);
extern "C" void fgetamdccharstring_(int& Nb,char* ChAr, long ChAr_len);
#endif
