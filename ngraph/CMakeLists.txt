# Copyright (C) 2018-2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

set(NGRAPH_INCLUDE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

project (ngraph)

#-----------------------------------------------------------------------------------------------
# Installation logic...
#-----------------------------------------------------------------------------------------------

set(NGRAPH_TARGETS_FILE "${CMAKE_CURRENT_BINARY_DIR}/ngraphTargets.cmake")

add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

#-----------------------------------------------------------------------------------------------
# External projects install directory
#-----------------------------------------------------------------------------------------------

add_subdirectory(core)

add_subdirectory(frontend)
add_subdirectory(test)
