/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author sss
 * @date Mar 2026
 * @brief Unit test for XMLCoreNode
 */


#undef NDEBUG
#include "XMLCoreParser/XMLCoreNode.h"
#include "TestTools/expect_exception.h"
#include <sstream>
#include <iostream>
#include <cassert>


void test1()
{
  std::cout << "test1\n";

  XMLCoreNode node1 (XMLCoreNode::ELEMENT_NODE, "node1");
  node1.set_attrib ("att1", "abc");
  node1.set_attrib ("att2", "123");
  node1.set_attrib ("att3", "4.5");
  node1.set_attrib ("att4", "0");

  {
    auto node2 = std::make_unique<XMLCoreNode> (XMLCoreNode::ELEMENT_NODE, "node2");
    node2->set_attrib ("att", "foobar");

    {
      auto node3a = std::make_unique<XMLCoreNode> (XMLCoreNode::ELEMENT_NODE, "node3");
      node3a->set_attrib ("name", "node3a");
      node2->add_child (std::move (node3a));
    }
    node2->add_child (std::make_unique<XMLCoreNode> (XMLCoreNode::TEXT_NODE, "", "Some text"));
    {
      auto node3b = std::make_unique<XMLCoreNode> (XMLCoreNode::ELEMENT_NODE, "node3");
      node3b->set_attrib ("name", "node3b");
      node2->add_child (std::move (node3b));
    }
    node2->add_child (std::make_unique<XMLCoreNode> (XMLCoreNode::COMMENT_NODE, "", "Some comment"));
    {
      auto node3c = std::make_unique<XMLCoreNode> (XMLCoreNode::ELEMENT_NODE, "node3");
      node3c->set_attrib ("name", "node3c");
      node2->add_child (std::move (node3c));
    }

    node1.add_child (std::move (node2));
  }

  assert (node1.get_parent() == nullptr);
  assert (node1.get_type() == XMLCoreNode::ELEMENT_NODE);
  assert (node1.get_name() == "node1");
  assert (node1.get_value() == "");

  assert (node1.n_attribs() == 4);
  assert (node1.get_attrib_name(0) == "att1");
  assert (node1.get_attrib_name(1) == "att2");
  assert (node1.get_attrib_name(2) == "att3");
  assert (node1.get_attrib_name(3) == "att4");
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_attrib_name(4));

  assert (node1.has_attrib ("att1"));
  assert (!node1.has_attrib ("attx"));

  assert (node1.try_int_attrib("att2").value() == 123);
  assert (!node1.try_int_attrib("att1"));
  assert (!node1.try_int_attrib("att3"));
  assert (!node1.try_int_attrib("attx"));

  assert (node1.try_double_attrib("att3").value() == 4.5);
  assert (node1.try_double_attrib("att2").value() == 123);
  assert (!node1.try_double_attrib("att1"));
  assert (!node1.try_double_attrib("attx"));

  assert (node1.get_attrib("att1") == "abc");
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_attrib("attx"));

  assert (node1.get_int_attrib("att2") == 123);
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_int_attrib("att1"));
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_int_attrib("att3"));
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_int_attrib("attx"));

  assert (node1.get_double_attrib("att3") == 4.5);
  assert (node1.get_double_attrib("att2") == 123);
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_double_attrib("att1"));
  EXPECT_EXCEPTION(ExcXMLCore, node1.get_double_attrib("attx"));

  assert (node1.get_int_attrib("att4") == 0);

  assert (node1.get_child("xxx") == nullptr);
  const XMLCoreNode* node2 = node1.get_child ("node2");
  assert (node2->get_name() == "node2");
  assert (node2->get_parent() == &node1);

  const XMLCoreNode* node3 = node1.get_child ("*/node3");
  assert (node3->get_name() == "node3");
  assert (node3->get_parent() == node2);
  assert (node3->get_attrib("name") == "node3a");

  std::vector<const XMLCoreNode*> children1 = node1.get_children ("*/node3");
  assert (children1.size() == 3);
  assert (children1[0]->get_name() == "node3");
  assert (children1[0]->get_parent() == node2);
  assert (children1[0]->get_attrib("name") == "node3a");

  assert (children1[1]->get_name() == "node3");
  assert (children1[1]->get_parent() == node2);
  assert (children1[1]->get_attrib("name") == "node3b");

  assert (children1[2]->get_name() == "node3");
  assert (children1[2]->get_parent() == node2);
  assert (children1[2]->get_attrib("name") == "node3c");

  std::vector<const XMLCoreNode*> children2 = node1.get_children ("*/*");
  assert (children2.size() == 5);
  assert (children2[0]->get_attrib("name") == "node3a");
  assert (children2[2]->get_attrib("name") == "node3b");
  assert (children2[4]->get_attrib("name") == "node3c");
  assert (children2[1]->get_type() == XMLCoreNode::TEXT_NODE);
  assert (children2[1]->get_value() == "Some text");
  assert (children2[3]->get_type() == XMLCoreNode::COMMENT_NODE);
  assert (children2[3]->get_value() == "Some comment");

  std::ostringstream ss;
  node1.print (ss, "header");
  const char* exp_print =
    "header\n"
    "<node1 att1='abc' att2='123' att3='4.5' att4='0'>\n"
    "  <node2 att='foobar'>\n"
    "    <node3 name='node3a'>\n"
    "    </node3>\n"
    "Some text\n"
    "    <node3 name='node3b'>\n"
    "    </node3>\n"
    "    <!--Some comment-->\n"
    "    <node3 name='node3c'>\n"
    "    </node3>\n"
    "  </node2>\n"
    "</node1>\n";
  assert (ss.str() == exp_print);
}


int main()
{
  std::cout << "XMLCoreParser/XMLCoreNode_test\n";
  test1();
  return 0;
}
