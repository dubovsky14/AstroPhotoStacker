#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "../headers/SharpeningFunctions.h"

namespace AstroPhotoStacker {
    __global__ void apply_kernel_cuda_kernel(float *d_original_image, float *d_sharpened_image, float *d_kernel, int width, int height, int kernel_size);


    inline void CHECK(const cudaError_t error)
    {
        if(error != cudaSuccess)
        {
            fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__);
            fprintf(stderr, "code: %d, reason: %s\n", error, cudaGetErrorString(error));
            exit(1);
        }
    }

}

__global__ void AstroPhotoStacker::apply_kernel_cuda_kernel(float *d_original_image, float *d_sharpened_image, float *d_kernel, int width, int height, int kernel_size)    {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int kernel_half_size = kernel_size / 2;

    if (x >= kernel_half_size && x < width - kernel_half_size && y >= kernel_half_size && y < height - kernel_half_size) {
        float sharpened_value = 0;
        for (int y_kernel = 0; y_kernel < kernel_size; y_kernel++) {
            for (int x_kernel = 0; x_kernel < kernel_size; x_kernel++) {
                sharpened_value += d_kernel[y_kernel * kernel_size + x_kernel] * d_original_image[(y + y_kernel - kernel_half_size) * width + x + x_kernel - kernel_half_size];
            }
        }
        d_sharpened_image[y * width + x] = max(sharpened_value, 0.0f);
    }
};


std::vector<float> AstroPhotoStacker::apply_kernel_cuda_float(const std::vector<float> &original_image, int width, int height, const std::vector<std::vector<float>> &kernel)  {
    const size_t n_pixels = width * height;
    std::cout << "Applying kernel on GPU..." << std::endl;
    float *d_original_image, *d_sharpened_image, *d_kernel;
    std::vector<float> kernel_flattened(kernel.size() * kernel.size());
    for (size_t i = 0; i < kernel.size(); i++) {
        for (size_t j = 0; j < kernel.size(); j++) {
            kernel_flattened[i * kernel.size() + j] = kernel[i][j];
        }
    }

    double sum_original = std::accumulate(original_image.begin(), original_image.end(), 0.0);


    CHECK(cudaMalloc(&d_original_image, n_pixels * sizeof(float)));
    CHECK(cudaMalloc(&d_sharpened_image, n_pixels * sizeof(float)));
    CHECK(cudaMalloc(&d_kernel, kernel.size() * kernel.size() * sizeof(float)));

    CHECK(cudaMemcpy(d_sharpened_image, original_image.data(), n_pixels * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_original_image, original_image.data(), n_pixels * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_kernel, kernel_flattened.data(), kernel_flattened.size() * sizeof(float), cudaMemcpyHostToDevice));

    const int block_size = 16;
    const int grid_size_x = (width + block_size - 1) / block_size;
    const int grid_size_y = (height + block_size - 1) / block_size;
    dim3 block(block_size, block_size);
    dim3 grid(grid_size_x, grid_size_y);
    std::cout << "Launching kernel with grid size (" << grid_size_x << ", " << grid_size_y << ") and block size (" << block_size << ", " << block_size << ")" << std::endl;
    apply_kernel_cuda_kernel<<<grid, block>>>(d_original_image, d_sharpened_image, d_kernel, width, height, kernel.size());
    std::cout << "Kernel launched." << std::endl;
    CHECK(cudaDeviceSynchronize());

    std::vector<float> sharpened_image(n_pixels);
    CHECK(cudaMemcpy(sharpened_image.data(), d_sharpened_image, n_pixels * sizeof(float), cudaMemcpyDeviceToHost));
    CHECK(cudaFree(d_original_image));
    CHECK(cudaFree(d_sharpened_image));
    cudaFree(d_kernel);

    double sum_sharpened = std::accumulate(sharpened_image.begin(), sharpened_image.end(), 0.0);
    std::cout << "Sum of original image: " << sum_original << std::endl;
    std::cout << "Sum of sharpened image: " << sum_sharpened << std::endl;

    return sharpened_image;
};
