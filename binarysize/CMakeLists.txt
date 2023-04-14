# Copyright 2023 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# --------------------------------------------------------------------------- #
# Baseline binary size sample application
# --------------------------------------------------------------------------- #

set(_NAME "binarysize")
add_executable(${_NAME})
target_sources(${_NAME} PRIVATE "binarysize.c")

# Rename to keep files on disk for future comparison
# set_target_properties(${_NAME} PROPERTIES OUTPUT_NAME "binarysize_alldrivers")

target_compile_options(${_NAME} PRIVATE ${IREE_DEFAULT_COPTS})

if(MSVC)
  target_link_options(${_NAME} PRIVATE "/INCREMENTAL:NO")
endif()

target_link_libraries(${_NAME} iree_runtime_runtime)

# --------------------------------------------------------------------------- #
# Static library binary size sample application
# --------------------------------------------------------------------------- #

# PREREQUISITE: build simple_mul.o and simple_mul.h
# TODO(scotttodd): enforce that, skip when not enabled
# TODO(scotttodd): add build.sh script that generates it

set(_MODEL_DIR "${CMAKE_BINARY_DIR}/models/")
add_library(simple_mul STATIC "${_MODEL_DIR}/simple_mul.o")
set_target_properties(simple_mul PROPERTIES LINKER_LANGUAGE C)

set(_NAME "binarysize_static")
add_executable(${_NAME})
target_sources(${_NAME} PRIVATE "binarysize_static.c")

target_compile_options(${_NAME} PRIVATE ${IREE_DEFAULT_COPTS})

if(MSVC)
  target_link_options(${_NAME} PRIVATE "/INCREMENTAL:NO")
endif()

target_link_libraries(${_NAME}
  iree_runtime_runtime
  simple_mul
)
