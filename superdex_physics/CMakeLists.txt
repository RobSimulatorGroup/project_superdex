# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_policy(VERSION 3.25)
cmake_minimum_required(VERSION 3.25)

# Apple's linker rescans static archives, so a library repeated on the link line is redundant and
# is diagnosed as a duplicate-libraries warning. CMP0156 NEW de-duplicates link-line entries, so
# only one copy of each library is passed and the warning goes away as a consequence.
if (POLICY CMP0156)
    cmake_policy(SET CMP0156 NEW)
endif ()

# Select default compiler. Must come before "project(superdex_physics)"
include("${CMAKE_CURRENT_LIST_DIR}/cmake/compiler.cmake")

#-----------------------------------------------------------------------------
# Root Project Setup
#-----------------------------------------------------------------------------

project(superdex_physics)

include("cmake/paths.cmake")

# Include build-wide settings that must be applied by the top level project.
# No-op if an outer project already included it.
include("cmake/root_project_setup.cmake")

#-----------------------------------------------------------------------------
# SuperDex Physics Settings
#-----------------------------------------------------------------------------

include("cmake/settings.cmake")

#-----------------------------------------------------------------------------
# Derived Settings
#-----------------------------------------------------------------------------

if (SUPERDEX_PHYSICS_CORE_ONLY)
    set(SUPERDEX_PHYSICS_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(SUPERDEX_PHYSICS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_ALL OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_DEBUGGER OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_IMGUIOS OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_RENDERER OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_MESH_LIB OFF CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_MESH_CLI OFF CACHE BOOL "" FORCE)
    set(MOCHI_USE_EIGEN OFF CACHE BOOL "" FORCE)
    set(MOCHI_USE_HDF5 OFF CACHE BOOL "" FORCE)
    set(MOCHI_USE_PYBIND OFF CACHE BOOL "" FORCE)
endif ()

if (SUPERDEX_PHYSICS_BUILD_ALL)
    set(SUPERDEX_PHYSICS_BUILD_BENCHMARKS ON CACHE BOOL "" FORCE)
    set(SUPERDEX_PHYSICS_BUILD_TESTS ON CACHE BOOL "" FORCE)
    set(MOCHI_BUILD_ALL ON CACHE BOOL "" FORCE)
endif ()

if (SUPERDEX_PHYSICS_BUILD_BENCHMARKS)
    set(MOCHI_BUILD_BENCHMARKS ON CACHE BOOL "" FORCE)
endif ()

if (SUPERDEX_PHYSICS_BUILD_TESTS)
    set(MOCHI_BUILD_TESTS ON CACHE BOOL "" FORCE)
endif ()

#-----------------------------------------------------------------------------
# Mochi (physics simulation backend)
#-----------------------------------------------------------------------------

# Set path variables for mochi. It will configure imguios and simple_reflection.
set(MOCHI_IMGUIOS_SOURCE_DIR "${SUPERDEX_IMGUIOS_SOURCE_DIR}" CACHE PATH "Source directory for building imguios" FORCE)
set(MOCHI_SIMPLE_REFLECTION_SOURCE_DIR "${SUPERDEX_SIMPLE_REFLECTION_SOURCE_DIR}" CACHE PATH "Source directory for building simple_reflection" FORCE)

add_subdirectory("${SUPERDEX_MOCHI_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/mochi")

#-----------------------------------------------------------------------------
# superdex_physics (header-only interface wrapper for mochi_physics)
#-----------------------------------------------------------------------------

mochi_add_interface(superdex_physics)
set_target_properties(superdex_physics PROPERTIES EXPORT_NAME Physics)
target_compile_features(superdex_physics INTERFACE cxx_std_20)
target_include_directories(superdex_physics INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(superdex_physics INTERFACE
    $<BUILD_INTERFACE:mochi_physics>
    $<INSTALL_INTERFACE:SuperDex::MochiPhysics>)
add_library(SuperDex::Physics ALIAS superdex_physics)

# Keep the install package deliberately narrow. mochi_core and the debugger
# support archive are linked into mochi_physics; marl is its only non-system
# runtime dependency in the core-only configuration.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set_target_properties(mochi_physics marl PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS mochi_physics marl
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS superdex_physics
    EXPORT SuperDexPhysicsTargets)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${SUPERDEX_MOCHI_SOURCE_DIR}/mochi_physics/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(DIRECTORY "${SUPERDEX_MOCHI_SOURCE_DIR}/mochi_core/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

set(SUPERDEX_PHYSICS_USE_DOUBLE_PRECISION 0)
set(SUPERDEX_PHYSICS_USE_AVX512 0)
set(SUPERDEX_PHYSICS_LIBRARY_NAME "mochi_physics")
if (MOCHI_USE_DOUBLE_PRECISION)
    set(SUPERDEX_PHYSICS_USE_DOUBLE_PRECISION 1)
    set(SUPERDEX_PHYSICS_LIBRARY_NAME "mochi_physics_double")
endif ()
if (MOCHI_USE_AVX512)
    set(SUPERDEX_PHYSICS_USE_AVX512 1)
endif ()
set(SUPERDEX_PHYSICS_LIBRARY_FILENAME
    "${CMAKE_SHARED_LIBRARY_PREFIX}${SUPERDEX_PHYSICS_LIBRARY_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SuperDexPhysicsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SuperDexPhysicsConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SuperDexPhysics")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SuperDexPhysicsConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion)

install(EXPORT SuperDexPhysicsTargets
    FILE SuperDexPhysicsTargets.cmake
    NAMESPACE SuperDex::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SuperDexPhysics")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SuperDexPhysicsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SuperDexPhysicsConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SuperDexPhysics")
