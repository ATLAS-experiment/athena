/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "CaloEvent/CaloClusterLinkTemplate.h"

#include "CaloEvent/CaloClusterNavigable.h"
#include "CaloEvent/CaloCellLink.h"
#include "CaloEvent/CaloCellLinkContainer.h"

#include <algorithm>
//#include <iostream>

CaloClusterNavigable::CaloClusterNavigable()
  : m_ownLinkStore(false)
{ }

CaloClusterNavigable::CaloClusterNavigable(CaloCellLinkContainer* pLinkColl,
					   CaloCellLink* pLink)
  : m_ownLinkStore(false)
{
  CaloClusterLinkTemplate<CaloCellLinkContainer>::setLink(pLinkColl,
							  pLink,
							  m_cellLink);
}

CaloClusterNavigable::CaloClusterNavigable(CaloCellLink* pLink)
  : m_ownLinkStore(true)
{
  m_cellLink.setElement(pLink);
}

CaloClusterNavigable::~CaloClusterNavigable()
{ 
  // If we own the store, the link holds the pointer directly.
  if ( m_ownLinkStore ) delete m_cellLink.cachedElement();
}

CaloCellLink* CaloClusterNavigable::getCellLink()
{
  if ( m_ownLinkStore )
    {
      if ( *m_cellLink == nullptr ) m_cellLink.setElement(new CaloCellLink());
      // Ok, because this happens only if we own the pointer.
      CaloCellLink* link ATLAS_THREAD_SAFE = const_cast<CaloCellLink*>(*m_cellLink);
      return link;
    }
  else
    {
      return (CaloCellLink*)nullptr;
    }

  // check if local pointer to store
  //  if ( m_cellLinkPointer != 0 ) { return m_cellLinkPointer;}
  // check if linked pointer to store
  //  if ( m_cellLink.isValid() && *m_cellLink != 0 ) 
  //    { 
      // FIXME
  //     CaloCellLink* pLink = const_cast<CaloCellLink*>(*m_cellLink);
  //      return pLink; 
  //    }
  // create a new local store/
  //  m_cellLinkPointer = new CaloCellLink();
  //  return m_cellLinkPointer;
}

const CaloCellLink* 
CaloClusterNavigable::getCellLink() const
{
  // return *m_cellLink;

  //  if ( m_cellLinkPointer != 0 )
  //  {
      //      std::cout << "[CaloClusterNavigable@" << this << "] return "
      //		<< "cell link @" << m_cellLinkPointer << std::endl;
      //      return m_cellLinkPointer;
      //    } 
  //  if ( *m_cellLink == 0 )
  //    {
  //      std::cout << "[CaloClusterNavigable@" << this << "] problem with "
  //		<< "cell link store, unexpected pointer = "
  //		<< *m_cellLink << std::endl;
  //    }
  return m_cellLink.isValid() ? *m_cellLink : nullptr;
}
