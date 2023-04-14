// Copyright 2023 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// This is a baseline sample runtime application using these features:
//   * High level "runtime" API (instead of lower level VM and HAL APIs)
//       * `iree_runtime_*` functions
//   * HAL driver registry
//       * `iree_runtime_instance_options_use_all_available_drivers`
//       * `iree_runtime_instance_driver_registry`
//   * Loading VM bytecode flatbuffers (.vmfb) from the file system
//       * `iree_runtime_session_append_bytecode_module_from_file`
//   * Buffer formatting and printing
//       * `iree_hal_buffer_view_fprint`

#include <iree/runtime/api.h>
#include <stdio.h>

static iree_status_t iree_runtime_demo_perform_mul(
    iree_runtime_session_t* session);

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: binarysize device module.vmfb\n");
    return 1;
  }
  const char* device_uri = argv[1];
  const char* module_path = argv[2];

  // Setup the shared runtime instance.
  iree_runtime_instance_options_t instance_options;
  iree_runtime_instance_options_initialize(&instance_options);
  iree_runtime_instance_options_use_all_available_drivers(&instance_options);
  iree_runtime_instance_t* instance = NULL;
  iree_status_t status = iree_runtime_instance_create(
      &instance_options, iree_allocator_system(), &instance);

  // Create the HAL device used to run the workloads.
  //
  // This form of iree_hal_create_device allows the user to pick the device on
  // the command line out of any available devices with their HAL drivers
  // compiled into the runtime.
  iree_hal_device_t* device = NULL;
  if (iree_status_is_ok(status)) {
    status = iree_hal_create_device(
        iree_runtime_instance_driver_registry(instance),
        iree_make_cstring_view(device_uri),
        iree_runtime_instance_host_allocator(instance), &device);
  }

  // Set up the session to run the demo module.
  iree_runtime_session_t* session = NULL;
  if (iree_status_is_ok(status)) {
    iree_runtime_session_options_t session_options;
    iree_runtime_session_options_initialize(&session_options);
    status = iree_runtime_session_create_with_device(
        instance, &session_options, device,
        iree_runtime_instance_host_allocator(instance), &session);
  }

  // Load the compiled user module from a file.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_session_append_bytecode_module_from_file(session,
                                                                   module_path);
  }

  // Build and issue the call.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_demo_perform_mul(session);
  }

  // Release resources (reverse order).
  iree_runtime_session_release(session);
  iree_hal_device_release(device);
  iree_runtime_instance_release(instance);

  int ret = (int)iree_status_code(status);
  if (!iree_status_is_ok(status)) {
    iree_status_fprint(stderr, status);
    iree_status_ignore(status);
  }
  return ret;
}

// Calls the simple_mul function and prints its results.
static iree_status_t iree_runtime_demo_perform_mul(
    iree_runtime_session_t* session) {
  // Initialize the call to the function.
  iree_runtime_call_t call;
  IREE_RETURN_IF_ERROR(iree_runtime_call_initialize_by_name(
      session, iree_make_cstring_view("module.simple_mul"), &call));

  // Append the function inputs with the HAL device allocator in use by the
  // session.
  iree_hal_allocator_t* device_allocator =
      iree_runtime_session_device_allocator(session);
  iree_allocator_t host_allocator =
      iree_runtime_session_host_allocator(session);
  iree_status_t status = iree_ok_status();
  if (iree_status_is_ok(status)) {
    // %lhs: tensor<4xf32>
    iree_hal_buffer_view_t* lhs = NULL;
    if (iree_status_is_ok(status)) {
      static const iree_hal_dim_t lhs_shape[1] = {4};
      static const float lhs_data[4] = {1.0f, 1.1f, 1.2f, 1.3f};
      status = iree_hal_buffer_view_allocate_buffer(
          device_allocator, IREE_ARRAYSIZE(lhs_shape), lhs_shape,
          IREE_HAL_ELEMENT_TYPE_FLOAT_32,
          IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR,
          (iree_hal_buffer_params_t){
              .type = IREE_HAL_MEMORY_TYPE_DEVICE_LOCAL,
              .access = IREE_HAL_MEMORY_ACCESS_READ,
              .usage = IREE_HAL_BUFFER_USAGE_DEFAULT,
          },
          iree_make_const_byte_span(lhs_data, sizeof(lhs_data)), &lhs);
    }
    if (iree_status_is_ok(status)) {
      IREE_IGNORE_ERROR(iree_hal_buffer_view_fprint(
          stdout, lhs, /*max_element_count=*/1024, host_allocator));
      status = iree_runtime_call_inputs_push_back_buffer_view(&call, lhs);
    }
    // Since the call retains the buffer view we can release it here.
    iree_hal_buffer_view_release(lhs);

    fprintf(stdout, "\n * \n");

    // %rhs: tensor<4xf32>
    iree_hal_buffer_view_t* rhs = NULL;
    if (iree_status_is_ok(status)) {
      static const iree_hal_dim_t rhs_shape[1] = {4};
      static const float rhs_data[4] = {10.0f, 100.0f, 1000.0f, 10000.0f};
      status = iree_hal_buffer_view_allocate_buffer(
          device_allocator, IREE_ARRAYSIZE(rhs_shape), rhs_shape,
          IREE_HAL_ELEMENT_TYPE_FLOAT_32,
          IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR,
          (iree_hal_buffer_params_t){
              .type = IREE_HAL_MEMORY_TYPE_DEVICE_LOCAL,
              .access = IREE_HAL_MEMORY_ACCESS_READ,
              .usage = IREE_HAL_BUFFER_USAGE_DEFAULT,
          },
          iree_make_const_byte_span(rhs_data, sizeof(rhs_data)), &rhs);
    }
    if (iree_status_is_ok(status)) {
      IREE_IGNORE_ERROR(iree_hal_buffer_view_fprint(
          stdout, rhs, /*max_element_count=*/1024, host_allocator));
      status = iree_runtime_call_inputs_push_back_buffer_view(&call, rhs);
    }
    iree_hal_buffer_view_release(rhs);
  }

  // Synchronously perform the call.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_call_invoke(&call, /*flags=*/0);
  }

  fprintf(stdout, "\n = \n");

  // Dump the function outputs.
  iree_hal_buffer_view_t* result = NULL;
  if (iree_status_is_ok(status)) {
    status = iree_runtime_call_outputs_pop_front_buffer_view(&call, &result);
  }
  if (iree_status_is_ok(status)) {
    status = iree_hal_buffer_view_fprint(
        stdout, result, /*max_element_count=*/1024, host_allocator);
  }
  iree_hal_buffer_view_release(result);

  iree_runtime_call_deinitialize(&call);
  return status;
}
