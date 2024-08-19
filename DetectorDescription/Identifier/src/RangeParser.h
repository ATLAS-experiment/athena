/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Identifier_RangeParser_h
#define Identifier_RangeParser_h
#include <string>
#include "Identifier/Range.h" //for typedef
class RangeParserFriend;

namespace Identifier{
  
  class RangeParser { 
  friend RangeParserFriend;//for testing
  public: 
    typedef Range::size_type size_type; 
    RangeParser () = default;
    bool run(Range& range, const std::string& text);
   
  private:
    /**
     * @brief Give position of first non-space character
     * @param text Text to operate on
     * @param pos[in/out] The position of the first non-space character
     * function returns false if there are none, true otherwise
    **/
    bool skip_spaces (const std::string& text, size_type& pos);
    /**
     * @brief Test character at position pos for match with token
     * @param text Text to operate on
     * @param pos[in/out] Position of char to look at; is incremented if token is found
     * @param token Character to do match
     * Function returns false if there is no match, true otherwise.
     * Leading spaces are skipped.
    **/
    bool test_token (const std::string& text, size_type& pos, char token);
    /**
     * @brief Looks for numerical value, returned in 'value'
     * @param text Text to operate on
     * @param pos[in/out] Position in string to look at; incremented beyond the number, if found
     * @param value[out] Value returned, 0 in case no number is found
     * Function returns false if there is no number, true otherwise.
    **/
    bool parse_number (const std::string& text, size_type& pos, int& value);
    
    bool parse_maximum (Range::field& field, const std::string& text, size_type& pos); 
    bool parse_list (Range::field& field, const std::string& text, size_type& pos);
    bool parse_field (Range::field& field, const std::string& text, size_type& pos);
    bool parse (Range& range, const std::string& text, size_type& pos);
  }; 
}

#endif