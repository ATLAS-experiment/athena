/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file XMLCoreParser/src/XMLCoreNode.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2026
 * @brief Simple DOM-like node structure to hold the result of XML parsing.
 */


#include "XMLCoreParser/XMLCoreNode.h"
#include <charconv>
#include <iostream>


static const char* const SPACE = " \t\r\n";


/**
 * @brief Constructor for exception class.
 * @param what Error message.
 */
ExcXMLCore::ExcXMLCore (const std::string& what)
  : std::runtime_error ("ExcXMLCore: " + what)
{
}


/**
 * @brief Constructor.
 * @param type The node type.
 * @param name Node name (for elements).
 * @apram value Node value (for text/comments).
 */
XMLCoreNode::XMLCoreNode (NodeType type,
                          const std::string& name,
                          const std::string& value)
  : m_type (type),
    m_name (name),
    m_value (value)
{
}


/**
 * @brief Set the value of an attribute for this node.
 * @param name Attribute name.
 * @param value Attribute value (as a string).
 */
void XMLCoreNode::set_attrib (const std::string& name, const std::string& value)
{
  m_attribs[name] = value;
}


/**
 * @brief Add a new child to this node.
 * @param child The child to add.
 *
 * Children are ordered according to the order of @c add_child calls.
 */
XMLCoreNode* XMLCoreNode::add_child (std::unique_ptr<XMLCoreNode> child)
{
  child->set_parent (this);
  m_children.push_back (std::move (child));
  return m_children.back().get();
}


/**
 * @brief Get the parent of this node, or nullptr for a top-level node.
 */
XMLCoreNode* XMLCoreNode::get_parent()
{
  return m_parent;
}


/**
 * @brief Get the parent of this node, or nullptr for a top-level node.
 */
const XMLCoreNode* XMLCoreNode::get_parent() const
{
  return m_parent;
}


/**
 * @brief Return the type of this node.
 */
XMLCoreNode::NodeType XMLCoreNode::get_type() const
{
  return m_type;
}


/**
 * @brief Return the name of this node, or an empty string if no name.
 */
const std::string& XMLCoreNode::get_name() const
{
  return m_name;
}


/**
 * @brief Return the value of this node, or an empty string if no value.
 */
const std::string& XMLCoreNode::get_value() const
{
  return m_value;
}


/**
 * @brief Return the number of attributes for this node.
 */
size_t XMLCoreNode::n_attribs() const
{
  return m_attribs.size();
}


/**
 * @brief Return the name of the i'th attribute.
 * @param i The index of the attribute.
 *
 * Indexes count attributes in order sorted by name.
 * Throws an exception if the index is out of range.
 */
std::string XMLCoreNode::get_attrib_name (size_t i) const
{
  if (i >= m_attribs.size()) {
    throw ExcXMLCore ("Out-of-range attribute index " + std::to_string(i) + " in " + m_name);
  }

  auto it = m_attribs.begin();
  std::advance (it, i);
  return it->first;
}


/**
 * @brief Test for presence of an attribute with a given name.
 * @param name Attribute name to test.
 */
bool XMLCoreNode::has_attrib (const std::string& name) const
{
  return m_attribs.contains (name);
}


/**
 * @brief Try to retrieve an attribute of type @c T.
 * param name Name of the attribute to retrieve.
 *
 * Returns an invalid @c optional if the attribute does not exist
 * or cannot be converted to @c T.
 */
template <class T>
std::optional<T> XMLCoreNode::try_attrib (const std::string& name) const
{
  std::optional<T> ret;
  auto it = m_attribs.find (name);
  if (it == m_attribs.end()) return ret;

  T val = 0;
  std::string::size_type beg = it->second.find_first_not_of (SPACE);
  std::string::size_type end = it->second.find_last_not_of (SPACE);
  if (beg == std::string::npos || end == std::string::npos) return ret;
  const char* begp = it->second.c_str()+beg;
  const char* endp = it->second.c_str()+end+1;
  auto [ptr, ec] = std::from_chars (begp, endp, val);
  if (ec == std::errc() && ptr == endp)
    ret.emplace (val);
  return ret;
}


/**
 * @brief Try to retrieve an integer attribute.
 * @param name Name of the attribute.
 *
 * Returns an invalid optional if the attribute does not exist or cannot
 * be converted to an integer.
 */
std::optional<int> XMLCoreNode::try_int_attrib (const std::string& name) const
{
  return try_attrib<int> (name);
}


/**
 * @brief Try to retrieve an double attribute.
 * @param name Name of the attribute.
 *
 * Returns an invalid optional if the attribute does not exist or cannot
 * be converted to a double.
 */
std::optional<double> XMLCoreNode::try_double_attrib (const std::string& name) const
{
  return try_attrib<double> (name);
}


/**
 * @brief Retrieve the value of an attribute.
 * @param name Name of the attribute.
 *
 * Throws an exception if the attribute does not exist.
 */
const std::string& XMLCoreNode::get_attrib (const std::string& name) const
{
  auto it = m_attribs.find (name);
  if (it == m_attribs.end()) {
    throw ExcXMLCore ("Cannot find attribute " + name + " in " + m_name);
  }
  return it->second;
}


/**
 * @brief Retrieve the value of an attribute as an integer.
 * @param name Name of the attribute.
 *
 * Throws an exception if the attribute does not exist or cannot
 * be converted to an int.
 */
int XMLCoreNode::get_int_attrib (const std::string& name) const
{
  std::optional<int> val = try_int_attrib (name);
  if (!val) {
    throw ExcXMLCore ("Bad integer attribute " + name + " in " + m_name);
  }
  return val.value();
}


/**
 * @brief Retrieve the value of an attribute as a double.
 * @param name Name of the attribute.
 *
 * Throws an exception if the attribute does not exist or cannot
 * be converted to a double.
 */
double XMLCoreNode::get_double_attrib (const std::string& name) const
{
  std::optional<double> val = try_double_attrib (name);
  if (!val) {
    throw ExcXMLCore ("Bad double attribute " + name + " in " + m_name);
  }
  return val.value();
}


/**
 * @brief Return the first child matching a pattern, or nullptr.
 * @param path Pattern of the child path for which to search.
 *
 * The @path argument is a set of nested child node names, separated by slashes.
 * For example, path node1/node2 means to look for a child of the current
 * node named node1, and then look for a child of that one named node2.
 * A node name may also be * to match any child.
 */
const XMLCoreNode* XMLCoreNode::get_child (const std::string& path) const
{
  std::vector<const XMLCoreNode*> children;
  collect_children (path, children, true);
  if (!children.empty()) {
    return children[0];
  }
  return nullptr;
}


/**
 * @brief Return all children matching a pattern.
 * @param path Pattern of the child path for which to search.
 *
 * For example, path node1/node2 means to look for a child of the current
 * node named node1, and then look for a child of that one named node2.
 * node named node1, and then look for a child of that one named node2.
 * A node name may also be * to match any child.
 */
std::vector<const XMLCoreNode*>
XMLCoreNode::get_children (const std::string& path /*= "*"*/) const
{
  std::vector<const XMLCoreNode*> children;
  collect_children (path, children, false);
  return children;
}


/*
 * @brief Print the node structure in XML format (to cout).
 * @param header String to print on hte first line.
 * @param depth Initial nesting depth.
 */
void XMLCoreNode::print (const std::string& header,
                         int depth /*= 0*/) const
{
  print (std::cout, header, depth);
}


/*
 * @brief Print the node structure in XML format.
 * @param os Stream to which to print.
 * @param header String to print on hte first line.
 * @param depth Initial nesting depth.
 */
void XMLCoreNode::print (std::ostream& os,
                         const std::string& header,
                         int depth /*= 0*/) const
{
  if (!header.empty()) {
    os << header << std::endl;
  }

  if (m_type == TEXT_NODE) {
    bool allspace = m_value.find_last_of(SPACE) == std::string::npos;
    if (!allspace)
      os << m_value << std::endl;
    return;
  }

  for (int i = 0; i < depth; i++) os << "  ";

  if (m_type == COMMENT_NODE) {
    os << "<!--" << m_value << "-->" << std::endl;
    return;
  }

  os << "<" << m_name;

  for (const auto& p : m_attribs) {
    os << " " << p.first << "='" << p.second << "'";
  }

  os << ">" << std::endl;

  for (const auto& child : m_children) {
    child->print (os, "", depth+1);
  }

  for (int i = 0; i < depth; i++) os << "  ";
  os << "</" << m_name << ">" << std::endl;
  return;
}


/**
 * @brief Set the parent for this node.
 * @param Pointer to the parent node.
 */
void XMLCoreNode::set_parent (XMLCoreNode* parent)
{
  m_parent = parent;
}


/**
 * @brief Collect children of this node matching a path.
 * @param path Pattern of the child path for which to search.
 * @param children[out] Vector in which to collect matching children.
 * @param only_one If true, stop after finding the first child.
 *
 * See @c get_child above for a description of @c path.
 */
void XMLCoreNode::collect_children (const std::string& path,
                                    std::vector<const XMLCoreNode*>& children,
                                    bool only_one) const
{
  std::string::size_type pos = path.find ('/');
  std::string name = path.substr (0, pos);
  std::string tail;
  if  (pos != std::string::npos) {
    tail = path.substr (pos+1);
  }

  for (const auto& child : m_children) {
    if (name == "*" || child->get_name() == name) {
      if (tail.empty()) {
        children.push_back (child.get());
      }
      else {
        child->collect_children (tail, children, only_one);
      }
    }
    if (only_one && !children.empty()) break;
  }
}


