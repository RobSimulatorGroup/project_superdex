#pragma once

#include <mochi_physics/utils/step_profiling.h>

#include <entt/entt.hpp>

#include <atomic>
#include <chrono>

namespace mochi {

struct CStepProfile {
  struct Timing {
    std::atomic<double> seconds{0.0};
    std::atomic<std::uint64_t> calls{0};
  };
  std::array<Timing, static_cast<size_t>(StepProfileStage::Count)> stages;
  std::atomic<std::uint64_t> linearIterations{0};

  void Reset() {
    for (auto& stage : stages) {
      stage.seconds.store(0.0, std::memory_order_relaxed);
      stage.calls.store(0, std::memory_order_relaxed);
    }
    linearIterations.store(0, std::memory_order_relaxed);
  }

  void Add(StepProfileStage stage, double seconds, std::uint64_t calls = 1) {
    auto& timing = stages[static_cast<size_t>(stage)];
    timing.seconds.fetch_add(seconds, std::memory_order_relaxed);
    timing.calls.fetch_add(calls, std::memory_order_relaxed);
  }
};

// No thread-local state: Marl tasks can suspend and resume on a different worker.
class ScopedStepTiming {
 public:
  ScopedStepTiming(entt::registry& reg, StepProfileStage stage)
      : _profile(reg.try_ctx<CStepProfile>()), _stage(stage) {
    if (_profile) {
      _start = Clock::now();
    }
  }
  ~ScopedStepTiming() {
    if (_profile) {
      _profile->Add(_stage, std::chrono::duration<double>(Clock::now() - _start).count());
    }
  }
  ScopedStepTiming(ScopedStepTiming const&) = delete;
  ScopedStepTiming& operator=(ScopedStepTiming const&) = delete;

 private:
  using Clock = std::chrono::steady_clock;
  CStepProfile* _profile;
  StepProfileStage _stage;
  Clock::time_point _start{};
};

} // namespace mochi
