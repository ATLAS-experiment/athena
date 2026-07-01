// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file XMLCoreParser/XMLCoreNode.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2026
 * @brief Simple DOM-like node structure to hold the result of XML parsing.
 */


#ifndef XMLCOREPARSER_XMLCORENODE_H
#define XMLCOREPARSER_XMLCORENODE_H


#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <iosfwd>
#include <stdexcept>


/**
 * @brief Class for exceptions thrown from XMLCoreParser.
 */
class ExcXMLCore
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param what Error message.
   */
  ExcXMLCore (const std::string& what);
};


/**
 * @brief Simple DOM-like node structure to hold the result of XML parsing.
 */
class XMLCoreNode
{
public:
  /**
   * @brief Classify node types.
   */
  typedef enum
    {
      DOCUMENT_NODE,
      ELEMENT_NODE,
      COMMENT_NODE,
      TEXT_NODE,
      ENTITY_NODE,
      ENTITY_REFERENCE_NODE
    } NodeType;


  /**
   * @brief Constructor.
   * @param type The node type.
   * @param name Node name (for elements).
   * @apram value Node value (for text/comments).
   */
  XMLCoreNode (NodeType type,
               const std::string& name="",
               const std::string& value="");


  /**
   * @brief Set the value of an attribute for this node.
   * @param name Attribute name.
   * @param value Attribute value (as a string).
   */
  void set_attrib (const std::string& name, const std::string& value);


  /**
   * @brief Add a new child to this node.
   * @param child The child to add.
   *
   * Children are ordered according to the order of @c add_child calls.
   */
  XMLCoreNode* add_child (std::unique_ptr<XMLCoreNode> child);


  /**
   * @brief Get the parent of this node, or nullptr for a top-level node.
   */
  XMLCoreNode* get_parent();


  /**
   * @brief Get the parent of this node, or nullptr for a top-level node.
   */
  const XMLCoreNode* get_parent() const;


  /**
   * @brief Return the type of this node.
   */
  NodeType get_type () const;


  /**
   * @brief Return the name of this node, or an empty string if no name.
   */
  const std::string& get_name () const;


  /**
   * @brief Return the value of this node, or an empty string if no value.
   */
  const std::string& get_value () const;


  /**
   * @brief Return the number of attributes for this node.
   */
  size_t n_attribs() const;


  /**
   * @brief Return the name of the i'th attribute.
   * @param i The index of the attribute.
   *
   * Indexes count attributes in order sorted by name.
   * Throws an exception if the index is out of range.
   */
  std::string get_attrib_name (size_t i) const;


  /**
   * @brief Test for presence of an attribute with a given name.
   * @param name Attribute name to test.
   */
  bool has_attrib (const std::string& name) const;


  /**
   * @brief Try to retrieve an integer attribute.
   * @param name Name of the attribute.
   *
   * Returns an invalid optional if the attribute does not exist or cannot
   * be converted to an integer.
   */
  std::optional<int> try_int_attrib (std::string_view name) const;


  /**
   * @brief Try to retrieve an double attribute.
   * @param name Name of the attribute.
   *
   * Returns an invalid optional if the attribute does not exist or cannot
   * be converted to a double.
   */
  std::optional<double> try_double_attrib (std::string_view name) const;


  /**
   * @brief Retrieve the value of an attribute.
   * @param name Name of the attribute.
   *
   * Throws an exception if the attribute does not exist.
   */
  const std::string& get_attrib (std::string_view name) const;


  /**
   * @brief Retrieve the value of an attribute as an integer.
   * @param name Name of the attribute.
   *
   * Throws an exception if the attribute does not exist or cannot
   * be converted to an int.
   */
  int get_int_attrib (std::string_view name) const;


  /**
   * @brief Retrieve the value of an attribute as a double.
   * @param name Name of the attribute.
   *
   * Throws an exception if the attribute does not exist or cannot
   * be converted to a double.
   */
  double get_double_attrib (std::string_view name) const;


  /**
   * @brief Return the first child matching a pattern, or nullptr.
   * @param path Pattern of the child path for which to search.
   *
   * The @path argument is a set of nested child node names, separated by dots.
   * For example, path node1.node2 means to look for a child of the current
   * node named node1, and then look for a child of that one named node2.
   * A node name may also be * to match any child.
   */
  const XMLCoreNode* get_child (const std::string& path) const;


  /**
   * @brief Return all children matching a pattern.
   * @param path Pattern of the child path for which to search.
   *
   * The @path argument is a set of nested child node names, separated by dots.
   * For example, path node1.node2 means to look for a child of the current
   * node named node1, and then look for a child of that one named node2.
   * A node name may also be * to match any child.
   */
  std::vector<const XMLCoreNode*> get_children (const std::string& path ="*") const;


  /*
   * @brief Print the node structure in XML format (to cout).
   * @param header String to print on hte first line.
   * @param depth Initial nesting depth.
   */
  void print (const std::string& header, int depth = 0) const;


  /*
   * @brief Print the node structure in XML format.
   * @param os Stream to which to print.
   * @param header String to print on hte first line.
   * @param depth Initial nesting depth.
   */
  void print (std::ostream& os, const std::string& header, int depth = 0) const;


private:
  /**
   * @brief Set the parent for this node.
   * @param Pointer to the parent node.
   */
  void set_parent (XMLCoreNode* parent);


  /**
   * @brief Try to retrieve an attribute of type @c T.
   * param name Name of the attribute to retrieve.
   *
   * Returns an invalid @c optional if the attribute does not exist
   * or cannot be converted to @c T.
   */
  template <class T>
  std::optional<T> try_attrib (std::string_view name) const;


  /**
   * @brief Collect children of this node matching a path.
   * @param path Pattern of the child path for which to search.
   * @param children[out] Vector in which to collect matching children.
   * @param only_one If true, stop after finding the first child.
   *
   * See @c get_child above for a description of @c path.
   */
  void collect_children (const std::string& path,
                         std::vector<const XMLCoreNode*>& children,
                         bool only_one) const;


  /// The parent of this node, or null for a top-level node.
  XMLCoreNode* m_parent = nullptr;

  /// The type of this node.
  NodeType m_type;

  /// The name of this node.
  std::string m_name;

  /// The value fo this node.
  std::string m_value;

  /// Attributes of this node.
  std::map<std::string, std::string, std::less<>> m_attribs;

  /// Children of this node.
  std::vector<std::unique_ptr<XMLCoreNode> > m_children;
};


#endif // not XMLCOREPARSER_XMLCORENODE_H
