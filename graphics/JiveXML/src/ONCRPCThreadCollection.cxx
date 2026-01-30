/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include <JiveXML/ONCRPCThreadCollection.h>

namespace JiveXML{
  
   //Constructor
  ThreadCollection::ThreadCollection(){

    //initialize the semaphore
    sem_init(&m_semaphore,0,0);

  }

   //Constructor
  ThreadCollection::~ThreadCollection(){

    //Destroy the semaphore
    sem_destroy(&m_semaphore);

  }

  //Add a thread to the container - don't need to be too strict
  //If a thread is double added, joinAll will still remove it.
  void ThreadCollection::AddThread( const pthread_t& thread ){
    
    //Now add the thread to the list of vectors
    {
      std::lock_guard lock (m_mutex);
      push_back(thread);
    }

    //And signal any potentially waiting threads
    sem_post(&m_semaphore);
    
  }

  //Wait until a thread has been added
  void ThreadCollection::WaitAdd(){

    //simply wait for the access semaphore to be set
    sem_wait(&m_semaphore);
  }

  //Remove a thread
  void ThreadCollection::RemoveThread( const pthread_t& thread ){

    //First get a mutex
    std::lock_guard lock (m_mutex);

    //Loop over list and find that entry
    ThreadCollection::iterator threadItr = begin();
    while ( threadItr != end() ){
      //See if this is the thread we are looking for
      if ( *threadItr == thread ){
        //remove it from the collection
        erase(threadItr);
        //iterator is invalid, we removed thread, so stop looping
        break ;
      }
      //Go to next thread
      ++threadItr;
    }

    //Set this threads state to detached, so its
    //resources are reclaimed once it
    //finishes.
    pthread_detach(thread);
    
  }

  //Wait for all threads to finish
  void ThreadCollection::JoinAll(){
    
    //The threads are removing themselves from the list,
    //so iterators are getting invalid while we are looping.
    //However, some threads may have crashed w/o being able to remove themselves
    //So wait for all of them to finish w/o keeping the mutex locked while
    //waiting
    
    //Loop till all threads are gone
    while ( size() > 0 ){

      //Order is not important - take the first element
      pthread_t thread;
      {
        std::lock_guard lock (m_mutex);
        thread = *begin();
      }

      //Wait for that thread to finish
      pthread_join(thread,NULL);

      //Now remove it - if it has already removed itself, nothing will happen
      //If it hadn't removed itself, we will remove it
      RemoveThread(thread);
    }
  }

    
  //Return number of elements in the vector
  int ThreadCollection::NumberOfThreads() {

    std::lock_guard lock (m_mutex);
    //Return number of elements
    return size();
  }

}//namespace
