/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_SIGN_ENUMS_H
#define EVENT_SELECTOR_SIGN_ENUMS_H


/// \brief the conversion key for comparison operators for Event Selection Algorithms

namespace SignEnum {

  /// \brief all possible comparison user inputs
  enum ComparisonOperator {
    LT,
    GT,
    EQ,
    GE,
    LE
  };
  
  /// \brief the map between user inputs and comparison operators
  static const std::map<std::string, ComparisonOperator> stringToOperator = {
    {"LT", ComparisonOperator::LT}, // <
    {"GT", ComparisonOperator::GT}, // >
    {"EQ", ComparisonOperator::EQ}, // ==
    {"GE", ComparisonOperator::GE}, // >=
    {"LE", ComparisonOperator::LE}  // <=
  };

  /// \brief the comparison test given the specified sign and two test values
  template <typename T>
  bool checkValue(T reference, ComparisonOperator sign, T test) {
    switch (sign) {
    case ComparisonOperator::LT:
      return test < reference;
    case ComparisonOperator::GT:
      return test > reference;
    case ComparisonOperator::EQ:
      return test == reference;
    case ComparisonOperator::GE:
      return test >= reference;
    case ComparisonOperator::LE:
      return test <= reference;
    }
    throw std::runtime_error("SignEnum::checkValue did not recognise the sign argument! Make sure it is listed within SignEnum::ComparisonOperator.");
  }

} // namespace SignEnum

#endif // EVENT_SELECTOR_SIGN_ENUMS_H
