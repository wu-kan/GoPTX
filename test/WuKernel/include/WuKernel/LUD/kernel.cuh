#pragma once
namespace wuk {

namespace LUD {

// https://github.com/accel-sim/gpu-app-collection/blob/v1.1/src/cuda/rodinia/3.1/cuda/lud/cuda/lud_kernel.cu#L167
template <typename ElTp = float, int BLOCK_SIZE = 16, int LAUNCH1D = 1>
__global__ void lud_internal(ElTp *m, int matrix_dim, int offset) {
  int blockIdx_y;
  int blockIdx_x;
  int threadIdx_y;
  int threadIdx_x;
  if (LAUNCH1D) {
    int blockDim_x = BLOCK_SIZE;
    threadIdx_y = threadIdx.x / blockDim_x;
    threadIdx_x = threadIdx.x % blockDim_x;
    int gridDim_x = (matrix_dim - offset) / blockDim_x;
    blockIdx_y = blockIdx.x / gridDim_x;
    blockIdx_x = blockIdx.x % gridDim_x;
  } else {
    threadIdx_x = threadIdx.x;
    threadIdx_y = threadIdx.y;
    blockIdx_x = blockIdx.x;
    blockIdx_y = blockIdx.y;
  }

  if (blockIdx_y == 0 || blockIdx_x == 0)
    return;
  --blockIdx_y;
  --blockIdx_x;

  __shared__ ElTp peri_row[BLOCK_SIZE][BLOCK_SIZE];
  __shared__ ElTp peri_col[BLOCK_SIZE][BLOCK_SIZE];

  int i;
  ElTp sum;

  int global_row_id = offset + (blockIdx_y + 1) * BLOCK_SIZE;
  int global_col_id = offset + (blockIdx_x + 1) * BLOCK_SIZE;

  peri_row[threadIdx_y][threadIdx_x] =
      m[(offset + threadIdx_y) * matrix_dim + global_col_id + threadIdx_x];
  peri_col[threadIdx_y][threadIdx_x] =
      m[(global_row_id + threadIdx_y) * matrix_dim + offset + threadIdx_x];

  __syncthreads();

  sum = 0;
  for (i = 0; i < BLOCK_SIZE; i++)
    sum += peri_col[threadIdx_y][i] * peri_row[i][threadIdx_x];
  m[(global_row_id + threadIdx_y) * matrix_dim + global_col_id + threadIdx_x] -=
      sum;
}

#define WUK_DEFINE_KERNEL_LUD                                                  \
  template __global__ void lud_internal<>(float *m, int matrix_dim, int offset)

extern WUK_DEFINE_KERNEL_LUD;

}; // namespace LUD

}; // namespace wuk