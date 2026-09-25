/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "AthLinks/AssociationMap.h"
#include "SGTools/CurrentEventStore.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <print>
#include <vector>


#include "SGTools/TestStore.h"
using namespace SGTest;


class Cluster
{
public:
  Cluster() : m_e(0.) { };
  Cluster(double e) : m_e(e) { };
  ~Cluster() { };
  double getE() const { return m_e; }
private:
  double m_e;
};

template <class T>
struct DV : public std::vector<T*>
{
  typedef T base_value_type;
};

class ClusterContainer : public DV< Cluster >
{
public:
  ClusterContainer() : DV< Cluster >() { };
  virtual ~ClusterContainer() { };
};

CLASS_DEF( ClusterContainer, 12345, 1)

class Track
{
public:
  Track() : m_p(0.) { };
  Track(double p) : m_p(p) { };
  ~Track() { };
  double getP() const { return m_p; }
private:
  double m_p;
};

class TrackContainer : public DV< Track >
{
public:
  TrackContainer() : DV< Track >() { };
  virtual ~TrackContainer() { };
};

CLASS_DEF( TrackContainer, 54321, 1 )

class PTAss
  : public AssociationMap< ClusterContainer, TrackContainer >
{
public:

  PTAss() : AssociationMap< ClusterContainer, TrackContainer >() { }
  virtual ~PTAss() { }
};

CLASS_DEF( PTAss , 94601450 , 1 )

class TTAss : public AssociationMap<TrackContainer,TrackContainer>
{
public:
  TTAss() : AssociationMap< TrackContainer, TrackContainer >() { }
  virtual ~TTAss() { }
};

CLASS_DEF( TTAss, 67890, 1 )


void test1 (SGTest::TestStore& store)
{
  std::println (" *** AssociationMap test in progress: ");

  std::println ("Build fake data and associations:");

  ClusterContainer* cCont = new ClusterContainer();
  cCont->push_back(new Cluster(1.));
  cCont->push_back(new Cluster(2.));
  cCont->push_back(new Cluster(3.));
  std::println ("Cluster Container...............: {} clusters",
                cCont->size());

  TrackContainer* tCont = new TrackContainer();
  tCont->push_back(new Track(-1.));
  tCont->push_back(new Track(-2.));
  tCont->push_back(new Track(-3.));
  std::println ("Track Container.................: {} tracks",
                tCont->size());

  store.record (cCont, "cCont");
  store.record (tCont, "tCont");

  PTAss aMap;
  size_t tCtr = 0;
  size_t cCtr = 0;

  /////////////////////////////////////////////////
  ///  Exploring AssociationMap non-const interface
  ///

  ClusterContainer::const_iterator cEnd  = cCont->end();

  for ( ClusterContainer::const_iterator cIter = cCont->begin(); 
	cIter != cEnd; 
	++cIter ) {
    for ( tCtr=0; tCtr<tCont->size(); ++tCtr ) {
      const Track* aTrack = (*tCont)[tCtr];
      std::println ("Associate Clusters and Tracks...: [{},{}] with data [{},{}]",
                    cCtr, tCtr, (*cIter)->getE(), aTrack->getP());
      try {
	if ( 0 == tCtr ) {
	  aMap.addAssociation(cCont,(*cCont)[cCtr],tCont,(*tCont)[tCtr]);
	}
	else if ( 1 == tCtr ) {
	  aMap.addAssociation(ElementLink<ClusterContainer> ("cCont", cCtr),
                               ElementLink<TrackContainer> ("tCont", tCtr));
	}
        else {
	  aMap.addAssociation(cCont,cCtr,tCont,tCtr);
	}
      } catch(std::exception& error) {
        std::println (std::cerr, "Caught std::exception:\n{}", error.what());
      }
    }
    ++cCtr;
  }

  /////////////////////////////////////////////////
  ///  Exploring AssociationMap const interface
  ///

  const PTAss   * const cstMap    = &aMap;
  const Cluster * const myCluster = (*cCont)[0];
  const Track   * const myTrack   = (*tCont)[0];

  std::println ("List of objects in AssociationMap:");
  {
    PTAss::object_iterator cEnd = cstMap->endObject();
    for ( PTAss::object_iterator cIter = cstMap->beginObject();
	  cIter != cEnd;
	  ++cIter ) {
      const Cluster * const theCluster = (*cIter).getObject();
      assert (*(cIter.getObjectLink()) == theCluster);
      std::println ("\tCluster : ");
      PTAss::asso_iterator tEnd = cstMap->endAssociation(theCluster);
      for ( PTAss::asso_iterator tIter = cstMap->beginAssociation(theCluster);
	    tIter != tEnd;
	    ++tIter ) {
	const Track * const theTrack = *tIter;
        assert (*(tIter.getLink()) == theTrack);
	std::println ("\t\tTrack : {}", theTrack->getP());
	const Track * const assoTrack = cstMap->getAssociation( tIter );
	assert( theTrack == assoTrack );

	assert( tIter == cstMap->findAssociation( theCluster, theTrack ) );
	assert( tIter == cstMap->findAssociation( cIter,      theTrack ) );

	assert( cstMap->containsAssociation( theCluster, theTrack ) );
	assert( cstMap->containsAssociation( theTrack ) );

	{
	  std::list<const Cluster*> objectList;
	  assert( cstMap->getObjects( theTrack, objectList ) );
	  assert( tCont->size() == objectList.size() );
	}

	{
	  std::list<const Cluster*> objectList;
	  assert( cstMap->getObjects( cstMap->beginAssociation(theCluster), 
				      objectList ) );
	  assert( tCont->size() == objectList.size() );
	}

	assert( tCont->size() == cstMap->getNumberOfAssociations(cIter) );
	assert( tCont->size() == cstMap->getNumberOfAssociations(theCluster) );
      }
    }
  }

  std::println ("Check that AssociationMap contains myCluster : {}",
                ( cstMap->containsObject( myCluster ) ? "true" : "false" ));

  std::println ("Number of associations for myCluster= {}",
                cstMap->size( myCluster));
  assert( tCont->size() == cstMap->size( myCluster ) );

  {
    PTAss::asso_list myTracks;
    cstMap->getAssociations( myCluster, myTracks );
    assert( tCont->size() == myTracks.size() );
  }
  {
    PTAss::asso_list myTracks;
    cstMap->getAssociations( cstMap->findObject(myCluster), myTracks );
    assert( tCont->size() == myTracks.size() );
  }

  std::println ("Number of association objects:{}", cstMap->size());
  assert( cCtr == cstMap->size() );


  ////////////////////////////////////////////////////
  ///  Exploring AssociationMap with OBJCONT=ASSOCONT
  ///

  std::println ("\n *** Test AssociationMap<TrackContainer,TrackContainer> :");
  TTAss ttAsso;

  /////////////////////////////////////////////////
  ///  Exploring AssociationMap non-const interface
  ///

  TrackContainer::const_iterator tEnd  = tCont->end();
  unsigned int tIdx = 0;
  for ( TrackContainer::const_iterator tIter = tCont->begin(); 
	tIter != tEnd; 
	++tIter,++tIdx ) {
    unsigned int assoIdx = tIdx+1;
    for ( TrackContainer::const_iterator assItr = tIter+1; 
	  assItr != tEnd; 
	  ++assItr,++assoIdx ) {
      if ( *tIter != *assItr ) {
	std::println ("Associate Tracks and Tracks...: [{},{}] with data [{},{}]",
                      tIdx, assoIdx, (*tIter)->getP(),  (*assItr)->getP());
	if ( assoIdx == tIdx + 1 ) {
	  ttAsso.addAssociation( tCont, *tIter, tCont, *assItr );
	} else {
	  ttAsso.addAssociation( tCont, tIdx,   tCont, assoIdx );
	}
      }
    }//> loop over tracks to be associated
  }//> loop over tracks
  
  /////////////////////////////////////////////////
  ///  Exploring AssociationMap const interface
  ///

  const TTAss * const ttMap   = &ttAsso;
  
  assert( tCont->size()-1 == ttMap->size( myTrack ) );

  assert( tCont->size()-1 == ttMap->size() );
}


int main()
{
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();
  test1 (*store);

  return 0;
}
