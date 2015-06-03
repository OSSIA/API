/*!
 * \file State.h
 *
 * \brief
 *
 * \details
 *
 * \author Clément Bossut
 * \author Théo de la Hogue
 *
 * \copyright This code is licensed under the terms of the "CeCILL-C"
 * http://www.cecill.info
 */

#pragma once

#include <memory>

#include "Editor/StateElement.h"
#include "Misc/Container.h"

namespace OSSIA {

class State : public StateElement {

    public:

      // Life cycle
      static std::shared_ptr<State> create();
      virtual std::shared_ptr<State> clone() const = 0;
      virtual ~State() = default;

      // Execution
      virtual void launch() const override = 0;

      Container<StateElement>& stateElements()
      { return m_stateElements; }
      const Container<StateElement>& stateElements() const
      { return m_stateElements; }

    private:
      Container<StateElement> m_stateElements;

};

}
