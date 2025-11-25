// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/throw_out_of_range.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Helpers for throwing @c out_of_range exceptions.
 *
 * Useful in inlined code.
 */


#ifndef CXXUTILS_THROW_OUT_OF_RANGE_H
#define CXXUTILS_THROW_OUT_OF_RANGE_H


#include <string>
#include <cstdlib>


namespace CxxUtils {


/**
 * @brief Throw an @c out_of_range exception.
 * @param what Description of the error.
 * @param index The index that was out of range.
 * @param size The size of the container.
 * @param obj Pointer to the container (or other relevant object).
 */
[[noreturn]]
void throw_out_of_range (const std::string& what,
                         size_t index, size_t size, const void* obj);



/**
 * @brief Throw an @c out_of_range exception.
 * @param what Description of the error.
 * @param index The index that was out of range.
 * @param size The size of the container.
 * @param obj Pointer to the container (or other relevant object).
 */
[[noreturn]]
void throw_out_of_range (const char* what,
                         size_t index, size_t size, const void* obj);


} // namespace CxxUtils


#endif // not CXXUTILS_THROW_OUT_OF_RANGE_H
