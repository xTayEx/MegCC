/**
 * \file
 * compiler/lib/KernelGen/Arm/Arm64/ConvKernel/Fp32/Winograd/WinogradF23Strategy4x16MK4.h
 *
 * This file is part of MegCC, a deep learning compiler developed by Megvii.
 *
 * \copyright Copyright (c) 2021-2022 Megvii Inc. All rights reserved.
 */
#pragma once
#include <string>
#include "Arm/ArmCommon/ConvKernel/Fp32/Winograd/WinogradCommon.h"
#include "Common/ConvKernel.h"
#include "Utils/StringTemplate.h"
#include "compiler/KernelGen/KernelGen.h"
namespace megcc {
namespace KernelGen {
namespace Arm64 {

class WinogradF23Strategy4x16MK4 : public ArmCommon::WinogradStrategyBase {
public:
    uint32_t GetKernelSize() override { return 3; }
    uint32_t GetOutputBlockSize() override { return 2; }
    std::string DependMatmulSymbol() override;
    std::string WeightTrans(const std::vector<std::string>& strs) override;
    std::string InputFeatureTrans(const std::vector<std::string>& strs) override;
    std::string BatchedMatMul(const std::vector<std::string>& strs) override;
    std::string OutputFeatureTrans(
            const std::vector<std::string>& strs, TContext*) override;
};

}  // namespace Arm64
}  // namespace KernelGen
}  // namespace megcc

// vim: syntax=cpp.doxygen
