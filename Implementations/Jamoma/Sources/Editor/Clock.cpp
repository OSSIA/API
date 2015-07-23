#include "Editor/JamomaClock.h"

#include <iostream> //! \todo to remove. only here for debug purpose

using namespace OSSIA;

# pragma mark -
# pragma mark Life cycle

namespace OSSIA
{
  shared_ptr<Clock> Clock::create(const TimeValue& duration,
                                       const TimeValue& granularity,
                                       const TimeValue& offset,
                                       float speed,
                                       bool external)
  {
    return make_shared<JamomaClock>(duration, granularity, offset, speed, external);
  }
}

JamomaClock::JamomaClock(const TimeValue& duration,
                         const TimeValue& granularity,
                         const TimeValue& offset,
                         float speed,
                         bool external) :
mDuration(duration),
mGranularity(granularity),
mOffset(offset),
mSpeed(speed),
mExternal(external),
mRunning(false)
{}

JamomaClock::JamomaClock(const JamomaClock * other)
{}

shared_ptr<Clock> JamomaClock::clone() const
{
  return make_shared<JamomaClock>(this);
}

JamomaClock::~JamomaClock()
{
  stop();
}

# pragma mark -
# pragma mark Execution

void JamomaClock::go()
{
  if (mDuration <= mOffset)
    return stop();
  
  if (mRunning)
    return;
  
  // reset timing informations
  mRunning = true;
  mPaused = false;

  // set clock at a time grain
  mDate = std::floor(mOffset / (mGranularity * mSpeed)) * (mGranularity * mSpeed);
  mPosition = mDate / mDuration;
  mLastTime = steady_clock::now();
  mElapsedTime = std::floor(mOffset / mGranularity) * mGranularity * 1000;
  
  //! \todo notify each observers
  // sendNotification(TTSymbol("ClockRunningChanged"), mRunning);

  if (!mExternal)
  {
    if (mThread.joinable())
        mThread.join();
      
    // launch a new thread to run the clock execution
    mThread = thread(&JamomaClock::threadCallback, this);
  }
}

void JamomaClock::stop()
{
  mRunning = false;
  mPaused = false;
  
  if (!mExternal)
  {
    if (mThread.joinable())
        mThread.join();
  }
    
  //! \todo notify each observers
  // sendNotification(TTSymbol("ClockRunningChanged"), mRunning);
}

void JamomaClock::pause()
{
  mPaused = true;
}

void JamomaClock::resume()
{
  mPaused = false;
}

bool JamomaClock::tick()
{
  if (mPaused || !mRunning)
     return false;
  
  long long granularityInUs(mGranularity * 1000);
  
  // how many time since the last tick ?
  long long deltaInUs = duration_cast<microseconds>(steady_clock::now() - mLastTime).count();
  
  if (mExternal)
  {
    // if too early: avoid this tick
    if (mElapsedTime / granularityInUs == (mElapsedTime + deltaInUs) / granularityInUs)
      return false;
  }
  else
  {
    // how many time to pause to reach the next time grain ?
    long long pauseInUs = granularityInUs - mElapsedTime % granularityInUs;
    
    //! \debug cout << "pause = " << pauseInUs / 1000. << endl;

    // if too early: wait
    if (pauseInUs > 0)
    {
      // pause the thread
      this_thread::sleep_for(std::chrono::microseconds(pauseInUs));
      
      // how many time since the last tick ?
      deltaInUs = duration_cast<microseconds>(steady_clock::now() - mLastTime).count();
    }
    
    //! \debug cout << "delta = " << deltaInUs / 1000. << endl;
  }
  
  // how many time elapsed from the start ?
  mDate += (deltaInUs / 1000.) * mSpeed;
  mElapsedTime += deltaInUs;

  // test also paused and running status after computing the date because there is a sleep before
  if (!mPaused && mRunning)
  {
    if (mDuration - mDate >= Zero || mDuration.isInfinite())
    {
      mPosition = mDate / mDuration;
      
      // notify the owner
      (mCallback)(mPosition, mDate);
    }
    else
    {
      // notify the owner forcing position to 1. to allow filtering
      (mCallback)(One, mDate);
      
      mRunning = false;
      mPaused = false;
    }
  }
  
  // tick is done now
  mLastTime = steady_clock::now();
  
  return true;
}

JamomaClock::ExecutionCallback JamomaClock::getExecutionCallback() const
{
  return mCallback;
}

void JamomaClock::setExecutionCallback(ExecutionCallback callback)
{
  mCallback = callback;
}

# pragma mark -
# pragma mark Accessors

const TimeValue & JamomaClock::getDuration() const
{
  return mDuration;
}

Clock & JamomaClock::setDuration(const TimeValue& duration)
{
  mDuration = duration;
  mDate = mOffset;
  
  if (mDuration != Zero)
    mPosition = mDate / mDuration;
  else
    mPosition = Zero;
  
  return *this;
}

const TimeValue & JamomaClock::getGranularity() const
{
  return mGranularity;
}

Clock & JamomaClock::setGranularity(const TimeValue& granularity)
{
  mGranularity = granularity;
  return *this;
}

const TimeValue & JamomaClock::getOffset() const
{
  return mOffset;
}

Clock & JamomaClock::setOffset(const TimeValue& offset)
{
  mOffset = offset;
  mDate = mOffset;
  
  if (mDuration != Zero)
    mPosition = mDate / mDuration;
  else
    mPosition = Zero;
  
  return *this;
}

float JamomaClock::getSpeed() const
{
  return mSpeed;
}

Clock & JamomaClock::setSpeed(float speed)
{
  mSpeed = speed;
  return *this;
}

bool JamomaClock::getExternal() const
{
  return mExternal;
}

Clock & JamomaClock::setExternal(bool external)
{
  mExternal = external;
  return *this;
}

bool JamomaClock::getRunning() const
{
  return mRunning;
}

const TimeValue & JamomaClock::getPosition() const
{
  return mPosition;
}

const TimeValue & JamomaClock::getDate() const
{
  return mDate;
}

# pragma mark -
# pragma mark Internal

void JamomaClock::threadCallback()
{
  // launch the tick if the duration is valid and while it have to run
  if (mDuration > Zero)
    while (mRunning)
      tick();
}
