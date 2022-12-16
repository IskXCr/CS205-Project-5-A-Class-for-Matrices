/**
 * @file mat.cpp
 * @author IskXCr (IskXCr@outlook.com)
 * @brief This file contains the basic implementation of a Matrix class,
 *        with its major reference from @ref "The C++ Programming Language, Fourth Edition" by Bjarne Stroustrup.
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022 IskXCr
 *
 */
#include "mat.hpp"

#include <cassert>

using namespace utils;

// ------------------------------
// Other constructors, from submatrix or reference to Matrix.

/**
 * @brief Construct a new Matrix<T, N>:: Matrix object from the matrix x if the type U is convertible to T.
 *
 * @tparam T
 * @tparam N
 * @tparam U
 * @param x
 */
template <typename T, size_t N>
template <typename U>
Matrix<T, N>::Matrix(const Matrix_ref<U, N> &x) : desc{x.desc}, elems{x.begin(), x.end()}
{
    // Ensure the type U is convertible to T.
    static_assert(std::is_convertible<U, T>, "Matrix constructor: incompatible element types");
}

/**
 * @brief Copying the matrix x if the type U is convertible to T.
 *
 * @tparam T type of elements
 * @tparam N number of dimensions
 * @tparam Exts number of extents
 * @param x the other matrix to be copied
 * @return Matrix<T, N>& this matrix, hard copied from x
 */
template <typename T, size_t N>
template <typename U>
Matrix<T, N> &Matrix<T, N>::operator=(const Matrix_ref<U, N> &x)
{
    static_assert(std::is_convertible<U, T>, "Matrix operator=: incompatible element types");

    desc = x.desc;
    elems.assign(x.begin(), x.end());
    return *this;
}

/**
 * @brief Construct a new Matrix<T, N>:: Matrix object from the extents specified. For example, Matrix<double, 2> mat1(2, 2).
 *
 * @tparam T type of elements
 * @tparam N number of dimensions
 * @tparam Exts number of extents
 * @param exts extents
 */
template <typename T, size_t N>
template <typename... Exts>
Matrix<T, N>::Matrix(Exts... exts) : desc{exts...}, elems(desc.size) {}

/**
 * @brief Construct a new Matrix< T,  N>:: Matrix object from {} initializer. For example, Matrix<int, 2> mat1 = {{1, 2}, {3, 4}}.
 *
 * @tparam T type of elements
 * @tparam N number of dimensions
 * @param init Matrix_initializer that is in itself a matrix
 */
template <typename T, size_t N>
Matrix<T, N>::Matrix(Matrix_initializer<T, N> init)
{
    Matrix_impl::derive_extents(init, desc.extents); // deduce extents from initializer list
    elems.reserve(desc.size);                        // make room for slices
    Matrix_impl::insert_flat(init, elems);           // Initialize from initializer list
    assert(elems.size() == desc.size);
}

// ------------------------------
// Element Accessors

// ------------------------------
// Arithmetic operations on the matrix

/**
 * @brief Accepts a function to apply to each element of this Matrix.
 *
 * @tparam T type of elements
 * @tparam N number of dimensions
 * @tparam F Function that can be applied to an reference element of type T
 * @param f a function that accepts a reference of type T and modifies the variable behind the reference.
 * @return Matrix<T, N>& this matrix after modification, allowing chained operations
 */
template <typename T, size_t N>
template <typename F>
Matrix<T, N> &Matrix<T, N>::apply(F f)
{
    for (auto &x : elems)
        f(x);
    return *this;
}

template <typename T, size_t N>
template <typename M, typename F>
Enable_if<Matrix_type<M>(), Matrix<T, N> &> Matrix<T, N>::apply(M &m, F f)
{
    assert(same_extents(desc, m.descriptor()));
    for (auto i = begin(), j = m.begin(); i != end(); ++i, ++j)
        f(*i, *j);
    return *this;
}

template <typename T, size_t N>
Matrix<T, N> &Matrix<T, N>::operator+=(const T &val)
{
    return apply([&](T &a)
                 { a += val; });
}

template <typename T, size_t N>
Matrix<T, N> &Matrix<T, N>::operator-=(const T &val)
{
    return apply([&](T &a)
                 { a -= val; });
}

template <typename T, size_t N>
Matrix<T, N> &Matrix<T, N>::operator*=(const T &val)
{
    return apply([&](T &a)
                 { a *= val; });
}

template <typename T, size_t N>
Matrix<T, N> &Matrix<T, N>::operator/=(const T &val)
{
    return apply([&](T &a)
                 { a /= val; });
}

template <typename T, size_t N>
Matrix<T, N> &Matrix<T, N>::operator%=(const T &val)
{
    return apply([&](T &a)
                 { a %= val; });
}

template <typename T, size_t N>
template <typename M>
Enable_if<Matrix_type<M>(), Matrix<T, N> &> Matrix<T, N>::operator+=(const M &m)
{
    static_assert(m.order() == N, "+=: mismatched Matrix dimensions");
    assert(same_extents(desc, m.descriptor()));

    return apply(m, [](T &a, Value_type<M> &b)
                 { a += b; });
}

template <typename T, size_t N>
template <typename M>
Enable_if<Matrix_type<M>(), Matrix<T, N> &> Matrix<T, N>::operator-=(const M &m)
{
    static_assert(m.order() == N, "+=: mismatched Matrix dimensions");
    assert(same_extents(desc, m.descriptor()));

    return apply(m, [](T &a, Value_type<M> &b)
                 { a -= b; });
}

// Defining Common_type results for operator +