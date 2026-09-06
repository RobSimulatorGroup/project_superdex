#pragma once

#include <mochi_physics/mochi_physics.h>

#include <array>
#include <cstdint>

namespace mochi {

// Optional C++ diagnostics, independent of the generated scene/serialization API.
enum class StepProfileStage {
  PreStep,
  Islands,
  PostStep,
  IslandPrepare,
  IslandNewton,
  IslandQueries,
  CollisionDetection,
  ContactJacobians,
  Assembly,
  LinearSetup,
  LinearSolve,
  LineSearch,
  Count
};

struct StepStageTiming {
  double seconds = 0.0;
  std::uint64_t calls = 0;
};

struct StepProfile {
  bool enabled = false;
  // Inclusive elapsed times. Only PreStep/Islands/PostStep are sequential wall times.
  // All other entries sum scopes across islands/tasks and can overlap or nest:
  // line search includes assembly; assembly includes collision and contact Jacobians.
  std::array<StepStageTiming, static_cast<size_t>(StepProfileStage::Count)> stages{};
  std::uint64_t linearIterations = 0;
};

// Call only between steps on the scene's owning thread. Profiling is off by default,
// local to the scene, and excluded from checkpoints. Step(0) clears the counters.
MOCHI_API void SetStepProfilingEnabled(Scene& scene, bool enabled);
MOCHI_API StepProfile GetStepProfile(Scene const& scene);

} // namespace mochi
