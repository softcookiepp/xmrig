if (BUILD_STATIC AND XMRIG_OS_UNIX AND WITH_VULKAN)
    message(WARNING "Vulkan backend is not compatible with static build, use -DWITH_VULKAN=OFF to suppress this warning")

    set(WITH_VULKAN OFF)
endif()

if (WITH_VULKAN)
    add_definitions(/DXMRIG_FEATURE_VULKAN /DCL_USE_DEPRECATED_OPENCL_1_2_APIS_CLSPV)

    set(HEADERS_BACKEND_VULKAN
        src/backend/vulkan/cl/OclSource.h
        src/backend/vulkan/interfaces/IOclRunner.h
        src/backend/vulkan/kernels/Cn0Kernel.h
        src/backend/vulkan/kernels/Cn1Kernel.h
        src/backend/vulkan/kernels/Cn2Kernel.h
        src/backend/vulkan/kernels/CnBranchKernel.h
        src/backend/vulkan/OclBackend.h
        src/backend/vulkan/OclCache.h
        src/backend/vulkan/OclConfig.h
        src/backend/vulkan/OclConfig_gen.h
        src/backend/vulkan/OclGenerator.h
        src/backend/vulkan/OclLaunchData.h
        src/backend/vulkan/OclThread.h
        src/backend/vulkan/OclThreads.h
        src/backend/vulkan/OclWorker.h
        src/backend/vulkan/runners/OclBaseRunner.h
        src/backend/vulkan/runners/OclCnRunner.h
        src/backend/vulkan/runners/tools/OclCnR.h
        src/backend/vulkan/runners/tools/OclSharedData.h
        src/backend/vulkan/runners/tools/OclSharedState.h
        src/backend/vulkan/wrappers/OclContext.h
        src/backend/vulkan/wrappers/OclDevice.h
        src/backend/vulkan/wrappers/OclError.h
        src/backend/vulkan/wrappers/OclKernel.h
        src/backend/vulkan/wrappers/OclLib.h
        src/backend/vulkan/wrappers/OclPlatform.h
        src/backend/vulkan/wrappers/OclVendor.h
        )

    set(SOURCES_BACKEND_VULKAN
        src/backend/vulkan/cl/OclSource.cpp
        src/backend/vulkan/generators/ocl_generic_cn_generator.cpp
        src/backend/vulkan/generators/ocl_vega_cn_generator.cpp
        src/backend/vulkan/kernels/Cn0Kernel.cpp
        src/backend/vulkan/kernels/Cn1Kernel.cpp
        src/backend/vulkan/kernels/Cn2Kernel.cpp
        src/backend/vulkan/kernels/CnBranchKernel.cpp
        src/backend/vulkan/OclBackend.cpp
        src/backend/vulkan/OclCache.cpp
        src/backend/vulkan/OclConfig.cpp
        src/backend/vulkan/OclLaunchData.cpp
        src/backend/vulkan/OclThread.cpp
        src/backend/vulkan/OclThreads.cpp
        src/backend/vulkan/OclWorker.cpp
        src/backend/vulkan/runners/OclBaseRunner.cpp
        src/backend/vulkan/runners/OclCnRunner.cpp
        src/backend/vulkan/runners/tools/OclCnR.cpp
        src/backend/vulkan/runners/tools/OclSharedData.cpp
        src/backend/vulkan/runners/tools/OclSharedState.cpp
        src/backend/vulkan/wrappers/OclContext.cpp
        src/backend/vulkan/wrappers/OclDevice.cpp
        src/backend/vulkan/wrappers/OclError.cpp
        src/backend/vulkan/wrappers/OclKernel.cpp
        src/backend/vulkan/wrappers/OclLib.cpp
        src/backend/vulkan/wrappers/OclPlatform.cpp
        )

    if (XMRIG_OS_APPLE)
		message(WARNING "Vulkan backend is likely not compatible with Apple as of now, sorry for any problems in advance")

        #add_definitions(/DCL_TARGET_OPENCL_VERSION=120)
        #list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/wrappers/OclDevice_mac.cpp)
    elseif (WITH_OPENCL_VERSION)
        add_definitions(/DCL_TARGET_OPENCL_VERSION=${WITH_OPENCL_VERSION})
    endif()

    if (WIN32)
        list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/OclCache_win.cpp)
    else()
        list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/OclCache_unix.cpp)
    endif()

    if (WITH_RANDOMX)
        list(APPEND HEADERS_BACKEND_VULKAN
             src/backend/vulkan/kernels/rx/Blake2bHashRegistersKernel.h
             src/backend/vulkan/kernels/rx/Blake2bInitialHashBigKernel.h
             src/backend/vulkan/kernels/rx/Blake2bInitialHashDoubleKernel.h
             src/backend/vulkan/kernels/rx/Blake2bInitialHashKernel.h
             src/backend/vulkan/kernels/rx/ExecuteVmKernel.h
             src/backend/vulkan/kernels/rx/FillAesKernel.h
             src/backend/vulkan/kernels/rx/FindSharesKernel.h
             src/backend/vulkan/kernels/rx/HashAesKernel.cpp
             src/backend/vulkan/kernels/rx/InitVmKernel.h
             src/backend/vulkan/kernels/rx/RxJitKernel.h
             src/backend/vulkan/kernels/rx/RxRunKernel.h
             src/backend/vulkan/runners/OclRxBaseRunner.h
             src/backend/vulkan/runners/OclRxJitRunner.h
             src/backend/vulkan/runners/OclRxVmRunner.h
             )

        list(APPEND SOURCES_BACKEND_VULKAN
             src/backend/vulkan/generators/ocl_generic_rx_generator.cpp
             src/backend/vulkan/kernels/rx/Blake2bHashRegistersKernel.cpp
             src/backend/vulkan/kernels/rx/Blake2bInitialHashBigKernel.cpp
             src/backend/vulkan/kernels/rx/Blake2bInitialHashDoubleKernel.cpp
             src/backend/vulkan/kernels/rx/Blake2bInitialHashKernel.cpp
             src/backend/vulkan/kernels/rx/ExecuteVmKernel.cpp
             src/backend/vulkan/kernels/rx/FillAesKernel.cpp
             src/backend/vulkan/kernels/rx/FindSharesKernel.cpp
             src/backend/vulkan/kernels/rx/HashAesKernel.cpp
             src/backend/vulkan/kernels/rx/InitVmKernel.cpp
             src/backend/vulkan/kernels/rx/RxJitKernel.cpp
             src/backend/vulkan/kernels/rx/RxRunKernel.cpp
             src/backend/vulkan/runners/OclRxBaseRunner.cpp
             src/backend/vulkan/runners/OclRxJitRunner.cpp
             src/backend/vulkan/runners/OclRxVmRunner.cpp
             )
    endif()

    if (WITH_KAWPOW)
        list(APPEND HEADERS_BACKEND_VULKAN
             src/backend/vulkan/kernels/kawpow/KawPow_CalculateDAGKernel.h
             src/backend/vulkan/runners/OclKawPowRunner.h
             src/backend/vulkan/runners/tools/OclKawPow.h
             )

        list(APPEND SOURCES_BACKEND_VULKAN
             src/backend/vulkan/generators/ocl_generic_kawpow_generator.cpp
             src/backend/vulkan/kernels/kawpow/KawPow_CalculateDAGKernel.cpp
             src/backend/vulkan/runners/OclKawPowRunner.cpp
             src/backend/vulkan/runners/tools/OclKawPow.cpp
             )
    endif()

    if (WITH_STRICT_CACHE)
        add_definitions(/DXMRIG_STRICT_OPENCL_CACHE)
    else()
        remove_definitions(/DXMRIG_STRICT_OPENCL_CACHE)
    endif()

    if (WITH_INTERLEAVE_DEBUG_LOG)
        add_definitions(/DXMRIG_INTERLEAVE_DEBUG)
    endif()

    if (WITH_ADL AND (XMRIG_OS_WIN OR XMRIG_OS_LINUX))
        add_definitions(/DXMRIG_FEATURE_ADL)

        list(APPEND HEADERS_BACKEND_VULKAN
             src/backend/vulkan/wrappers/AdlHealth.h
             src/backend/vulkan/wrappers/AdlLib.h
             )

        if (XMRIG_OS_WIN)
            list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/wrappers/AdlLib.cpp)
        else()
            list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/wrappers/AdlLib_linux.cpp)
        endif()
    else()
       remove_definitions(/DXMRIG_FEATURE_ADL)
    endif()
else()
    remove_definitions(/DXMRIG_FEATURE_VULKAN)
    remove_definitions(/DXMRIG_FEATURE_ADL)

    set(HEADERS_BACKEND_VULKAN "")
    set(SOURCES_BACKEND_VULKAN "")
endif()
