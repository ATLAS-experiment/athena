// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/AthContainersPackedContainersDict.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Dictionary instantiations: PackedContainer classes.
 */


#ifndef ATHCONTAINERS_ATHCONTAINERSPACKEDCONTAINERSDICT_H
#define ATHCONTAINERS_ATHCONTAINERSPACKEDCONTAINERSDICT_H


#include "AthContainers/PackedContainer.h"
#include <vector>


namespace {
  struct DUMMY_INSTANTIATION_ATHCONTAINERS {
    SG::PackedContainer<char>           pchar;
    SG::PackedContainer<unsigned char>  puchar;
    SG::PackedContainer<short>          pshort;
    SG::PackedContainer<unsigned short> pushort;
    SG::PackedContainer<int>            pint;
    SG::PackedContainer<unsigned int>   puint;
    SG::PackedContainer<float>          pfloat;
    SG::PackedContainer<double>         pdouble;

    SG::PackedContainer<std::vector<char> >           pvchar;
    SG::PackedContainer<std::vector<unsigned char> >  pvuchar;
    SG::PackedContainer<std::vector<short> >          pvshort;
    SG::PackedContainer<std::vector<unsigned short> > pvushort;
    SG::PackedContainer<std::vector<int> >            pvint;
    SG::PackedContainer<std::vector<unsigned int> >   pvuint;
    SG::PackedContainer<std::vector<float> >          pvfloat;
    SG::PackedContainer<std::vector<double> >         pvdouble;

    SG::PackedContainer<std::vector<std::vector<char> > >           pvvchar;
    SG::PackedContainer<std::vector<std::vector<unsigned char> > >  pvvuchar;
    SG::PackedContainer<std::vector<std::vector<short> > >          pvvshort;
    SG::PackedContainer<std::vector<std::vector<unsigned short> > > pvvushort;
    SG::PackedContainer<std::vector<std::vector<int> > >            pvvint;
    SG::PackedContainer<std::vector<std::vector<unsigned int> > >   pvvuint;
    SG::PackedContainer<std::vector<std::vector<float> > >          pvvfloat;
    SG::PackedContainer<std::vector<std::vector<double> > >         pvvdouble;
  };
}

#endif // not ATHCONTAINERS_ATHCONTAINERSPACKEDCONTAINERSDICT_H
