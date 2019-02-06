// MIT License
//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <type_traits>
#include <cstdlib>
#include <algorithm>

// Google Test
#include <gtest/gtest.h>

// Thrust
#include <thrust/scatter.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/iterator/retag.h>
#include <thrust/sequence.h>
#include <thrust/device_vector.h>
#include <thrust/fill.h>

// HIP API
#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC
#include <hip/hip_runtime_api.h>

#define HIP_CHECK(condition) ASSERT_EQ(condition, hipSuccess)
#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

#include "test_utils.hpp"

template<
    class InputType
>
struct Params
{
    using input_type = InputType;
};

template<class Params>
class FillTests : public ::testing::Test
{
public:
    using input_type = typename Params::input_type;
};

typedef ::testing::Types<
    Params<thrust::host_vector<short>>,
    Params<thrust::host_vector<int>>,
    Params<thrust::host_vector<long long>>,
    Params<thrust::host_vector<unsigned short>>,
    Params<thrust::host_vector<unsigned int>>,
    Params<thrust::host_vector<unsigned long long>>,
    Params<thrust::host_vector<float>>,
    Params<thrust::host_vector<double>>,
    Params<thrust::device_vector<short>>,
    Params<thrust::device_vector<int>>,
    Params<thrust::device_vector<long long>>,
    Params<thrust::device_vector<unsigned short>>,
    Params<thrust::device_vector<unsigned int>>,
    Params<thrust::device_vector<unsigned long long>>,
    Params<thrust::device_vector<float>>,
    Params<thrust::device_vector<double>>
> FillTestsParams;

TYPED_TEST_CASE(FillTests, FillTestsParams);

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC

TYPED_TEST(FillTests, TestFillSimple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector v(5);
  v[0] = T(0); v[1] = T(1); v[2] = T(2); v[3] = T(3); v[4] = T(4);

  thrust::fill(v.begin() + 1, v.begin() + 4, T(7));

  ASSERT_EQ(v[0], T(0));
  ASSERT_EQ(v[1], T(7));
  ASSERT_EQ(v[2], T(7));
  ASSERT_EQ(v[3], T(7));
  ASSERT_EQ(v[4], T(4));

  thrust::fill(v.begin() + 0, v.begin() + 3, T(8));

  ASSERT_EQ(v[0], T(8));
  ASSERT_EQ(v[1], T(8));
  ASSERT_EQ(v[2], T(8));
  ASSERT_EQ(v[3], T(7));
  ASSERT_EQ(v[4], T(4));

  thrust::fill(v.begin() + 2, v.end(), T(9));

  ASSERT_EQ(v[0], T(8));
  ASSERT_EQ(v[1], T(8));
  ASSERT_EQ(v[2], T(9));
  ASSERT_EQ(v[3], T(9));
  ASSERT_EQ(v[4], T(9));

  thrust::fill(v.begin(), v.end(), T(1));

  ASSERT_EQ(v[0], T(1));
  ASSERT_EQ(v[1], T(1));
  ASSERT_EQ(v[2], T(1));
  ASSERT_EQ(v[3], T(1));
  ASSERT_EQ(v[4], T(1));

}

TEST(FillTests, TestFillDiscardIterator)
{
  // there's no result to check because fill returns void
  thrust::fill(thrust::discard_iterator<thrust::host_system_tag>(),
               thrust::discard_iterator<thrust::host_system_tag>(10),
               13);

  thrust::fill(thrust::discard_iterator<thrust::device_system_tag>(),
               thrust::discard_iterator<thrust::device_system_tag>(10),
               13);
}

TYPED_TEST(FillTests, TestFillMixedTypes)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector v(4);

  thrust::fill(v.begin(), v.end(), (long) 10);

  ASSERT_EQ(v[0], T(10));
  ASSERT_EQ(v[1], T(10));
  ASSERT_EQ(v[2], T(10));
  ASSERT_EQ(v[3], T(10));

  thrust::fill(v.begin(), v.end(), (float) 20);

  ASSERT_EQ(v[0], T(20));
  ASSERT_EQ(v[1], T(20));
  ASSERT_EQ(v[2], T(20));
  ASSERT_EQ(v[3], T(20));
}

TYPED_TEST(FillTests, TestFill)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  const std::vector<size_t> sizes = get_sizes();
  for(auto size : sizes)
  {
    thrust::host_vector<T>   h_data  = get_random_data<T>( size,
                                                           std::numeric_limits<T>::min(),
                                                           std::numeric_limits<T>::max());

    thrust::device_vector<T> d_data = h_data;

    thrust::fill(h_data.begin() + std::min((size_t)1, size), h_data.begin() + std::min((size_t)3, size), T(0));
    thrust::fill(d_data.begin() + std::min((size_t)1, size), d_data.begin() + std::min((size_t)3, size), T(0));

    ASSERT_EQ(h_data, d_data);

    thrust::fill(h_data.begin() + std::min((size_t)117, size), h_data.begin() + std::min((size_t)367, size), T(1));
    thrust::fill(d_data.begin() + std::min((size_t)117, size), d_data.begin() + std::min((size_t)367, size), T(1));

    ASSERT_EQ(h_data, d_data);

    thrust::fill(h_data.begin() + std::min((size_t)8, size), h_data.begin() + std::min((size_t)259, size), T(2));
    thrust::fill(d_data.begin() + std::min((size_t)8, size), d_data.begin() + std::min((size_t)259, size), T(2));

    ASSERT_EQ(h_data, d_data);

    thrust::fill(h_data.begin() + std::min((size_t)3, size), h_data.end(), T(3));
    thrust::fill(d_data.begin() + std::min((size_t)3, size), d_data.end(), T(3));

    ASSERT_EQ(h_data, d_data);

    thrust::fill(h_data.begin(), h_data.end(), T(4));
    thrust::fill(d_data.begin(), d_data.end(), T(4));

    ASSERT_EQ(h_data, d_data);
  }
}

TYPED_TEST(FillTests, TestFillNSimple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;
  using Iterator = typename Vector::iterator;

  Vector v(5);
  v[0] = T(0); v[1] = T(1); v[2] = T(2); v[3] = T(3); v[4] = T(4);

  Iterator iter = thrust::fill_n(v.begin() + 1, 3, T(7));

  ASSERT_EQ(v[0], T(0));
  ASSERT_EQ(v[1], T(7));
  ASSERT_EQ(v[2], T(7));
  ASSERT_EQ(v[3], T(7));
  ASSERT_EQ(v[4], T(4));
  ASSERT_EQ(v.begin() + 4, iter);

  iter = thrust::fill_n(v.begin() + 0, 3, T(8));

  ASSERT_EQ(v[0], T(8));
  ASSERT_EQ(v[1], T(8));
  ASSERT_EQ(v[2], T(8));
  ASSERT_EQ(v[3], T(7));
  ASSERT_EQ(v[4], T(4));
  ASSERT_EQ(v.begin() + 3, iter);

  iter = thrust::fill_n(v.begin() + 2, 3, T(9));

  ASSERT_EQ(v[0], T(8));
  ASSERT_EQ(v[1], T(8));
  ASSERT_EQ(v[2], T(9));
  ASSERT_EQ(v[3], T(9));
  ASSERT_EQ(v[4], T(9));
  ASSERT_EQ(v.end(), iter);

  iter = thrust::fill_n(v.begin(), v.size(), T(1));

  ASSERT_EQ(v[0], T(1));
  ASSERT_EQ(v[1], T(1));
  ASSERT_EQ(v[2], T(1));
  ASSERT_EQ(v[3], T(1));
  ASSERT_EQ(v[4], T(1));
  ASSERT_EQ(v.end(), iter);

}

TEST(FillTests, TestFillNDiscardIterator)
{
  thrust::discard_iterator<thrust::host_system_tag> h_result =
    thrust::fill_n(thrust::discard_iterator<thrust::host_system_tag>(),
                   10,
                   13);

  thrust::discard_iterator<thrust::device_system_tag> d_result =
    thrust::fill_n(thrust::discard_iterator<thrust::device_system_tag>(),
                   10,
                   13);

  thrust::discard_iterator<> reference(10);

  ASSERT_EQ(reference, h_result);
  ASSERT_EQ(reference, d_result);
}

TYPED_TEST(FillTests, TestFillNMixedTypes)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;
  using Iterator = typename Vector::iterator;

  Vector v(4);

  Iterator iter = thrust::fill_n(v.begin(), v.size(), (long) 10);

  ASSERT_EQ(v[0], T(10));
  ASSERT_EQ(v[1], T(10));
  ASSERT_EQ(v[2], T(10));
  ASSERT_EQ(v[3], T(10));
  ASSERT_EQ(v.end(), iter);

  iter = thrust::fill_n(v.begin(), v.size(), (float) 20);

  ASSERT_EQ(v[0], T(20));
  ASSERT_EQ(v[1], T(20));
  ASSERT_EQ(v[2], T(20));
  ASSERT_EQ(v[3], T(20));
  ASSERT_EQ(v.end(), iter);
}


TYPED_TEST(FillTests, TestFillN)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  const std::vector<size_t> sizes = get_sizes();
  for(auto size : sizes)
  {
    thrust::host_vector<T>   h_data  = get_random_data<T>( size,
                                                           std::numeric_limits<T>::min(),
                                                           std::numeric_limits<T>::max());

    thrust::device_vector<T> d_data = h_data;

    size_t begin_offset = std::min<size_t>(1,size);
    thrust::fill_n(h_data.begin() + begin_offset, std::min((size_t)3, size) - begin_offset, T(0));
    thrust::fill_n(d_data.begin() + begin_offset, std::min((size_t)3, size) - begin_offset, T(0));

    ASSERT_EQ(h_data, d_data);

    begin_offset = std::min<size_t>(117, size);
    thrust::fill_n(h_data.begin() + begin_offset, std::min((size_t)367, size) - begin_offset, T(1));
    thrust::fill_n(d_data.begin() + begin_offset, std::min((size_t)367, size) - begin_offset, T(1));

    ASSERT_EQ(h_data, d_data);

    begin_offset = std::min<size_t>(8, size);
    thrust::fill_n(h_data.begin() + begin_offset, std::min((size_t)259, size) - begin_offset, T(2));
    thrust::fill_n(d_data.begin() + begin_offset, std::min((size_t)259, size) - begin_offset, T(2));

    ASSERT_EQ(h_data, d_data);

    begin_offset = std::min<size_t>(3, size);
    thrust::fill_n(h_data.begin() + begin_offset, h_data.size() - begin_offset, T(3));
    thrust::fill_n(d_data.begin() + begin_offset, d_data.size() - begin_offset, T(3));

    ASSERT_EQ(h_data, d_data);

    thrust::fill_n(h_data.begin(), h_data.size(), T(4));
    thrust::fill_n(d_data.begin(), d_data.size(), T(4));

    ASSERT_EQ(h_data, d_data);
  }
}

TYPED_TEST(FillTests, TestFillZipIterator)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;

  Vector v1(3,T(0));
  Vector v2(3,T(0));
  Vector v3(3,T(0));

  thrust::fill(thrust::make_zip_iterator(thrust::make_tuple(v1.begin(),v2.begin(),v3.begin())),
               thrust::make_zip_iterator(thrust::make_tuple(v1.end(),v2.end(),v3.end())),
               thrust::tuple<T,T,T>(4,7,13));

  ASSERT_EQ(T(4),  v1[0]);
  ASSERT_EQ(T(4),  v1[1]);
  ASSERT_EQ(T(4),  v1[2]);
  ASSERT_EQ(T(7),  v2[0]);
  ASSERT_EQ(T(7),  v2[1]);
  ASSERT_EQ(T(7),  v2[2]);
  ASSERT_EQ(T(13), v3[0]);
  ASSERT_EQ(T(13), v3[1]);
  ASSERT_EQ(T(13), v3[2]);
}

// TODO: Implement tuple.
/*TYPED_TEST(FillTests, TestFillTuple)
{
  using Vector = typename TestFixture::input_type;
  using T = typename Vector::value_type;
  using Tuple = typename thrust::tuple<T,T>;

  thrust::host_vector<Tuple>   h(3, Tuple(0,0));
  thrust::device_vector<Tuple> d(3, Tuple(0,0));

  thrust::fill(h.begin(), h.end(), Tuple(4,7));
  thrust::fill(d.begin(), d.end(), Tuple(4,7));

  ASSERT_EQ(h, d);
};*/

struct TypeWithTrivialAssigment
{
  int x, y, z;
};

TEST(FillTests, TestFillWithTrivialAssignment)
{
  using T = TypeWithTrivialAssigment;

  thrust::host_vector<T>   h(1);
  thrust::device_vector<T> d(1);

  ASSERT_EQ(h[0].x, 0);
  ASSERT_EQ(h[0].y, 0);
  ASSERT_EQ(h[0].z, 0);
  ASSERT_EQ(static_cast<T>(d[0]).x, 0);
  ASSERT_EQ(static_cast<T>(d[0]).y, 0);
  ASSERT_EQ(static_cast<T>(d[0]).z, 0);

  T val;
  val.x = 10;
  val.y = 20;
  val.z = -1;

  thrust::fill(h.begin(), h.end(), val);
  thrust::fill(d.begin(), d.end(), val);

  ASSERT_EQ(h[0].x, 10);
  ASSERT_EQ(h[0].y, 20);
  ASSERT_EQ(h[0].z, -1);
  ASSERT_EQ(static_cast<T>(d[0]).x, 10);
  ASSERT_EQ(static_cast<T>(d[0]).y, 20);
  ASSERT_EQ(static_cast<T>(d[0]).z, -1);
}

struct TypeWithNonTrivialAssigment
{
  int x, y, z;

  __host__ __device__
  TypeWithNonTrivialAssigment() : x(0), y(0), z(0) {}

  __host__ __device__
  TypeWithNonTrivialAssigment& operator=(const TypeWithNonTrivialAssigment& t)
  {
    x = t.x;
    y = t.y;
    z = t.x + t.y;
    return *this;
  }

  __host__ __device__
  bool operator==(const TypeWithNonTrivialAssigment& t) const
  {
    return x == t.x && y == t.y && z == t.z;
  }
};

TEST(FillTests, TestFillWithNonTrivialAssignment)
{
  using T = TypeWithNonTrivialAssigment;

  thrust::host_vector<T>   h(1);
  thrust::device_vector<T> d(1);

  ASSERT_EQ(h[0].x, 0);
  ASSERT_EQ(h[0].y, 0);
  ASSERT_EQ(h[0].z, 0);
  ASSERT_EQ(static_cast<T>(d[0]).x, 0);
  ASSERT_EQ(static_cast<T>(d[0]).y, 0);
  ASSERT_EQ(static_cast<T>(d[0]).z, 0);

  T val;
  val.x = 10;
  val.y = 20;
  val.z = -1;

  thrust::fill(h.begin(), h.end(), val);
  thrust::fill(d.begin(), d.end(), val);

  ASSERT_EQ(h[0].x, 10);
  ASSERT_EQ(h[0].y, 20);
  ASSERT_EQ(h[0].z, 30);
  ASSERT_EQ(static_cast<T>(d[0]).x, 10);
  ASSERT_EQ(static_cast<T>(d[0]).y, 20);
  ASSERT_EQ(static_cast<T>(d[0]).z, 30);
}

template<typename ForwardIterator, typename T>
void fill(my_system &system, ForwardIterator, ForwardIterator, const T&)
{
    system.validate_dispatch();
}

TEST(FillTests, TestFillDispatchExplicit)
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::fill(sys, vec.begin(), vec.end(), 0);

  ASSERT_EQ(true, sys.is_valid());
}

template<typename ForwardIterator, typename T>
void fill(my_tag, ForwardIterator first, ForwardIterator, const T&)
{
    *first = 13;
}

TEST(FillTests, TestFillDispatchImplicit)
{
  thrust::device_vector<int> vec(1);

  thrust::fill(thrust::retag<my_tag>(vec.begin()),
               thrust::retag<my_tag>(vec.end()),
               0);

  ASSERT_EQ(13, vec.front());
}

template<typename OutputIterator, typename Size, typename T>
OutputIterator fill_n(my_system &system, OutputIterator first, Size, const T&)
{
    system.validate_dispatch();
    return first;
}

TEST(FillTests, TestFillNDispatchExplicit)
{
  thrust::device_vector<int> vec(1);

  my_system sys(0);
  thrust::fill_n(sys, vec.begin(), vec.size(), 0);

  ASSERT_EQ(true, sys.is_valid());
}

template<typename OutputIterator, typename Size, typename T>
OutputIterator fill_n(my_tag, OutputIterator first, Size, const T&)
{
    *first = 13;
    return first;
}

TEST(FillTests, TestFillNDispatchImplicit)
{
  thrust::device_vector<int> vec(1);

  thrust::fill_n(thrust::retag<my_tag>(vec.begin()),
                 vec.size(),
                 0);

  ASSERT_EQ(13, vec.front());
}

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_HCC
