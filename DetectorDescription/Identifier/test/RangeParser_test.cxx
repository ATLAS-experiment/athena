// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "src/RangeParser.h"
#include "Identifier/Range.h"
#include <string>

using Identifier::RangeParser;

class RangeParserFriend{
  public: 
    typedef Range::size_type size_type; 
    RangeParserFriend () = default;
    bool run(Range& range, const std::string& text){ return m_rp.run(range,text); }
    bool skip_spaces (const std::string& text, size_type& pos){ return m_rp.skip_spaces(text, pos);}
    bool test_token (const std::string& text, size_type& pos, char token){ return m_rp.test_token(text, pos, token);}
    bool parse_number (const std::string& text, size_type& pos, int& value){ return m_rp.parse_number(text, pos, value);}
    bool parse_maximum (Range::field& field, const std::string& text, size_type& pos){ return m_rp.parse_maximum(field, text, pos);}
    bool parse_list (Range::field& field, const std::string& text, size_type& pos){return m_rp.parse_list(field, text, pos);}
    bool parse_field (Range::field& field, const std::string& text, size_type& pos){return m_rp.parse_field(field, text, pos);}
  private:
    RangeParser m_rp;
  }; 

//nonsense cases
const std::string empty{};
const std::string spaces("     ");
//random text with leading space
const std::string txt("  gt566 hh ");

BOOST_AUTO_TEST_SUITE(RangeParserTest)
BOOST_AUTO_TEST_CASE(RangeParserConstructor){
  BOOST_CHECK_NO_THROW([[maybe_unused]] RangeParser rp);
  BOOST_CHECK_NO_THROW([[maybe_unused]] RangeParserFriend rpf);
}

BOOST_AUTO_TEST_CASE(RangeParser_skip_spaces){
  RangeParserFriend rpf;
  size_t pos{};
  //has non-space text in it
  BOOST_TEST(rpf.skip_spaces(txt, pos) == true); 
  BOOST_TEST(pos == 2);
  //is empty
  BOOST_TEST(rpf.skip_spaces(empty, pos) ==  false); 
  BOOST_TEST(pos == std::string::npos);
  //only has spaces
  BOOST_TEST(rpf.skip_spaces(spaces, pos) ==  false);
  BOOST_TEST(pos == std::string::npos);
  //looking beyond the end of the string
  size_t tooBig{100};
  BOOST_TEST(rpf.skip_spaces(txt, tooBig) == false); 
  BOOST_TEST(tooBig == std::string::npos);
}

BOOST_AUTO_TEST_CASE(RangeParser_test_token){
  RangeParserFriend rpf;
  size_t pos{};
  char token{'t'};
  //empty string can't have 't' in it
  BOOST_TEST(rpf.test_token(empty, pos, token) == false);
  BOOST_TEST(pos == std::string::npos);
  //all spaces string can't have 't' in it
  BOOST_TEST(rpf.test_token(spaces, pos, token) == false);
  BOOST_TEST(pos == std::string::npos);
  //look for 't' in the wrong position
  pos=5;
  BOOST_TEST(rpf.test_token(txt, pos, token) == false);
  BOOST_TEST(pos == 5);
  //found a 't' at the right position, increments pos
  pos=3;
  BOOST_TEST(rpf.test_token(txt, pos, token) == true);
  BOOST_TEST(pos == 4);
  //look for 't' beyond the end of the string
  size_t tooBig{100};
  BOOST_TEST(rpf.test_token(txt, tooBig, token) == false);
  BOOST_TEST(tooBig == std::string::npos);
}

BOOST_AUTO_TEST_CASE(RangeParser_parse_number){
  RangeParserFriend rpf;
  size_t pos{4};
  int value{};//to be returned
  //search for integer number at the right position
  BOOST_TEST(rpf.parse_number(txt, pos, value) == true);
  BOOST_TEST(pos == 8);//incremented past number
  BOOST_TEST(value == 566);
  //search for integer number at the wrong position
  pos=3;
  value=-5;
  BOOST_TEST(rpf.parse_number(txt, pos, value) == false);
  BOOST_TEST(pos == 3);//unaltered
  BOOST_TEST(value == 0);//set to zero (!??)
}

BOOST_AUTO_TEST_CASE(RangeParser_parse_maximum){
  RangeParserFriend rpf;
  size_t pos{0};
  Range::field f;//to be returned
  const std::string bounded(" 2:10 ff");
  //search for integer number at the right position
  BOOST_TEST(rpf.parse_maximum(f, bounded, pos) == true);
  BOOST_TEST(f.get_maximum() == 2);
  BOOST_TEST(pos == 2);
  //search for integer number at the wrong position
  pos=5;
  f = Range::field();
  BOOST_TEST(rpf.parse_maximum(f, bounded, pos) == false);
  BOOST_TEST(f.get_maximum() == 0);//dangerous
  BOOST_TEST(pos == 6);//why: first non-whitespace after position 5.
}

BOOST_AUTO_TEST_CASE(RangeParser_parse_list){
  RangeParserFriend rpf;
  size_t pos{0};
  Range::field f;//to be returned
  const std::string enumerated("1, 2, 3, 6, 7, 8 ghghghghg");
  //search for integer number at the right position
  BOOST_TEST(rpf.parse_list(f, enumerated, pos) == true);
  BOOST_TEST(f.get_maximum() == 8);
  BOOST_TEST(pos == 17);
  //search for integer number at the wrong position
  pos=18;
  f = Range::field();
  BOOST_TEST(rpf.parse_list(f, enumerated, pos) == true);
  BOOST_TEST(f.get_maximum() == 0);//dangerous
  BOOST_TEST(pos == 18);
  const std::string exampleLar("-1,0,1,3,19,255");
  pos=0;
  BOOST_TEST(rpf.parse_list(f, exampleLar, pos) == true);
  BOOST_TEST(pos == std::string::npos);//??
}

BOOST_AUTO_TEST_CASE(RangeParser_run){
  const std::string larExample="4/1/-1,1/3/0/0:19/0:255";
  Range r;
  RangeParserFriend rpf;
  BOOST_TEST(rpf.run(r, larExample) == true);
  BOOST_TEST(r[2].get_mode() = Range::field::mode::enumerated);
  BOOST_TEST(r[5].get_mode() = Range::field::mode::both_bounded);
  BOOST_TEST(r.fields() = 7);
}



BOOST_AUTO_TEST_SUITE_END()