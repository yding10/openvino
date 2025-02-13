// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <vector>
#include <string>

#include "conformance.hpp"

#include "functional_test_utils/skip_tests_config.hpp"

namespace ConformanceTests {
const char *targetDevice = "";
const char *targetPluginName = "";

std::vector<std::string> IRFolderPaths = {};
std::vector<std::string> disabledTests = {};
}

std::vector<std::string> disabledTestPatterns() {
    return ConformanceTests::disabledTests;
}
