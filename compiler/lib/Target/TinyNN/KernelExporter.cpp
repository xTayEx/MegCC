/**
 * \file compiler/lib/Target/TinyNN/KernelExporter.cpp
 *
 * This file is part of MegCC, a deep learning compiler developed by Megvii.
 *
 * \copyright Copyright (c) 2021-2022 Megvii Inc. All rights reserved.
 */

#include <set>
#include <sstream>
#include <vector>
#include "compiler/Common/Logger.h"
#include "compiler/Common/Version.h"
#include "compiler/KernelGen/KernelGen.h"
#include "compiler/Target/TinyNN/export.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

using namespace megcc;
namespace mlir {
namespace {
struct KernelExporterHelper {
    static constexpr const char* headerTemplate = R"(
// this file should be generated by MegCC
#ifndef KERNEL_H
#define KERNEL_H

#include "data_struct.h"
#include "stdint.h"
#include "string.h"

#define NR_KERNELS ({0})
#define NR_INIT ({1})
#define NR_DEDUCE_SHAPE ({2})
#define NR_WORKSPACE ({3})

#define KERNEL_MARK_USDED_VAR(x) (void)(x);

extern KernelFunc kernels[NR_KERNELS];

extern InitFunc init_kernels[NR_INIT];
extern WorkspaceFunc workspace_func[NR_WORKSPACE];
extern DeduceFunc deduce_func[NR_DEDUCE_SHAPE];
//! declare all the kernel here in C

{4}

void load_kernel_init_function();

#endif
// vim: syntax=cpp.doxygen
)";

    static constexpr const char* kernelRegistrationTemplate = R"(
// this file should be generated by MegCC

#include "kernels.h"

KernelFunc kernels[NR_KERNELS];
InitFunc init_kernels[NR_INIT];
WorkspaceFunc workspace_func[NR_WORKSPACE];
DeduceFunc deduce_func[NR_DEDUCE_SHAPE];

//! not thread safe
void load_kernel_init_function() {{
    static int done = 0;
    if (done) {{
        return ;
    }
    {0}
    done = 1;
    return ;
}
)";

    static constexpr const char* singleFuncUnitTemplate = R"(
// this file should be generated by MegCC

#include "kernels.h"

{0}

{1}

{2}
)";
    static constexpr const char* nodepFuncUnitTemplate = R"(
// this file should be generated by MegCC


{0}

{1}

{2}
)";

    static void genHeader(std::string save_path, KernelExporter::Config config) {
        std::error_code EC;
        llvm::raw_fd_stream os(save_path, EC);
        std::string declStr;
        llvm::raw_string_ostream decl(declStr);
        auto funcDecl = [&decl](auto&& f, auto&& guard_begin, auto&& guard_end) {
            decl << guard_begin;
            decl << KernelGen::GenCommonRet() << " " << f << ";\n";
            decl << guard_end;
        };
        for (auto&& i : config.kernels) {
            funcDecl(i.signature, i.guard_begin, i.guard_end);
        }
        for (auto&& i : config.inits) {
            funcDecl(i.signature, i.guard_begin, i.guard_end);
        }
        for (auto&& i : config.workspaces) {
            funcDecl(i.signature, i.guard_begin, i.guard_end);
        }
        for (auto&& i : config.deduce_shape) {
            funcDecl(i.signature, i.guard_begin, i.guard_end);
        }
        os << llvm::formatv(
                headerTemplate, config.kernels.size(), config.inits.size(),
                config.deduce_shape.size(), config.workspaces.size(), decl.str());
    }
    static void genInstSwitch(
            std::string save_path, std::set<std::string>& inst_type_set) {
        std::error_code EC;
        llvm::raw_fd_stream os(save_path, EC);
        os << "// this file should be generated by MegCC\n";
        for (auto& inst_type : inst_type_set) {
            os << "#define ENABLE_INST_" << inst_type << " 1 \n";
        }
    }
    static void genKernelRegistration(
            std::string save_path, KernelExporter::Config config) {
        std::error_code EC;
        llvm::raw_fd_stream os(save_path, EC);
        std::string regStr;
        llvm::raw_string_ostream reg(regStr);
        for (size_t i = 0; i < config.kernels.size(); ++i) {
            reg << llvm::formatv(
                    "{0}kernels[{1}] = {2};{3}\n", config.kernels[i].guard_begin, i,
                    config.kernels[i].symbol, config.kernels[i].guard_end);
        }
        for (size_t i = 0; i < config.inits.size(); ++i) {
            reg << llvm::formatv(
                    "{0}init_kernels[{1}] = {2};{3}\n", config.kernels[i].guard_begin,
                    i, config.inits[i].symbol, config.kernels[i].guard_end);
        }
        for (size_t i = 0; i < config.workspaces.size(); ++i) {
            reg << llvm::formatv(
                    "{0}workspace_func[{1}] = {2};{3}\n", config.kernels[i].guard_begin,
                    i, config.workspaces[i].symbol, config.kernels[i].guard_end);
        }
        for (size_t i = 0; i < config.deduce_shape.size(); ++i) {
            reg << llvm::formatv(
                    "{0}deduce_func[{1}] = {2};{3}\n", config.kernels[i].guard_begin, i,
                    config.deduce_shape[i].symbol, config.kernels[i].guard_end);
        }
        os << llvm::formatv(kernelRegistrationTemplate, reg.str());
    }

    static void writeFunc(
            std::string save_path, KernelExporter::FuncUnit func,
            bool no_tinynn_dep = false) {
        static auto getFileName = [](std::string dir, std::string file) -> std::string {
            return dir + "/" + file + ".c";
        };
        std::error_code EC;
        llvm::raw_fd_stream os(getFileName(save_path, func.symbol), EC);
        if (no_tinynn_dep) {
            os << llvm::formatv(
                    nodepFuncUnitTemplate, func.guard_begin, func.body, func.guard_end);
        } else {
            os << llvm::formatv(
                    singleFuncUnitTemplate, func.guard_begin, func.body,
                    func.guard_end);
        }
    }

    static void writeKernels(std::string save_path, KernelExporter::Config config) {
        for (auto&& i : config.kernels) {
            writeFunc(save_path, i);
        }
        for (auto&& i : config.cvkernels) {
            writeFunc(save_path, i, true);
        }
        for (auto&& i : config.deduce_shape) {
            writeFunc(save_path, i);
        }
    }

    static void writeInits(std::string save_path, KernelExporter::Config config) {
        for (auto&& i : config.inits) {
            writeFunc(save_path, i);
        }
    }

    static void writeInternalKernels(
            std::string save_path, KernelExporter::Config config) {
        for (auto&& i : config.internal_kernels) {
            writeFunc(save_path, i, true);
        }
    }

    static void genFile(std::string save_path, std::string msg) {
        std::error_code EC;
        llvm::raw_fd_stream os(save_path, EC);
        os << msg;
    }
};
}  // namespace

void KernelExporter::write(std::string save_path) {
    KernelExporterHelper::genHeader(save_path + "/kernels.h", config);
    KernelExporterHelper::genInstSwitch(
            save_path + "/runtime_inst_switch.h", used_inst);
    KernelExporterHelper::genKernelRegistration(save_path + "/kernels.c", config);
    KernelExporterHelper::writeKernels(save_path, config);
    KernelExporterHelper::writeInits(save_path, config);

    KernelExporterHelper::writeInternalKernels(save_path, config);
    KernelExporterHelper::genFile(
            save_path + "/megcc_version.txt", getMegccVersionString());
}
}  // namespace mlir

// vim: syntax=cpp.doxygen
