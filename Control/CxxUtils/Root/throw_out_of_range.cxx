/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/Root/throw_out_of_range.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Helpers for throwing @c out_of_range exceptions.
 */


#include "CxxUtils/throw_out_of_range.h"
#include <format>
#include <stdexcept>


namespace CxxUtils {


/**
 * @brief Throw an @c out_of_range exception.
 * @param what Description of the error.
 * @param index The index that was out of range.
 * @param size The size of the container.
 * @param obj Pointer to the container (or other relevant object).
 */
void throw_out_of_range (const std::string& what,
                         size_t index, size_t size, const void* obj)
{
  throw std::out_of_range (std::format
    ("CxxUtils::throw_out_of_range {} requested index {} >= {} for object at {}",
     what, index, size, obj));
};


/**
 * @brief Throw an @c out_of_range exception.
 * @param what Description of the error.
 * @param index The index that was out of range.
 * @param size The size of the container.
 * @param obj Pointer to the container (or other relevant object).
 */
void throw_out_of_range (const char* what,
                         size_t index, size_t size, const void* obj)
{
  throw_out_of_range (std::string(what), index, size, obj);
}


} // namespace CxxUtils
