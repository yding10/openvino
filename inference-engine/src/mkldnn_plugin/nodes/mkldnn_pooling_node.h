// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <ie_common.h>
#include <mkldnn_node.h>
#include <string>
#include <memory>
#include <vector>

namespace MKLDNNPlugin {

class MKLDNNPoolingNode : public MKLDNNNode {
public:
    MKLDNNPoolingNode(const std::shared_ptr<ngraph::Node>& op, const mkldnn::engine& eng, MKLDNNWeightsSharing::Ptr &cache);

    void createDescriptor(const std::vector<MemoryDescPtr>& inputDesc,
                          const std::vector<MemoryDescPtr>& outputDesc) override;
    std::vector<mkldnn::memory::format_tag> getAvailableFormatsForDims(const Shape &dims) const override;
    void getSupportedDescriptors() override;
    void initSupportedPrimitiveDescriptors() override;
    void initDescriptor(const NodeConfig& config) override;
    void createPrimitive() override;
    bool created() const override;
    bool canBeInPlace() const override {
        return false;
    }

    static bool isSupportedOperation(const std::shared_ptr<const ngraph::Node>& op, std::string& errorMessage) noexcept;

private:
    void setPostOps(mkldnn::primitive_attr &attr, bool initWeights = false);

    bool exclude_pad = false;
    std::vector<ptrdiff_t> stride;
    std::vector<ptrdiff_t> kernel;

    /// Effective padding. Used to define correct output shape by MKLDNN
    /// reshape formula: (iw - kernel + pad_l + pad_r) / strides[i - 2] + 1
    /// should be passed into pooling desc constructor.
    std::vector<ptrdiff_t> effective_pad_begin;
    std::vector<ptrdiff_t> effective_pad_end;

    /// Effective pad value. Describe how much zero element added to input
    /// data tensor. May be less than "Effective padding" values.
    /// If pooling window is out of this padding, the region of averaging
    /// is decreased.
    std::vector<ptrdiff_t> data_pad_begin;
    std::vector<ptrdiff_t> data_pad_end;

    InferenceEngine::Precision inputPrecision = InferenceEngine::Precision::FP32;
    InferenceEngine::Precision outputPrecision = InferenceEngine::Precision::FP32;
};

}  // namespace MKLDNNPlugin

