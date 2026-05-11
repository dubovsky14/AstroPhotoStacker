#include "cuda_runtime.h"
#include "device_launch_parameters.h"

namespace AstroPhotoStacker {

    /**
     * @brief RAII wrapper for CUDA memory management. Automatically frees the memory when going out of scope. Similar to std::unique_ptr
     */
    template <typename T>
    class cuda_unique_ptr {
        public:
            cuda_unique_ptr() = default;

            explicit cuda_unique_ptr(T* ptr) : m_ptr(ptr) {};

            ~cuda_unique_ptr() {
                if (m_ptr) {
                    cudaFree(m_ptr);
                }
            };

            cuda_unique_ptr(const cuda_unique_ptr&) = delete;
            cuda_unique_ptr& operator=(const cuda_unique_ptr&) = delete;

            cuda_unique_ptr(cuda_unique_ptr&& other) noexcept : m_ptr(other.m_ptr) {
                other.m_ptr = nullptr;
            }

            cuda_unique_ptr& operator=(cuda_unique_ptr&& other) noexcept {
                if (this != &other) {
                    if (m_ptr) {
                        cudaFree(m_ptr);
                    }
                    m_ptr = other.m_ptr;
                    other.m_ptr = nullptr;
                }
                return *this;
            };

            T* get() const { return m_ptr; }

            T* release() {
                T* temp = m_ptr;
                m_ptr = nullptr;
                return temp;
            };

            T& operator*() const {
                return *m_ptr;
            }

        private:
            T* m_ptr = nullptr;
    };

    template <typename T>
    cuda_unique_ptr<T> cuda_make_unique(size_t n_elements) {
        T* ptr;
        cudaMalloc(&ptr, n_elements * sizeof(T));
        return cuda_unique_ptr<T>(ptr);
    };

    template <typename T>
    cudaError_t cuda_make_unique(cuda_unique_ptr<T> *ptr, size_t n_elements) {
        T* raw_ptr;
        const cudaError_t status = cudaMalloc(&raw_ptr, n_elements * sizeof(T));
        *ptr = std::move(cuda_unique_ptr<T>(raw_ptr));
        return status;
    };
}