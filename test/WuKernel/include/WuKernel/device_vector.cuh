#pragma once

#include <thrust/device_ptr.h>

#ifndef WUK_DEVICE_VECTOR_USE_THRUST

#include <thrust/device_free.h>
#include <thrust/device_malloc.h>
#include <type_traits>

namespace wuk {

template <typename T> class device_vector {
  static_assert(std::is_same<T, bool>::value == 0,
                "consider the fxxk std::vector<bool>, please use "
                "wuk::device_vector<int8_t> instead.");

  thrust::device_ptr<T> _data;
  size_t _size;

public:
  device_vector(size_t size)
      : _size(size), _data(thrust::device_malloc<T>(size)) {}
  ~device_vector() { thrust::device_free(_data); }
  auto data() { return _data; }
  thrust::device_ptr<T const> data() const { return _data; }
  auto size() const { return _size; }
};

} // namespace wuk

#else

#include <thrust/device_vector.h>

namespace wuk {
using thrust::device_vector;
} // namespace wuk

#endif

#include <thrust/functional.h>
#include <thrust/inner_product.h>
#include <thrust/random.h>
#include <thrust/tabulate.h>

namespace wuk {

template <typename T>
bool equal(const device_vector<T> &lhs, const device_vector<T> &rhs, T eps);

template <typename T>
void generate(device_vector<T> &v, T a, T b,
              int s = thrust::default_random_engine::default_seed);

template <typename T> class AbsDiff : thrust::binary_function<T, T, char> {
  const T eps;
  AbsDiff(T eps) : eps(eps) {}

public:
  char __host__ __device__ __forceinline__ operator()(const T &a, const T &b) {
    auto tmp = a < b ? b - a : a - b;
    return tmp >= eps;
  }
  friend bool equal<T>(const device_vector<T> &lhs, const device_vector<T> &rhs,
                       T eps);
};

template <typename T>
bool equal(const device_vector<T> &lhs, const device_vector<T> &rhs, T eps) {
  if (lhs.size() != rhs.size())
    return false;
  auto pa = thrust::device_pointer_cast(lhs.data()),
       pb = thrust::device_pointer_cast(rhs.data());
  return !thrust::inner_product(pa, pa + lhs.size(), pb, (char)0,
                                thrust::maximum<char>(), AbsDiff<T>(eps));
}

template <typename T> class QuickGenerator : thrust::unary_function<T, T> {
  const T _a, _b;
  const thrust::default_random_engine::result_type _seed;
  QuickGenerator(T a, T b, thrust::default_random_engine::result_type s)
      : _a(a), _b(b), _seed(s) {}

public:
  T __host__ __device__ __forceinline__ operator()(size_t idx) const {
    thrust::default_random_engine rng(_seed);
    rng.discard(idx);
    thrust::uniform_real_distribution<T> _dist(_a, _b);
    return _dist(rng);
  }
  friend void generate<T>(device_vector<T> &v, T a, T b, int s);
};

template <typename T> void generate(device_vector<T> &v, T a, T b, int s) {
  auto p = thrust::device_pointer_cast(v.data());
  thrust::tabulate(p, p + v.size(), QuickGenerator<T>(a, b, s));
}

using thrust::raw_pointer_cast;

} // namespace wuk