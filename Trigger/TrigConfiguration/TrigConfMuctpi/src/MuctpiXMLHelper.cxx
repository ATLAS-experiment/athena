/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfMuctpi/MuctpiXMLHelper.h"
#include <charconv>
#include <string_view>
#include <iostream>

using namespace std;
using boost::property_tree::ptree;

MuctpiXMLHelper::MuctpiXMLHelper() :
  TrigConf::TrigConfMessaging("MuctpiXML")
{}

void
MuctpiXMLHelper::printAttributes(const ptree & tree) {
  
   // initialize attributes ptree
   ptree tmp_ptree;
   ptree attributes = tree.get_child("<xmlattr>", tmp_ptree);
  
   // iterate through elements
   for(const ptree::value_type & a: attributes) {
      string attrName = a.first;
      string attrVal = a.second.data();
      std::cout << attrName << " : " << attrVal << std::endl;
   }
}

bool
MuctpiXMLHelper::hasAttribute(const ptree & tree, const string & attr) {

   // initialize attributes ptree 
   ptree tmp_ptree;
   ptree attributes = tree.get_child("<xmlattr>", tmp_ptree);
  
   if(attributes.empty()) return false;
  
   for(const ptree::value_type &a: attributes) {
      string attrName = a.first;
      if(attrName == attr) {
         return true;
      }
   }
   return false;
}


string
MuctpiXMLHelper::readAttribute(const ptree & tree, const string & attr) {
 
      // initialize attributes ptree 
      ptree tmp_ptree;
      ptree attributes = tree.get_child("<xmlattr>", tmp_ptree);
  
      // iterate through children
      for(const ptree::value_type &a : attributes) {
         if(a.first != attr) continue;
         return a.second.data(); //a.second.data is the value!
      }
      return "";
}


std::string
MuctpiXMLHelper::getAttribute(const ptree & tree, const string & attr) {
   if( ! hasAttribute(tree, attr) ) {
      TRG_MSG_WARNING("attribute " << attr << " does not exist");
      return "";
   }
   return readAttribute(tree,attr);
}

std::string
MuctpiXMLHelper::getAttribute(const ptree & tree, const string & attr, const std::string & defval) {
   if( ! hasAttribute(tree, attr) )
      return defval;
   return readAttribute(tree,attr);
}


int
MuctpiXMLHelper::getIntAttribute(const ptree & tree, const string & attr) {
   if( ! hasAttribute(tree, attr) ) {
      TRG_MSG_WARNING("attribute " << attr << " does not exist");
      return 0;
   }

   int ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not an int (it is '" << attr_value << "')");
   }
   return ret_value;
}

int
MuctpiXMLHelper::getIntAttribute(const ptree & tree, const string & attr, int defval) {
   if( ! hasAttribute(tree, attr) )
      return defval;
   int ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not an int (it is '" << attr_value << "')");
   }
   return ret_value;
}


unsigned int
MuctpiXMLHelper::getUIntAttribute(const ptree & tree, const string & attr) {
   if( ! hasAttribute(tree, attr) ) {
      TRG_MSG_WARNING("attribute " << attr << " does not exist");
      return 0;
   }
   unsigned int ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not an unsigned int (it is " << attr_value << ")");
   }
   return ret_value;
}

unsigned int
MuctpiXMLHelper::getUIntAttribute(const ptree & tree, const string & attr, unsigned int & defval) {
   if( ! hasAttribute(tree, attr) )
      return defval;
   unsigned int ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not an unsigned int (it is " << attr_value << ")");
   }
   return ret_value;
}


float
MuctpiXMLHelper::getFloatAttribute(const ptree & tree, const string & attr) {
   if( ! hasAttribute(tree, attr) ) {
      TRG_MSG_WARNING("attribute " << attr << " does not exist");
      return 0;
   }
   float ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value, std::chars_format::general);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not a float (it is " << attr_value << ")");
     printAttributes(tree);
   }
   return ret_value;
}

float
MuctpiXMLHelper::getFloatAttribute(const ptree & tree, const string & attr, float & defval) {
   if( ! hasAttribute(tree, attr) )
      return defval;
   float ret_value{0};
   std::string attr_value = readAttribute(tree, attr);
   auto [ptr, ec] = std::from_chars(attr_value.data(), attr_value.data() + attr_value.size(), ret_value, std::chars_format::general);
   if (ec != std::errc()) {
     TRG_MSG_ERROR("attribute '" << attr << "' is not a float (it is " << attr_value << ")");
     printAttributes(tree);
   }
   return ret_value;
}
