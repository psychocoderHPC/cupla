/* Copyright 2019 Axel Huebl, Benjamin Worpitz, Matthias Werner, René Widera
 *
 * This file is part of alpaka.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED

#include <alpaka/core/BoostPredef.hpp>

#if !BOOST_LANG_CUDA
    #error If ALPAKA_ACC_GPU_CUDA_ENABLED is set, the compiler has to support CUDA!
#endif

#include <alpaka/elem/Traits.hpp>
#include <alpaka/offset/Traits.hpp>
#include <alpaka/extent/Traits.hpp>
#include <alpaka/idx/Traits.hpp>
#include <alpaka/vec/Vec.hpp>
#include <alpaka/meta/Metafunctions.hpp>

// cuda_runtime_api.h: CUDA Runtime API C-style interface that does not require compiling with nvcc.
// cuda_runtime.h: CUDA Runtime API  C++-style interface built on top of the C API.
//  It wraps some of the C API routines, using overloading, references and default arguments.
//  These wrappers can be used from C++ code and can be compiled with any C++ compiler.
//  The C++ API also has some CUDA-specific wrappers that wrap C API routines that deal with symbols, textures, and device functions.
//  These wrappers require the use of \p nvcc because they depend on code being generated by the compiler.
//  For example, the execution configuration syntax to invoke kernels is only available in source code compiled with nvcc.
#include <cuda_runtime.h>
#include <cuda.h>

#include <type_traits>
#include <utility>
#include <string>
#include <stdexcept>
#include <cstddef>

#if (!defined(CUDART_VERSION) || (CUDART_VERSION < 9000))
    #error "CUDA version 9.0 or greater required!"
#endif

#if (!defined(CUDA_VERSION) || (CUDA_VERSION < 9000))
    #error "CUDA version 9.0 or greater required!"
#endif

#define ALPAKA_PP_CONCAT_DO(X,Y) X##Y
#define ALPAKA_PP_CONCAT(X,Y) ALPAKA_PP_CONCAT_DO(X,Y)
//! prefix a name with `cuda`
#define ALPAKA_API_PREFIX(name) ALPAKA_PP_CONCAT_DO(cuda,name)

namespace alpaka
{
    namespace cuda
    {
        namespace detail
        {
            //-----------------------------------------------------------------------------
            //! CUDA driver API error checking with log and exception, ignoring specific error values
            ALPAKA_FN_HOST inline auto cudaDrvCheck(
                CUresult const & error,
                char const * desc,
                char const * file,
                int const & line)
            -> void
            {
                if(error == CUDA_SUCCESS)
                    return;

                char const * cu_err_name = nullptr;
                char const * cu_err_string = nullptr;
                CUresult cu_result_name = cuGetErrorName(error, &cu_err_name);
                CUresult cu_result_string = cuGetErrorString(error, &cu_err_string);
                std::string sError = std::string(file)
                                   + "(" + std::to_string(line) + ") "
                                   + std::string(desc) + " : '";
                if( cu_result_name == CUDA_SUCCESS && cu_result_string == CUDA_SUCCESS )
                {
                    sError += std::string(cu_err_name) +  "': '"
                            + std::string(cu_err_string) + "'!";
                } else {
                    // cuGetError*() failed, so append corresponding error message
                    if( cu_result_name == CUDA_ERROR_INVALID_VALUE ) {
                        sError += " cuGetErrorName: 'Invalid Value'!";
                    }
                    if( cu_result_string == CUDA_ERROR_INVALID_VALUE ) {
                        sError += " cuGetErrorString: 'Invalid Value'!";
                    }
                }
#if ALPAKA_DEBUG >= ALPAKA_DEBUG_MINIMAL
                std::cerr << sError << std::endl;
#endif
                ALPAKA_DEBUG_BREAK;
                throw std::runtime_error(sError);
            }
        }
    }
}

//-----------------------------------------------------------------------------
//! CUDA driver error checking with log and exception.
#define ALPAKA_CUDA_DRV_CHECK(cmd)\
    ::alpaka::cuda::detail::cudaDrvCheck(cmd, #cmd, __FILE__, __LINE__)


//-----------------------------------------------------------------------------
// CUDA vector_types.h trait specializations.
namespace alpaka
{
    //-----------------------------------------------------------------------------
    //! The CUDA specifics.
    namespace cuda
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors 1D dimension get trait specialization.
            template<
                typename T>
            struct IsCudaBuiltInType :
                std::integral_constant<
                    bool,
                    std::is_same<T, char1>::value
                    || std::is_same<T, double1>::value
                    || std::is_same<T, float1>::value
                    || std::is_same<T, int1>::value
                    || std::is_same<T, long1>::value
                    || std::is_same<T, longlong1>::value
                    || std::is_same<T, short1>::value
                    || std::is_same<T, uchar1>::value
                    || std::is_same<T, uint1>::value
                    || std::is_same<T, ulong1>::value
                    || std::is_same<T, ulonglong1>::value
                    || std::is_same<T, ushort1>::value
                    || std::is_same<T, char2>::value
                    || std::is_same<T, double2>::value
                    || std::is_same<T, float2>::value
                    || std::is_same<T, int2>::value
                    || std::is_same<T, long2>::value
                    || std::is_same<T, longlong2>::value
                    || std::is_same<T, short2>::value
                    || std::is_same<T, uchar2>::value
                    || std::is_same<T, uint2>::value
                    || std::is_same<T, ulong2>::value
                    || std::is_same<T, ulonglong2>::value
                    || std::is_same<T, ushort2>::value
                    || std::is_same<T, char3>::value
                    || std::is_same<T, dim3>::value
                    || std::is_same<T, double3>::value
                    || std::is_same<T, float3>::value
                    || std::is_same<T, int3>::value
                    || std::is_same<T, long3>::value
                    || std::is_same<T, longlong3>::value
                    || std::is_same<T, short3>::value
                    || std::is_same<T, uchar3>::value
                    || std::is_same<T, uint3>::value
                    || std::is_same<T, ulong3>::value
                    || std::is_same<T, ulonglong3>::value
                    || std::is_same<T, ushort3>::value
                    || std::is_same<T, char4>::value
                    || std::is_same<T, double4>::value
                    || std::is_same<T, float4>::value
                    || std::is_same<T, int4>::value
                    || std::is_same<T, long4>::value
                    || std::is_same<T, longlong4>::value
                    || std::is_same<T, short4>::value
                    || std::is_same<T, uchar4>::value
                    || std::is_same<T, uint4>::value
                    || std::is_same<T, ulong4>::value
                    || std::is_same<T, ulonglong4>::value
                    || std::is_same<T, ushort4>::value
// CUDA built-in variables have special types in clang native CUDA compilation
// defined in cuda_builtin_vars.h
#if BOOST_COMP_CLANG_CUDA
                    || std::is_same<T, __cuda_builtin_threadIdx_t>::value
                    || std::is_same<T, __cuda_builtin_blockIdx_t>::value
                    || std::is_same<T, __cuda_builtin_blockDim_t>::value
                    || std::is_same<T, __cuda_builtin_gridDim_t>::value
#endif
                >
            {};
        }
    }
    namespace traits
    {
        //#############################################################################
        //! The CUDA vectors 1D dimension get trait specialization.
        template<
            typename T>
        struct DimType<
            T,
            std::enable_if_t<
                std::is_same<T, char1>::value
                || std::is_same<T, double1>::value
                || std::is_same<T, float1>::value
                || std::is_same<T, int1>::value
                || std::is_same<T, long1>::value
                || std::is_same<T, longlong1>::value
                || std::is_same<T, short1>::value
                || std::is_same<T, uchar1>::value
                || std::is_same<T, uint1>::value
                || std::is_same<T, ulong1>::value
                || std::is_same<T, ulonglong1>::value
                || std::is_same<T, ushort1>::value
            >>
        {
            using type = DimInt<1u>;
        };
        //#############################################################################
        //! The CUDA vectors 2D dimension get trait specialization.
        template<
            typename T>
        struct DimType<
            T,
            std::enable_if_t<
                std::is_same<T, char2>::value
                || std::is_same<T, double2>::value
                || std::is_same<T, float2>::value
                || std::is_same<T, int2>::value
                || std::is_same<T, long2>::value
                || std::is_same<T, longlong2>::value
                || std::is_same<T, short2>::value
                || std::is_same<T, uchar2>::value
                || std::is_same<T, uint2>::value
                || std::is_same<T, ulong2>::value
                || std::is_same<T, ulonglong2>::value
                || std::is_same<T, ushort2>::value
            >>
        {
            using type = DimInt<2u>;
        };
        //#############################################################################
        //! The CUDA vectors 3D dimension get trait specialization.
        template<
            typename T>
        struct DimType<
            T,
            std::enable_if_t<
                std::is_same<T, char3>::value
                || std::is_same<T, dim3>::value
                || std::is_same<T, double3>::value
                || std::is_same<T, float3>::value
                || std::is_same<T, int3>::value
                || std::is_same<T, long3>::value
                || std::is_same<T, longlong3>::value
                || std::is_same<T, short3>::value
                || std::is_same<T, uchar3>::value
                || std::is_same<T, uint3>::value
                || std::is_same<T, ulong3>::value
                || std::is_same<T, ulonglong3>::value
                || std::is_same<T, ushort3>::value
#if BOOST_COMP_CLANG_CUDA
                || std::is_same<T, __cuda_builtin_threadIdx_t>::value
                || std::is_same<T, __cuda_builtin_blockIdx_t>::value
                || std::is_same<T, __cuda_builtin_blockDim_t>::value
                || std::is_same<T, __cuda_builtin_gridDim_t>::value
#endif
            >>
        {
            using type = DimInt<3u>;
        };
        //#############################################################################
        //! The CUDA vectors 4D dimension get trait specialization.
        template<
            typename T>
        struct DimType<
            T,
            std::enable_if_t<
                std::is_same<T, char4>::value
                || std::is_same<T, double4>::value
                || std::is_same<T, float4>::value
                || std::is_same<T, int4>::value
                || std::is_same<T, long4>::value
                || std::is_same<T, longlong4>::value
                || std::is_same<T, short4>::value
                || std::is_same<T, uchar4>::value
                || std::is_same<T, uint4>::value
                || std::is_same<T, ulong4>::value
                || std::is_same<T, ulonglong4>::value
                || std::is_same<T, ushort4>::value
            >>
        {
            using type = DimInt<4u>;
        };

        //#############################################################################
        //! The CUDA vectors elem type trait specialization.
        template<
            typename T>
        struct ElemType<
            T,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<T>::value>>
        {
            using type = decltype(std::declval<T>().x);
        };
    }
    namespace extent
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            template<
                typename TExtent>
            struct GetExtent<
                DimInt<Dim<TExtent>::value - 1u>,
                TExtent,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 1)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                {
                    return extent.x;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            template<
                typename TExtent>
            struct GetExtent<
                DimInt<Dim<TExtent>::value - 2u>,
                TExtent,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 2)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                {
                    return extent.y;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            template<
                typename TExtent>
            struct GetExtent<
                DimInt<Dim<TExtent>::value - 3u>,
                TExtent,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 3)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                {
                    return extent.z;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent get trait specialization.
            template<
                typename TExtent>
            struct GetExtent<
                DimInt<Dim<TExtent>::value - 4u>,
                TExtent,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 4)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto getExtent(
                    TExtent const & extent)
                {
                    return extent.w;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                DimInt<Dim<TExtent>::value - 1u>,
                TExtent,
                TExtentVal,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 1)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.x = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                DimInt<Dim<TExtent>::value - 2u>,
                TExtent,
                TExtentVal,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 2)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.y = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                DimInt<Dim<TExtent>::value - 3u>,
                TExtent,
                TExtentVal,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 3)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.z = extentVal;
                }
            };
            //#############################################################################
            //! The CUDA vectors extent set trait specialization.
            template<
                typename TExtent,
                typename TExtentVal>
            struct SetExtent<
                DimInt<Dim<TExtent>::value - 4u>,
                TExtent,
                TExtentVal,
                std::enable_if_t<
                    cuda::traits::IsCudaBuiltInType<TExtent>::value
                    && (Dim<TExtent>::value >= 4)>>
            {
                ALPAKA_NO_HOST_ACC_WARNING
                ALPAKA_FN_HOST_ACC static auto setExtent(
                    TExtent const & extent,
                    TExtentVal const & extentVal)
                -> void
                {
                    extent.w = extentVal;
                }
            };
        }
    }
    namespace traits
    {
        //#############################################################################
        //! The CUDA vectors offset get trait specialization.
        template<
            typename TOffsets>
        struct GetOffset<
            DimInt<Dim<TOffsets>::value - 1u>,
            TOffsets,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 1)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto getOffset(
                TOffsets const & offsets)
            {
                return offsets.x;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset get trait specialization.
        template<
            typename TOffsets>
        struct GetOffset<
            DimInt<Dim<TOffsets>::value - 2u>,
            TOffsets,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 2)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto getOffset(
                TOffsets const & offsets)
            {
                return offsets.y;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset get trait specialization.
        template<
            typename TOffsets>
        struct GetOffset<
            DimInt<Dim<TOffsets>::value - 3u>,
            TOffsets,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 3)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto getOffset(
                TOffsets const & offsets)
            {
                return offsets.z;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset get trait specialization.
        template<
            typename TOffsets>
        struct GetOffset<
            DimInt<Dim<TOffsets>::value - 4u>,
            TOffsets,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 4)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto getOffset(
                TOffsets const & offsets)
            {
                return offsets.w;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset set trait specialization.
        template<
            typename TOffsets,
            typename TOffset>
        struct SetOffset<
            DimInt<Dim<TOffsets>::value - 1u>,
            TOffsets,
            TOffset,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 1)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto setOffset(
                TOffsets const & offsets,
                TOffset const & offset)
            -> void
            {
                offsets.x = offset;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset set trait specialization.
        template<
            typename TOffsets,
            typename TOffset>
        struct SetOffset<
            DimInt<Dim<TOffsets>::value - 2u>,
            TOffsets,
            TOffset,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 2)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto setOffset(
                TOffsets const & offsets,
                TOffset const & offset)
            -> void
            {
                offsets.y = offset;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset set trait specialization.
        template<
            typename TOffsets,
            typename TOffset>
        struct SetOffset<
            DimInt<Dim<TOffsets>::value - 3u>,
            TOffsets,
            TOffset,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 3)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto setOffset(
                TOffsets const & offsets,
                TOffset const & offset)
            -> void
            {
                offsets.z = offset;
            }
        };
        //#############################################################################
        //! The CUDA vectors offset set trait specialization.
        template<
            typename TOffsets,
            typename TOffset>
        struct SetOffset<
            DimInt<Dim<TOffsets>::value - 4u>,
            TOffsets,
            TOffset,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TOffsets>::value
                && (Dim<TOffsets>::value >= 4)>>
        {
            ALPAKA_NO_HOST_ACC_WARNING
            ALPAKA_FN_HOST_ACC static auto setOffset(
                TOffsets const & offsets,
                TOffset const & offset)
            -> void
            {
                offsets.w = offset;
            }
        };

        //#############################################################################
        //! The CUDA vectors idx type trait specialization.
        template<
            typename TIdx>
        struct IdxType<
            TIdx,
            std::enable_if_t<
                cuda::traits::IsCudaBuiltInType<TIdx>::value>>
        {
            using type = std::size_t;
        };
    }
}

#include <alpaka/core/UniformCudaHip.hpp>

#endif
