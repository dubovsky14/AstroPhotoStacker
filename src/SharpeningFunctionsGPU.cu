#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <exception>

#include "../headers/SharpeningFunctions.h"
#include "../headers/CudaSmartPointers.hxx"

namespace AstroPhotoStacker {
    __global__ void apply_kernel_cuda_kernel(float *d_original_image, float *d_sharpened_image, float *d_kernel, int width, int height, int kernel_size);

    inline void check_sucess_gpu(const cudaError_t error)
    {
        if(error != cudaSuccess)
        {
            fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__);
            fprintf(stderr, "code: %d, reason: %s\n", error, cudaGetErrorString(error));
            throw std::runtime_error("CUDA error occurred. Code " + std::to_string(error) + ": " + std::string(cudaGetErrorString(error)));
        }
    }
};

__global__ void AstroPhotoStacker::apply_kernel_cuda_kernel(float *d_original_image, float *d_sharpened_image, float *d_kernel, int width, int height, int kernel_size)    {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int kernel_half_size = kernel_size / 2;

    if (x >= kernel_half_size && x < width - kernel_half_size && y >= kernel_half_size && y < height - kernel_half_size) {
        double sharpened_value = 0;
        for (int y_kernel = 0; y_kernel < kernel_size; y_kernel++) {
            for (int x_kernel = 0; x_kernel < kernel_size; x_kernel++) {
                sharpened_value += d_kernel[y_kernel * kernel_size + x_kernel] * d_original_image[(y + y_kernel - kernel_half_size) * width + x + x_kernel - kernel_half_size];
            }
        }
        d_sharpened_image[y * width + x] = max(sharpened_value, 0.0);
    }
};


std::vector<float> AstroPhotoStacker::apply_kernel_cuda_float(const std::vector<float> &original_image, int width, int height, const std::vector<std::vector<float>> &kernel)  {
    const size_t n_pixels = width * height;
    std::cout << "Applying kernel on GPU..." << std::endl;
    AstroPhotoStacker::cuda_unique_ptr<float> d_original_image, d_sharpened_image, d_kernel;
    std::vector<float> kernel_flattened(kernel.size() * kernel.size());
    for (size_t i = 0; i < kernel.size(); i++) {
        for (size_t j = 0; j < kernel.size(); j++) {
            kernel_flattened[i * kernel.size() + j] = kernel[i][j];
        }
    }

    check_sucess_gpu(AstroPhotoStacker::cuda_make_unique(&d_original_image, n_pixels));
    check_sucess_gpu(AstroPhotoStacker::cuda_make_unique(&d_sharpened_image, n_pixels));
    check_sucess_gpu(AstroPhotoStacker::cuda_make_unique(&d_kernel, kernel.size() * kernel.size()));

    check_sucess_gpu(cudaMemcpy(d_sharpened_image.get(), original_image.data(), n_pixels * sizeof(float), cudaMemcpyHostToDevice));
    check_sucess_gpu(cudaMemcpy(d_original_image.get(), original_image.data(), n_pixels * sizeof(float), cudaMemcpyHostToDevice));
    check_sucess_gpu(cudaMemcpy(d_kernel.get(), kernel_flattened.data(), kernel_flattened.size() * sizeof(float), cudaMemcpyHostToDevice));

    const int block_size = 16;
    const int grid_size_x = (width + block_size - 1) / block_size;
    const int grid_size_y = (height + block_size - 1) / block_size;
    dim3 block(block_size, block_size);
    dim3 grid(grid_size_x, grid_size_y);
    apply_kernel_cuda_kernel<<<grid, block>>>(d_original_image.get(), d_sharpened_image.get(), d_kernel.get(), width, height, kernel.size());
    check_sucess_gpu(cudaDeviceSynchronize());

    std::vector<float> sharpened_image(n_pixels);
    check_sucess_gpu(cudaMemcpy(sharpened_image.data(), d_sharpened_image.get(), n_pixels * sizeof(float), cudaMemcpyDeviceToHost));

    return sharpened_image;
};
