// Copyright (c) 2017-2022, University of Tennessee. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#include "blas/device.hh"

#include "device_internal.hh"

namespace blas {

// -----------------------------------------------------------------------------
/// @deprecated
/// Set current GPU device.
/// (CUDA, ROCm only; doesn't work with SYCL.)
void set_device( int device )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas_dev_call(
            cudaSetDevice((device_blas_int)device) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas_dev_call(
            hipSetDevice((device_blas_int)device) );

    #elif defined(BLAS_HAVE_ONEMKL)
        throw blas::Error( "unsupported function for sycl backend", __func__ );

    #else
        throw blas::Error( "device BLAS not available", __func__ );
    #endif
}

// -----------------------------------------------------------------------------
/// Set the current GPU device as needed by the accelerator/gpu.
/// (CUDA, ROCm only; no-op for SYCL.)
void internal_set_device( int device )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas_dev_call(
            cudaSetDevice((device_blas_int)device) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas_dev_call(
            hipSetDevice((device_blas_int)device) );

    #elif defined(BLAS_HAVE_ONEMKL)
        // skip, no need to throw error since this is an internal function

    #else
        throw blas::Error( "unknown accelerator/gpu", __func__ );
    #endif
}

// -----------------------------------------------------------------------------
/// @deprecated
/// Get current GPU device.
/// (CUDA, ROCm only; doesn't work with SYCL.)
void get_device( int *device )
{
    #ifdef BLAS_HAVE_CUBLAS
        device_blas_int dev = -1;
        blas_dev_call(
            cudaGetDevice(&dev) );
        (*device) = dev;

    #elif defined(BLAS_HAVE_ROCBLAS)
        device_blas_int dev = -1;
        blas_dev_call(
            hipGetDevice(&dev) );
        (*device) = dev;

    #elif defined(BLAS_HAVE_ONEMKL)
        throw blas::Error( "unsupported function for sycl backend", __func__ );

    #else
        throw blas::Error( "device BLAS not available", __func__ );
    #endif
}

// -----------------------------------------------------------------------------
/// @return number of GPU devices.
device_blas_int get_device_count()
{
    device_blas_int dev_count = 0;

    #ifdef BLAS_HAVE_CUBLAS
        auto err = cudaGetDeviceCount(&dev_count);
        if (err != cudaSuccess && err != cudaErrorNoDevice)
            blas_dev_call( err );

    #elif defined(BLAS_HAVE_ROCBLAS)
        auto err = hipGetDeviceCount(&dev_count);
        if (err != hipSuccess && err != hipErrorNoDevice)
            blas_dev_call( err );

    #elif defined(BLAS_HAVE_ONEMKL)
        auto platforms = sycl::platform::get_platforms();
        for (auto &platform : platforms) {
            auto devices = platform.get_devices();
            for (auto &device : devices) {
                dev_count += device.is_gpu();
            }
        }

    #else
        // return dev_count = 0
    #endif

    return dev_count;
}

// -----------------------------------------------------------------------------
#ifdef BLAS_HAVE_ONEMKL
void enumerate_devices(std::vector<sycl::device> &devices)
{
    device_blas_int dev_count = get_device_count();

    if (devices.size() != (size_t)dev_count) {
        devices.clear();
        devices.reserve( dev_count );
    }

    auto platforms = sycl::platform::get_platforms();
    for (auto &platform : platforms) {
        auto all_devices = platform.get_devices();
        for (auto &idevice : all_devices) {
            if (idevice.is_gpu()) {
                devices.push_back( idevice );
            }
        }
    }

    // // must remove the if statement below in production mode
    // if (devices.size() == 0) {
    //     sycl::device default_device;
    //     devices.push_back( default_device );
    //     dev_count = 1;
    // }
}
#endif

// -----------------------------------------------------------------------------
/// @deprecated: use device_free( ptr, queue ).
/// Free a device memory space on the current device,
/// allocated with device_malloc.
/// (CUDA, ROCm only; doesn't work with SYCL.)
void device_free( void* ptr )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas_dev_call(
            cudaFree( ptr ) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas_dev_call(
            hipFree( ptr ) );

    #elif defined(BLAS_HAVE_ONEMKL)
        /// SYCL requires a device/queue to free.
        throw blas::Error( "unsupported function for sycl backend", __func__ );

    #else
        throw blas::Error( "device BLAS not available", __func__ );
    #endif
}

// -----------------------------------------------------------------------------
/// Free a device memory space, allocated with device_malloc,
/// on the queue's device.
void device_free( void* ptr, blas::Queue &queue )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas::internal_set_device( queue.device() );
        blas_dev_call(
            cudaFree( ptr ) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas::internal_set_device( queue.device() );
        blas_dev_call(
            hipFree( ptr ) );

    #elif defined(BLAS_HAVE_ONEMKL)
        blas_dev_call(
            sycl::free( ptr, queue.stream() ) );
    #endif
}

// -----------------------------------------------------------------------------
/// @deprecated: use host_free_pinned( ptr, queue ).
/// Free a pinned host memory space, allocated with host_malloc_pinned.
/// (CUDA, ROCm only; doesn't work with SYCL.)
void host_free_pinned( void* ptr )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas_dev_call(
            cudaFreeHost( ptr ) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas_dev_call(
            hipHostFree( ptr ) );

    #elif defined(BLAS_HAVE_ONEMKL)
        throw blas::Error( "unsupported function for sycl backend", __func__ );

    #else
        throw blas::Error( "device BLAS not available", __func__ );
    #endif
}

// -----------------------------------------------------------------------------
/// Free a pinned host memory space, allocated with host_malloc_pinned.
void host_free_pinned( void* ptr, blas::Queue &queue )
{
    #ifdef BLAS_HAVE_CUBLAS
        blas_dev_call(
            cudaFreeHost( ptr ) );

    #elif defined(BLAS_HAVE_ROCBLAS)
        blas_dev_call(
            hipHostFree( ptr ) );

    #elif defined(BLAS_HAVE_ONEMKL)
        blas_dev_call(
            sycl::free( ptr, queue.stream() ) );

    #else
        throw blas::Error( "device BLAS not available", __func__ );
    #endif
}

}  // namespace blas
