// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "engines_util/execute_tools.hpp"
#include "engines_util/test_case.hpp"
#include "engines_util/test_engines.hpp"
#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "ngraph/runtime/tensor.hpp"
#include "runtime/backend.hpp"
#include "util/all_close.hpp"
#include "util/all_close_f.hpp"
#include "util/ndarray.hpp"
#include "util/test_control.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

using TestEngine = test::ENGINE_CLASS_NAME(${BACKEND_NAME});

static void GroupConvolutionTest(const std::vector<float>& inputs,
                                 const Shape inputs_shape,
                                 const std::vector<float>& filters,
                                 const Shape filter_shape,
                                 const std::vector<float>& outputs,
                                 const Shape outputs_shape,
                                 const Strides& strides,
                                 const CoordinateDiff& padding,
                                 const Strides& dilations) {
    const CoordinateDiff pads_begin{padding};
    const CoordinateDiff pads_end{padding};
    const op::PadType auto_pad{op::PadType::EXPLICIT};

    auto inputs_param = make_shared<op::Parameter>(element::f32, inputs_shape);
    auto filters_param = make_shared<op::Parameter>(element::f32, filter_shape);
    auto conv = make_shared<op::v1::GroupConvolution>(inputs_param,
                                                      filters_param,
                                                      strides,
                                                      pads_begin,
                                                      pads_end,
                                                      dilations,
                                                      auto_pad);
    auto f = make_shared<Function>(conv, ParameterVector{inputs_param, filters_param});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<float>(inputs);
    test_case.add_input<float>(filters);
    test_case.add_expected_output<float>(outputs_shape, outputs);
    test_case.run();
}

// --------------------- 1D group convolution ------------------------------------------
// clang-format off
NGRAPH_TEST(${BACKEND_NAME}, group_convolution_1D_1group_1batch_1channel)
{
    const Strides strides{1};
    const CoordinateDiff padding{0};
    const Strides dilations{1};

    const Shape inputs_shape{1, 1, 6};
    const std::vector<float> inputs{1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f};

    const Shape filter_shape{1, 1, 1, 3};
    const std::vector<float> filters{2.0f, 0.0f, 1.0f};

    const Shape outputs_shape{1, 1, 4};
    const std::vector<float> outputs{5.0f, 6.0f, 7.0f, 2.0f};

    GroupConvolutionTest(inputs, inputs_shape, filters, filter_shape, outputs, outputs_shape,
                    strides, padding, dilations);
}

NGRAPH_TEST(${BACKEND_NAME}, group_convolution_1D_2group_1batch_2channel)
{
    const Strides strides{1};
    const CoordinateDiff padding{0};
    const Strides dilations{1};

    const Shape inputs_shape{1, 2, 6};
    const std::vector<float> inputs{1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f,
                                    1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f};

    const Shape filter_shape{2, 1, 1, 3};
    const std::vector<float> filters{1.0f, 0.0f, 3.0f,
                                     3.0f, 0.0f, 1.0f};

    const Shape outputs_shape{1, 2, 4};
    const std::vector<float> outputs{10.0f, 3.0f, 6.0f, 6.0f,
                                     6.0f, 9.0f, 10.0f, 2.0f};

    GroupConvolutionTest(inputs, inputs_shape, filters, filter_shape, outputs, outputs_shape,
                    strides, padding, dilations);
}

NGRAPH_TEST(${BACKEND_NAME}, group_convolution_1D_2group_1batch_2_filters_2channel)
{
    const Strides strides{1};
    const CoordinateDiff padding{0};
    const Strides dilations{1};

    const Shape inputs_shape{1, 2, 6};
    const std::vector<float> inputs{1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f,
                                    -1.0f, -3.0f, -3.0f, 0.0f, 1.0f, 2.0f};

    const Shape filter_shape{2, 2, 1, 3};
    const std::vector<float> filters{1.0f, 0.0f, 3.0f,
                                     3.0f, 0.0f, 1.0f,
                                     -3.0f, 0.0f, 1.0f,
                                     3.0f, 2.0f, -1.0f};

    const Shape outputs_shape{1, 4, 4};
    const std::vector<float> outputs{10.0f, 3.0f, 6.0f, 6.0f,
                                     6.0f, 9.0f, 10.0f, 2.0f,
                                     0.0f, 9.0f, 10.0f, 2.0f,
                                     -6.0f, -15.0f, -10.0f, 0.0f};

    GroupConvolutionTest(inputs, inputs_shape, filters, filter_shape, outputs, outputs_shape,
                    strides, padding, dilations);
}

NGRAPH_TEST(${BACKEND_NAME}, group_convolution_1D_2group_2batch_2channel)
{
    const Strides strides{1};
    const CoordinateDiff padding{0};
    const Strides dilations{1};

    const Shape inputs_shape{2, 2, 6};
    const std::vector<float> inputs{// -- batch 1 -- 
                                    1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f,
                                    1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f,
                                    // -- batch 2 -- 
                                    1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f,
                                    1.0f, 3.0f, 3.0f, 0.0f, 1.0f, 2.0f};

    const Shape filter_shape{2, 1, 1, 3};
    const std::vector<float> filters{1.0f, 0.0f, 3.0f,
                                     3.0f, 0.0f, 1.0f};

    const Shape outputs_shape{2, 2, 4};
    const std::vector<float> outputs{10.0f, 3.0f, 6.0f, 6.0f,
                                     6.0f, 9.0f, 10.0f, 2.0f,
                                     10.0f, 3.0f, 6.0f, 6.0f,
                                     6.0f, 9.0f, 10.0f, 2.0f};

    GroupConvolutionTest(inputs, inputs_shape, filters, filter_shape, outputs, outputs_shape,
                    strides, padding, dilations);
}
// // clang-format on
