if (BUILD_STATIC AND XMRIG_OS_UNIX AND WITH_VULKAN)
    message(WARNING "Vulkan backend is not compatible with static build, use -DWITH_VULKAN=OFF to suppress this warning")

    set(WITH_VULKAN OFF)
endif()

if (WITH_VULKAN)
    add_definitions(/DXMRIG_FEATURE_VULKAN /DCL_USE_DEPRECATED_OPENCL_1_2_APIS_CLSPV)

    set(HEADERS_BACKEND_VULKAN
        src/backend/vulkan/cl/VkSource.h
        src/backend/vulkan/interfaces/IVkRunner.h
        src/backend/vulkan/kernels/Cn0Kernel.h
        src/backend/vulkan/kernels/Cn1Kernel.h
        src/backend/vulkan/kernels/Cn2Kernel.h
        src/backend/vulkan/kernels/CnBranchKernel.h
        src/backend/vulkan/VkBackend.h
        src/backend/vulkan/VkCache.h
        src/backend/vulkan/VkConfig.h
        src/backend/vulkan/VkConfig_gen.h
        src/backend/vulkan/VkGenerator.h
        src/backend/vulkan/VkLaunchData.h
        src/backend/vulkan/VkThread.h
        src/backend/vulkan/VkThreads.h
        src/backend/vulkan/VkWorker.h
        src/backend/vulkan/runners/VkBaseRunner.h
        src/backend/vulkan/runners/VkCnRunner.h
        src/backend/vulkan/runners/tools/VkCnR.h
        src/backend/vulkan/runners/tools/VkSharedData.h
        src/backend/vulkan/runners/tools/VkSharedState.h
        src/backend/vulkan/wrappers/VkContext.h
        src/backend/vulkan/wrappers/VkDevice.h
        src/backend/vulkan/wrappers/VkError.h
        src/backend/vulkan/wrappers/VkKernel.h
        src/backend/vulkan/wrappers/VkLib.h
        src/backend/vulkan/wrappers/VkPlatform.h
        src/backend/vulkan/wrappers/VkVendor.h
        )

    set(SOURCES_BACKEND_VULKAN
        src/backend/vulkan/cl/VkSource.cpp
        src/backend/vulkan/generators/ocl_generic_cn_generator.cpp
        src/backend/vulkan/generators/ocl_vega_cn_generator.cpp
        src/backend/vulkan/kernels/Cn0Kernel.cpp
        src/backend/vulkan/kernels/Cn1Kernel.cpp
        src/backend/vulkan/kernels/Cn2Kernel.cpp
        src/backend/vulkan/kernels/CnBranchKernel.cpp
        src/backend/vulkan/VkBackend.cpp
        src/backend/vulkan/VkCache.cpp
        src/backend/vulkan/VkConfig.cpp
        src/backend/vulkan/VkLaunchData.cpp
        src/backend/vulkan/VkThread.cpp
        src/backend/vulkan/VkThreads.cpp
        src/backend/vulkan/VkWorker.cpp
        src/backend/vulkan/runners/VkBaseRunner.cpp
        src/backend/vulkan/runners/VkCnRunner.cpp
        src/backend/vulkan/runners/tools/VkCnR.cpp
        src/backend/vulkan/runners/tools/VkSharedData.cpp
        src/backend/vulkan/runners/tools/VkSharedState.cpp
        src/backend/vulkan/wrappers/VkContext.cpp
        src/backend/vulkan/wrappers/VkDevice.cpp
        src/backend/vulkan/wrappers/VkError.cpp
        src/backend/vulkan/wrappers/VkKernel.cpp
        src/backend/vulkan/wrappers/VkLib.cpp
        src/backend/vulkan/wrappers/VkPlatform.cpp
        )

    if (XMRIG_OS_APPLE)
		message(WARNING "Vulkan backend is likely not compatible with Apple as of now, sorry for any problems in advance")

        #add_definitions(/DCL_TARGET_OPENCL_VERSION=120)
        #list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/wrappers/VkDevice_mac.cpp)
    elseif (WITH_OPENCL_VERSION)
        add_definitions(/DCL_TARGET_OPENCL_VERSION=${WITH_OPENCL_VERSION})
    endif()

    if (WIN32)
        list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/VkCache_win.cpp)
    else()
        list(APPEND SOURCES_BACKEND_VULKAN src/backend/vulkan/VkCache_unix.cpp)
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
             src/backend/vulkan/runners/VkRxBaseRunner.h
             src/backend/vulkan/runners/VkRxJitRunner.h
             src/backend/vulkan/runners/VkRxVmRunner.h
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
             src/backend/vulkan/runners/VkRxBaseRunner.cpp
             src/backend/vulkan/runners/VkRxJitRunner.cpp
             src/backend/vulkan/runners/VkRxVmRunner.cpp
             )
    endif()

    if (WITH_KAWPOW)
        list(APPEND HEADERS_BACKEND_VULKAN
             src/backend/vulkan/kernels/kawpow/KawPow_CalculateDAGKernel.h
             src/backend/vulkan/runners/VkKawPowRunner.h
             src/backend/vulkan/runners/tools/VkKawPow.h
             )

        list(APPEND SOURCES_BACKEND_VULKAN
             src/backend/vulkan/generators/ocl_generic_kawpow_generator.cpp
             src/backend/vulkan/kernels/kawpow/KawPow_CalculateDAGKernel.cpp
             src/backend/vulkan/runners/VkKawPowRunner.cpp
             src/backend/vulkan/runners/tools/VkKawPow.cpp
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
