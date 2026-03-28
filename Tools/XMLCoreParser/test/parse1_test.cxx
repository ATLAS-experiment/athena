/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author sss
 * @date Mar 2026
 * @brief Test XML parsing.
 */


#undef NDEBUG
#include "XMLCoreParser/XMLCoreParser.h"
#include "XMLCoreParser/XMLCoreNode.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>


const char* const xml_in =
  "<elt1 att1=\"123\" att2=\"foo\" att3=\"4.5\">\n"
  "  <elt2 att=\"foobar\">\n"
  "    Some text here\n"
  "    <elt3 name=\"elt3a\"/>\n"
  "    Some more text here\n"
  "    <elt3 name=\"elt3b\"/>\n"
  "    <!-- And a comment -->\n"
  "    <elt3 name=\"elt3c\"/>\n"
  "  </elt2>\n"
  "</elt1>\n"
  ;


void test1()
{
  std::cout << "test1\n";

  XMLCoreParser p;
  std::unique_ptr<XMLCoreNode> doc = p.parse_string (xml_in);
  doc->print ("test1");

  const XMLCoreNode* elt1 = doc->get_child ("elt1");
  assert (elt1->get_type() == XMLCoreNode::ELEMENT_NODE);
  assert (elt1->get_name() == "elt1");
  assert (elt1->get_value() == "");
  assert (elt1->n_attribs() == 3);

  assert (elt1->has_attrib ("att1"));
  assert (!elt1->has_attrib ("attx"));
  assert (elt1->get_attrib ("att1") == "123");
  EXPECT_EXCEPTION (ExcXMLCore, elt1->get_attrib ("att1x"));

  assert (elt1->try_int_attrib ("att1").value() == 123);
  assert (!elt1->try_int_attrib ("att2"));
  assert (!elt1->try_int_attrib ("attx"));
  assert (elt1->try_double_attrib ("att3").value() == 4.5);
  assert (!elt1->try_double_attrib ("att2"));
  assert (!elt1->try_double_attrib ("attx"));

  assert (elt1->get_int_attrib ("att1") == 123);
  EXPECT_EXCEPTION (ExcXMLCore, elt1->get_int_attrib ("att2"));
  EXPECT_EXCEPTION (ExcXMLCore, elt1->get_int_attrib ("attx"));
  assert (elt1->get_double_attrib ("att3") == 4.5);
  EXPECT_EXCEPTION (ExcXMLCore, elt1->get_double_attrib ("att2"));
  EXPECT_EXCEPTION (ExcXMLCore, elt1->get_double_attrib ("attx"));

  const XMLCoreNode* elt2 = doc->get_child ("elt1/elt2");
  assert (elt2->get_name() == "elt2");
  assert (elt2->get_attrib ("att") == "foobar");
  assert (elt2->get_value() == "");

  assert (doc->get_child ("elt1/eltx") == nullptr);

  const XMLCoreNode* elt3 = doc->get_child ("elt1/elt2/elt3");
  assert (elt3->get_name() == "elt3");
  assert (elt3->get_attrib ("name") == "elt3a");
  assert (elt3->get_value() == "");

  std::vector<const XMLCoreNode*> elt2s = doc->get_children ("elt1/elt2");
  assert (elt2s.size() == 1);
  assert (elt2s[0] == elt2);

  std::vector<const XMLCoreNode*> elt3s = doc->get_children ("elt1/*/elt3");
  assert (elt3s.size() == 3);
  assert (elt3s[0] == elt3);
  assert (elt3s[1]->get_attrib ("name") == "elt3b");
  assert (elt3s[2]->get_attrib ("name") == "elt3c");
}

//coverity[UNCAUGHT_EXCEPT:FALSE]
int main()
{
  std::cout << "XMLCoreParser/parse1_test\n";
  test1();
  return 0;
}
