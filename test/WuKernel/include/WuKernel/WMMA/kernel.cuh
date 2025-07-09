#pragma once
#include <cuda_fp16.h>
#include <mma.h>

namespace wuk {

namespace WMMA {

// https://github.com/accel-sim/gpu-app-collection/blob/v1.1/src/cuda/NVIDIA_CUDA-11.0_Samples/cudaTensorCoreGemm/cudaTensorCoreGemm.cu#L359
// Performs an MxNxK GEMM (C=alpha*A*B + beta*C) assuming:
//  1) Matrices are packed in memory.
//  2) M, N and K are multiples of 16.
//  3) Neither A nor B are transposed.
// Note: This is a less performant version of the compute_gemm kernel. It is
// designed for
//       demonstration purposes only to show the CUDA WMMA API use without
//       relying on availability of the shared memory.
template <int WMMA_M = 16, int WMMA_N = 16, int WMMA_K = 16, int WARP_SIZE = 32,
          int LAUNCH1D = 1>
__global__ void simple_wmma_gemm(__half *const a, __half *const b,
                                 float *const c, float *d, int m_ld, int n_ld,
                                 int k_ld, float alpha, float beta) {
  int blockDim_x;
  int blockDim_y;
  int blockIdx_x;
  int blockIdx_y;
  int threadIdx_x;
  int threadIdx_y;
  if (LAUNCH1D) {
    blockDim_x = blockDim.x;
    blockDim_y = blockDim.x / blockDim_x;
    threadIdx_x = threadIdx.x % blockDim_x;
    threadIdx_y = threadIdx.x / blockDim_x;
    auto gridDim_x = (m_ld + (WMMA_M * blockDim_x / WARP_SIZE - 1)) /
                     (WMMA_M * blockDim_x / WARP_SIZE);
    blockIdx_x = blockIdx.x % gridDim_x;
    blockIdx_y = blockIdx.x / gridDim_x;
  } else {
    blockDim_x = blockDim.x;
    blockDim_y = blockDim.y;
    threadIdx_x = threadIdx.x;
    threadIdx_y = threadIdx.y;
    blockIdx_x = blockIdx.x;
    blockIdx_y = blockIdx.y;
  }
  // Leading dimensions. Packed with no transpositions.
  int lda = k_ld;
  int ldb = k_ld;
  int ldc = n_ld;

  // Tile using a 2D grid
  int warpM = (blockIdx_x * blockDim_x + threadIdx_x) / WARP_SIZE;
  int warpN = (blockIdx_y * blockDim_y + threadIdx_y);

  // Load in the current value of c, scale it by beta, and add this our result
  // scaled by alpha
  int cCol = warpN * WMMA_N;
  int cRow = warpM * WMMA_M;

  if (cRow >= m_ld || cCol >= n_ld)
    return;

  // Declare the fragments
  nvcuda::wmma::fragment<nvcuda::wmma::accumulator, WMMA_M, WMMA_N, WMMA_K,
                         float>
      acc_frag;

  nvcuda::wmma::fill_fragment(acc_frag, 0.0f);

  // Loop over k
  for (int i = 0, aCol, aRow, bCol, bRow;
       aCol = i, aRow = warpM * WMMA_M, bCol = warpN * WMMA_N, bRow = i,
           i < k_ld && aRow < m_ld && aCol < k_ld && bRow < k_ld && bCol < n_ld;
       i += WMMA_K) {
    nvcuda::wmma::fragment<nvcuda::wmma::matrix_a, WMMA_M, WMMA_N, WMMA_K,
                           __half, nvcuda::wmma::row_major>
        a_frag;
    nvcuda::wmma::fragment<nvcuda::wmma::matrix_b, WMMA_M, WMMA_N, WMMA_K,
                           __half, nvcuda::wmma::col_major>
        b_frag;
    // Load the inputs
    nvcuda::wmma::load_matrix_sync(a_frag, a + aCol + aRow * lda, lda);
    nvcuda::wmma::load_matrix_sync(b_frag, b + bRow + bCol * ldb, ldb);

    // Perform the matrix multiplication
    nvcuda::wmma::mma_sync(acc_frag, a_frag, b_frag, acc_frag);
  }

  for (int i = 0; i < acc_frag.num_elements; i++) {
    acc_frag.x[i] *= alpha;
  }

  if (beta) {
    nvcuda::wmma::fragment<nvcuda::wmma::accumulator, WMMA_M, WMMA_N, WMMA_K,
                           float>
        c_frag;

    // Load in the current value of c, scale it by beta, and add this our result
    // scaled by alpha
    nvcuda::wmma::load_matrix_sync(c_frag, c + cCol + cRow * ldc, ldc,
                                   nvcuda::wmma::mem_row_major);

    for (int i = 0; i < c_frag.num_elements; i++) {
      acc_frag.x[i] += beta * c_frag.x[i];
    }
  }

  // Store the output
  nvcuda::wmma::store_matrix_sync(d + cCol + cRow * ldc, acc_frag, ldc,
                                  nvcuda::wmma::mem_row_major);
}

#define WUK_DEFINE_KERNEL_WMMA                                                 \
  template __global__ void simple_wmma_gemm(                                   \
      __half *const a, __half *const b, float *const c, float *d, int m_ld,    \
      int n_ld, int k_ld, float alpha, float beta)

extern WUK_DEFINE_KERNEL_WMMA;

} // namespace WMMA

} // namespace wuk