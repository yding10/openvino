// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <vector>

#include "behavior/infer_request/wait.hpp"
#include "ie_plugin_config.hpp"

using namespace BehaviorTestsDefinitions;
namespace {
    const std::vector<std::map<std::string, std::string>> configs = {
            {},
            {{InferenceEngine::PluginConfigParams::KEY_CPU_THROUGHPUT_STREAMS, InferenceEngine::PluginConfigParams::CPU_THROUGHPUT_AUTO}},
            {{InferenceEngine::PluginConfigParams::KEY_CPU_THROUGHPUT_STREAMS, "0"}, {InferenceEngine::PluginConfigParams::KEY_CPU_THREADS_NUM, "1"}}
    };

    const std::vector<std::map<std::string, std::string>> Multiconfigs = {
            {{ MULTI_CONFIG_KEY(DEVICE_PRIORITIES) , CommonTestUtils::DEVICE_CPU}}
    };

    const std::vector<std::map<std::string, std::string>> Autoconfigs = {
            {{ AUTO_CONFIG_KEY(DEVICE_LIST) , CommonTestUtils::DEVICE_CPU}}
    };

    INSTANTIATE_TEST_SUITE_P(smoke_BehaviorTests, InferRequestWaitTests,
                            ::testing::Combine(
                                    ::testing::Values(CommonTestUtils::DEVICE_CPU),
                                    ::testing::ValuesIn(configs)),
                             InferRequestWaitTests::getTestCaseName);

    INSTANTIATE_TEST_SUITE_P(smoke_Multi_BehaviorTests, InferRequestWaitTests,
                            ::testing::Combine(
                                    ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                    ::testing::ValuesIn(Multiconfigs)),
                             InferRequestWaitTests::getTestCaseName);

    INSTANTIATE_TEST_SUITE_P(smoke_Auto_BehaviorTests, InferRequestWaitTests,
                            ::testing::Combine(
                                    ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                    ::testing::ValuesIn(Autoconfigs)),
                             InferRequestWaitTests::getTestCaseName);

}  // namespace
