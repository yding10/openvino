// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <onnx/onnx_pb.h>

#include "gtest/gtest.h"
#include "util/test_control.hpp"

using namespace ngraph;

static std::string s_manifest = "${MANIFEST}";

NGRAPH_TEST(onnx, check_ir_version_support) {
    // It appears you've changed the ONNX library version used by nGraph. Please update the value
    // tested below to make sure it equals the current IR_VERSION enum value defined in ONNX headers
    //
    // You should also check the onnx_common/src/onnx_model_validator.cpp file and make sure that
    // the details::onnx::is_correct_onnx_field() handles any new fields added in the new release
    // of the ONNX library. Make sure to update the "Field" enum and the function mentioned above.
    //
    // The last step is to also update the details::onnx::contains_onnx_model_keys() function
    // in the same file to make sure that prototxt format validation also covers the changes in ONNX
    EXPECT_EQ(ONNX_NAMESPACE::Version::IR_VERSION, 7)
        << "The IR_VERSION defined in ONNX does not match the version that OpenVINO supports. "
           "Please check the source code of this test for details and explanation how to proceed.";
}
