// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/pputils.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Preprocessor utilities.
 *
 * Given an argument list of arbitrary length, @c CXXUTILS_PP_FIRST returns
 * a comma followed by the first argument if it exists; otherwise,
 * it expands to nothing.  @c CXXUTILS_PP_SECOND does the same thing
 * for the second argument, and so on through @CXXUTILS_PP_FIFTH.
 *
 * That is:
 *@code
 *  CXXUTILS_PP_FIRST(a, b, c)  -> , a
 *  CXXUTILS_PP_FIRST()         ->
 *  CXXUTILS_PP_SECOND(a, b, c) -> , b
 *  CXXUTILS_PP_SECOND(a)       ->
 @endcode
 *
 * @c CXXUTILS_PP_ARG1 through @c CXXUTILS_PP_ARG5 are also available
 * as synonyms that may be more useful if these are used through the
 * expansion of other macros.
 */


#ifndef CXXUTILS_PPARGS_H
#define CXXUTILS_PPARGS_H


#define CXXUTILS_PP_FIRST(...) CXXUTILS_PP_ARG1(__VA_ARGS__)
#define CXXUTILS_PP_ARG1(...) __VA_OPT__(CXXUTILS_PP_ARG1_(__VA_ARGS__))
#define CXXUTILS_PP_ARG1_(A, ...) , A

#define CXXUTILS_PP_SECOND(...) CXXUTILS_PP_ARG2(__VA_ARGS__)
#define CXXUTILS_PP_ARG2(...) __VA_OPT__(CXXUTILS_PP_ARG2_(__VA_ARGS__))
#define CXXUTILS_PP_ARG2_(A, ...) CXXUTILS_PP_ARG1(__VA_ARGS__)

#define CXXUTILS_PP_THIRD(...) CXXUTILS_PP_ARG3(__VA_ARGS__)
#define CXXUTILS_PP_ARG3(...) __VA_OPT__(CXXUTILS_PP_ARG3_(__VA_ARGS__))
#define CXXUTILS_PP_ARG3_(A, ...) CXXUTILS_PP_ARG2(__VA_ARGS__)

#define CXXUTILS_PP_FOURTH(...) CXXUTILS_PP_ARG4(__VA_ARGS__)
#define CXXUTILS_PP_ARG4(...) __VA_OPT__(CXXUTILS_PP_ARG4_(__VA_ARGS__))
#define CXXUTILS_PP_ARG4_(A, ...) CXXUTILS_PP_ARG3(__VA_ARGS__)

#define CXXUTILS_PP_FIFTH(...) CXXUTILS_PP_ARG5(__VA_ARGS__)
#define CXXUTILS_PP_ARG5(...) __VA_OPT__(CXXUTILS_PP_ARG5_(__VA_ARGS__))
#define CXXUTILS_PP_ARG5_(A, ...) CXXUTILS_PP_ARG4(__VA_ARGS__)


#endif // not CXXUTILS_PPARGS_H
