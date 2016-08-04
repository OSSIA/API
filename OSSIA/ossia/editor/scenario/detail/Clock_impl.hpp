#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <ctime>
#include <mutex>
#include <ratio>
#include <thread>

#include <ossia/editor/scenario/clock.hpp>

using namespace ossia;

using namespace std::chrono;

namespace ossia
{
namespace detail
{
class clock_impl : public virtual clock
{
public:
  using clock_type = std::chrono::steady_clock;

protected:
  time_value mDuration{};    /// the time (in ms) the clock will run at normal
                             /// speed factor
  time_value mGranularity{}; /// the minimum time between each tick (in ms)
  time_value mOffset{};      /// the date (in ms) the clock will run from
  double mSpeed{};           /// the speed factor of the clock

  clock::DriveMode
      mDriveMode{}; /// in EXTERNAL drive mode the tick() method is
                    /// called from outside

  std::atomic_bool mRunning{}; /// is the clock running right now ?
  std::atomic_bool mPaused{};  /// is the clock paused right now ?
  time_value
      mPosition{};    /// the progression of the clock between the beginning
                      /// and the end [0. :: 1.]
  time_value mDate{}; /// how many time the clock is running (without no speed
                      /// factor consideration)

  std::thread mThread; /// a thread to launch the clock execution

  clock_type::time_point
      mLastTime{};          /// a time reference used to compute time tick
  long long mElapsedTime{}; /// a time reference used to know how many time are
                            /// elapsed in microsecond

private:
  ExecutionCallback mCallback; /// the callback to use for each step
  ExecutionStatusCallback mStatusCallback;

public:
  clock_impl(
      clock::ExecutionCallback, time_value = Infinite, time_value = 1.,
      time_value = 0., float = 1.,
      clock::DriveMode = clock::DriveMode::INTERNAL);

  ~clock_impl();

  void start() override;

  void stop() override;

  void pause() override;

  bool paused() const final override;

  void resume() override;

  bool tick() final override;
  bool tick(time_value usec) final override;

  time_value getDuration() const override;

  clock& setDuration(time_value) override;

  time_value getOffset() const final override;

  clock& setOffset(time_value) override;

  time_value getGranularity() const final override;

  clock& setGranularity(time_value) final override;

  double getSpeed() const final override;

  clock& setSpeed(double) final override;

  clock::DriveMode getDriveMode() const final override;

  clock& setDriveMode(clock::DriveMode) final override;

  bool getRunning() const final override;

  time_value getPosition() const final override;

  time_value getDate() const final override;

  void setExecutionStatusCallback(ExecutionStatusCallback e) final override
  {
    mStatusCallback = e;
  }
  ExecutionStatusCallback getExecutionStatusCallback() const final override
  {
    return mStatusCallback;
  }

  /*! to avoid dead lock in EXTERNAL drive mode if a TimeProcess wants to end
   * its ParentTimeConstraint's clock */
  void request_stop();

protected:
  /*! to allow TimeConstraint to override start method */
  void do_start() override;

  /*! to allow TimeConstraint to override stop method */
  void do_stop() override;

  /*! to allow TimeConstraint to override setDuration accessor */
  void do_setDuration(time_value) override;

  /*! to allow TimeConstraint to override setOffset accessor */
  void do_setOffset(time_value) override;

private:
  /*! called back by the internal thread */
  void threadCallback();
};
}
}
