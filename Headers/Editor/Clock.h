/*!
 * \file Clock.h
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
#include <functional>

#include "Editor/TimeValue.h"

namespace OSSIA
{

class Clock
{

public:
  
# pragma mark -
# pragma mark Life cycle
  
  /*! factory
   \param duration
   \param offset
   \param speed
   \return std::shared_ptr<#Clock> */
  static std::shared_ptr<Clock> create(const TimeValue,
                                       const TimeValue = 0.,
                                       const float = 0.);
  
  /*! destructor */
  virtual ~Clock() = default;

# pragma mark -
# pragma mark Execution
  
  /*! start the clock */
  virtual void go() = 0;
  
  /*! halt the clock */
  virtual void stop() = 0;
  
  /*! pause the clock progression */
  virtual void pause() = 0;
  
  /*! resume the clock progression */
  virtual void resume() = 0;
  
  /*! called every time a new step should be processed */
  virtual void tick() = 0;
  
# pragma mark -
# pragma mark Accessors
  
  /*! get the duration of the clock
   \return const #TimeValue duration */
  virtual const TimeValue getDuration() const;
  
  /*! set the duration of the clock execution
   \param const #TimeValue duration
   \return #Clock the clock */
  virtual Clock & setDuration(const TimeValue);
  
  /*! get the offset of the clock
   \return const #TimeValue offset */
  virtual const TimeValue getOffset() const;
  
  /** set the offset of the clock
   \param const #TimeValue offset
   \return #Clock the clock */
  virtual Clock & setOffset(const TimeValue);
  
  /*! get the speed of the clock
   \return const #TimeValue speed */
  virtual const float getSpeed() const;
  
  /** set the speed factor attribute
   \param const float speed factor
   \return #Clock the clock */
  virtual Clock & setSpeed(const float);
  
# pragma mark -
# pragma mark Callback
  
  /*! to get the clock position back */
  using PositionCallback = std::function<void(const TimeValue)>;
  
  /*! get the clock position callback function
   \return #ValueCallback function */
  virtual PositionCallback getPositionCallback() const = 0;
  
  /*! set the clock position callback function
   \param #ValueCallback function */
  virtual void setPositionCallback(PositionCallback) = 0;
  
};
}
