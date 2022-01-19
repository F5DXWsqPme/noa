/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2021, Roland Grinis, GrinisRIT ltd. (roland.grinis@grinisrit.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <torch/types.h>

namespace noa::utils::cuda {

    constexpr uint32_t MIN_BLOCK = 32;
    constexpr uint32_t MAX_BLOCK = 1024;

    inline std::tuple<uint32_t, uint32_t> get_grid(const uint32_t num_tasks) {
        const uint32_t thread_blocks = (num_tasks + MIN_BLOCK - 1) / MIN_BLOCK;
        const uint32_t num_threads = std::min(MIN_BLOCK * thread_blocks, MAX_BLOCK);
        return std::make_tuple((num_tasks + num_threads - 1) / num_threads, num_threads);
    }

    template<typename Kernel>
    __global__ void launch_kernel(const Kernel kernel, const uint32_t num_tasks) {
        uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i < num_tasks) kernel(i);
    }

    template<typename Dtype, typename Lambda>
    inline void for_eachi(const Lambda &lambda, const torch::Tensor &result) {
        Dtype *pres = result.data_ptr<Dtype>();

        const auto n = result.numel();
        const auto [blocks, threads] = get_grid(n);

        const auto lambda_kernel = [pres, lambda] __device__(const int i){
            lambda(i, pres[i]);
        };
        launch_kernel<<<blocks, threads>>>(lambda_kernel, n);
    }

    template<typename Dtype, typename Lambda>
    inline void for_each(const Lambda &lambda, const torch::Tensor &result) {
        for_eachi<Dtype>([lambda] __device__(const int, Dtype &k){lambda(k);}, result);
    }

    template<typename Dtype, typename Lambda>
    inline void vmapi(const torch::Tensor &values, const Lambda &lambda, const torch::Tensor &result) {
        const Dtype *pvals = values.data_ptr<Dtype>();
        for_eachi<Dtype>([lambda, pvals] __device__(const int i, Dtype &k){k = lambda(i, pvals[i]);}, result);
    }

    template<typename Dtype, typename Lambda>
    inline void vmap(const torch::Tensor &values, const Lambda &lambda, const torch::Tensor &result) {
        vmapi<Dtype>(values,
                     [lambda] __device__(const int, const Dtype &k){ return lambda(k);},
                     result);
    }

    template<typename Dtype, typename Lambda>
    inline torch::Tensor vmapi(const torch::Tensor &values, const Lambda &lambda) {
        const auto result = torch::zeros_like(values);
        vmapi<Dtype>(values, lambda, result);
        return result;
    }

    template<typename Dtype, typename Lambda>
    inline torch::Tensor vmap(const torch::Tensor &values, const Lambda &lambda) {
        const auto result = torch::zeros_like(values);
        vmap<Dtype>(values, lambda, result);
        return result;
    }

}