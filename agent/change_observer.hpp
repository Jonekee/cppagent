//
// Copyright Copyright 2012, System Insights, Inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once

#include <vector>
#include "dlib/threads.h"
#include "globals.hpp"

class ChangeSignaler;

class ChangeObserver
{  
public:
  ChangeObserver() : m_signal(m_mutex), m_sequence(UINT64_MAX) { }
  virtual ~ChangeObserver();

  bool wait(unsigned long aTimeout) {
    dlib::auto_mutex lock(m_mutex);
    if (m_sequence == UINT64_MAX)
      return m_signal.wait_or_timeout(aTimeout); 
    else
      return true;
  }
  void signal(uint64_t aSequence) {
    dlib::auto_mutex lock(m_mutex);
    if (m_sequence > aSequence && aSequence != 0)
      m_sequence = aSequence;
    m_signal.signal(); 
  }
  uint64_t getSequence() const {
    return m_sequence;
  }
  bool wasSignaled() const {
    return m_sequence != UINT64_MAX;
  }
  void reset() { 
    dlib::auto_mutex lock(m_mutex); 
    m_sequence = UINT64_MAX; 
  }
  
private:
  dlib::rmutex m_mutex;
  dlib::rsignaler m_signal;
  std::vector<ChangeSignaler*> m_signalers;
  volatile uint64_t m_sequence;
  
protected:
  friend class ChangeSignaler;
  void addSignaler(ChangeSignaler *aSig);
  bool removeSignaler(ChangeSignaler *aSig);
};

class ChangeSignaler 
{
public:
  /* Observer Management */
  void addObserver(ChangeObserver *aObserver);
  bool removeObserver(ChangeObserver *aObserver);
  bool hasObserver(ChangeObserver *aObserver);
  void signalObservers(uint64_t aSequence);
  
  virtual ~ChangeSignaler();
  
protected:
  /* Observer Lists */
  dlib::rmutex m_observerMutex;
  std::vector<ChangeObserver*> m_observers;
};
