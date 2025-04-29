/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

#include "Identifier/IdHelper.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/IdContext.h"
#include <string>
class IMessageSvc;
//dummy definition
class IdDictMgr {};


class IdHelperStub:public IdHelper{
public:
    // Create compact id from hash id (return == 0 for OK)
    int get_id(const IdentifierHash& , Identifier& , const IdContext* = nullptr) const override{
      return 0;
    }
    // Create hash id from compact id (return == 0 for OK)
    int get_hash(const Identifier&, IdentifierHash& ,const IdContext* = nullptr) const override{
      return 0;
    }
    // Initialization from the identifier dictionary
    int initialize_from_dictionary(const IdDictMgr& ) override {
      return 0;
    }
    // retrieve version of the dictionary
    std::string dictionaryVersion() const override {
      return m_version;
    }
    //
    bool	do_checks() const override{
      return m_checks;
    }
    void set_do_checks(bool do_checks) override{
      m_checks = do_checks;
    }
    /// Neighbour initialization is performed by default
    bool do_neighbours() const override{
      return m_doNeighbours;
    }
    void set_do_neighbours(bool do_neighbours) override{
      m_doNeighbours = do_neighbours;
    }
    // setting pointer to the MessageSvc
    void setMessageSvc(IMessageSvc* ) override {
    //nop
    }
    //
    void setDictVersion(const IdDictMgr& , const std::string& name) override{
      m_version = name;
    }
private:
  std::string m_version{"default"};
  bool m_checks{};
  bool m_doNeighbours{true};
};


BOOST_AUTO_TEST_SUITE(IdHelperTest)

BOOST_AUTO_TEST_CASE(IdHelperConstructors){
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdHelperStub a);
}

BOOST_AUTO_TEST_CASE(IdHelperAccessors){
  IdHelperStub a;
  IdentifierHash idHash;
  Identifier id;
  BOOST_TEST(a.get_id(idHash, id) == 0);
  BOOST_TEST(a.get_hash(id, idHash) == 0);
  BOOST_TEST(a.dictionaryVersion() == "default");
  BOOST_TEST(a.do_checks() == false);
  BOOST_TEST(a.do_neighbours() == true);
}

BOOST_AUTO_TEST_CASE(IdHelperModifiers){
  IdHelperStub a;
  //
  BOOST_CHECK_NO_THROW(a.set_do_checks(true));
  BOOST_TEST(a.do_checks() == true);
  //
  BOOST_CHECK_NO_THROW(a.set_do_neighbours(false));
  BOOST_TEST(a.do_neighbours() == false);
  //
  IdDictMgr dictMgr{};
  BOOST_CHECK_NO_THROW(a.setDictVersion(dictMgr, "not default"));
  BOOST_TEST(a.dictionaryVersion() == "not default");
}


BOOST_AUTO_TEST_SUITE_END()


