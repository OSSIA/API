/*!
 * \file State_Impl.h
 *
 * \author Clément Bossut
 * \author Théo de la Hogue
 *
 * This code is licensed under the terms of the "CeCILL-C"
 * http://www.cecill.info
 */

#ifndef STATE_IMPL_H_
#define STATE_IMPL_H_

#include "Editor/State.h"

#include "TTScore.h"

namespace OSSIA {

class State_Impl : public State {

public:

  // Constructors, destructor, assignment
  virtual State() override;
  virtual ~State() = default;

  // Lecture
  virtual void launch() const override;

  // Iterators
  class const_iterator; // bidirectional
  virtual const_iterator begin() const override;
  virtual const_iterator end() const override;
  virtual const_iterator find(const StateElement&) const override;

  // Managing StateElements
  virtual void addStateElement(const StateElement&) override;
  virtual bool removeStateElement(const StateElement&) override;
  
  // Implementation Specific
};

}

#endif /* STATE_IMPL_H_ */
