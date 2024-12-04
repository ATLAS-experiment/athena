// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/AthContainersPmrDict.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Dictionary instantiations: polymorphic allocator classes.
 */


#ifndef ATHCONTAINERS_ATHCONTAINERSPMRDICT_H
#define ATHCONTAINERS_ATHCONTAINERSPMRDICT_H


#include <vector>
#include <memory>


template class std::vector<char, std::pmr::polymorphic_allocator<char> >;
template class std::vector<unsigned char, std::pmr::polymorphic_allocator<unsigned char> >;
template class std::vector<short, std::pmr::polymorphic_allocator<short> >;
template class std::vector<unsigned short, std::pmr::polymorphic_allocator<unsigned short> >;
template class std::vector<int, std::pmr::polymorphic_allocator<int> >;
template class std::vector<unsigned int, std::pmr::polymorphic_allocator<unsigned int> >;
template class std::vector<float, std::pmr::polymorphic_allocator<float> >;
template class std::vector<double, std::pmr::polymorphic_allocator<double> >;


#endif // not ATHCONTAINERS_ATHCONTAINERSPMRDICT_H
