/**
 * @file mat.hpp
 * @author IskXCr (IskXCr@outlook.com)
 * @brief This file contains the header of a basic Matrix class,
 *        with its major reference from @ref "The C++ Programming Language, Fourth Edition" by Bjarne Stroustrup.
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022 IskXCr
 *
 */
#ifndef MAT_H
#define MAT_H

// Header files
#include <iostream>
#include <type_traits> // For converting types and undefining some functions
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstddef>

/**
 * @brief The namespace for this c++ project.
 *        The major class is <code>utils::Mat</code>.
 */
namespace utils
{
    // Provided function specialization for some types.

    /**
     * To check whether a definition should exist, for example operator!=, use
     *  @code
     *      template<typename T>
            auto operator!=(const T &a, const T &b) -> decltype(!a==b)
            {
                return !(a==b);
            }
        @endcode
     */

    // ------------------------------
    // Check if a function should exist
    template <bool B, typename T>
    using Enable_if = typename std::enable_if<B, T>::type; // Enable the declaration of a function IFF the boolean condition is true.

    template <typename T>
    bool Is_class()
    {
        return std::is_class<T>::value;
    }

    // ------------------------------
    // Matrix_initializer
    template <typename T, size_t N>
    using Matrix_initializer = typename Matrix_impl::Matrix_init<T, N>::type;

    /**
     * @brief The namespace for the implementation routines for utils::Matrix class.
     *
     */
    namespace Matrix_impl
    {
    }

    /**
     * Requirements for this matrix class:
     *      (Most of the requirements comes from "The C++ Programming Language, Fourth Edition" by Bjarne Stroustrup.)
     *      ~ N dimensions, where N is a parameter that can vary from 0 to many, without specialized code for every dimension.
     *      ~ Element type can be anything.
     *      ~ Mathematical operations should apply to any type if such operations make sense.
     *      ~ Array-style subscripting and Fortran-style subscripting using one index per dimension. Example: mat1[1][2][3], mat1(1,2,3) for an element in a matrix;
     *      ~ Subscripting should be potentially fast and potentially range checked.
     *      ~ Move assignment and move constructor to ensure efficient passing of Matrix results and to eliminate expensive temporaries. Avoid memory hard copy.
     *      ~ Mathematical operations, including =, ==, +, -, *, etc.
     *      ~ (Submatrices) Region of Interest (ROI) with a way to read, write and pass around references to ROIs, Matrix_refs, for use for both reading and writing elements.
     *      ~ Try to avoid memory leak and to release memory multiple times.
     *      ~ Combine critical operations, for example, m*v + v2 as single function call.
     *
     * Additional requirements:
     *      ~ Divide-and-conquer algorithm && parallel execution for some mathematical operations (from Project 4)
     */

    /**
     * @brief A template class for matrices that can hold elements of any type that can be stored.
     *
     * @tparam T type of elements
     * @tparam N number of dimensions, or order
     *
     * @note The order of this Matrix is the dimension of this matrix. The extent of any specific dimension determines how many elements uniquely determine that dimension.
     *       For example, a Matrix of order 3 has three dimensions, each of which can be uniquely represented by i, j, k respectively, where i, j, k are positive integers.
     *       A Matrix of order 0 is also possible, meaning that the Matrix represents a scalar element.
     *       See reference: @ref "The C++ Programming Language, Fourth Edition" by Bjarne Stroustrup.
     */
    template <typename T, size_t N>
    class Matrix
    {
    private:
        Matrix_slice<N> desc; // Slice defining extents in the N dimensions
        std::vector<T> elems; // Store the elements of this matrix

    public:
        static constexpr size_t order = N;
        using value_type = T;
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;

        // ------------------------------
        // Constructors. Preferably use default constructors due to the usage of std::vector.
        // Expected behavior: memberwise copy or move of Matrix::desc and Matrix::elems.

        Matrix() = default;
        Matrix(Matrix &&) = default; // Move
        Matrix &operator=(Matrix &&) = default;
        Matrix(Matrix const &) = default;
        Matrix &operator=(Matrix const &) = default; // Copy
        ~Matrix() = default;

        // ------------------------------
        // Other constructors, from submatrix or reference to Matrix.

        template <typename U>
        Matrix(const Matrix_ref<U, N> &); // construct from Matrix_ref

        template <typename U>
        Matrix &operator=(const Matrix_ref<U, N> &); // assign from Matrix_ref

        template <typename... Exts>
        explicit Matrix(Exts... exts); // the user must explicityly specify the extents

        Matrix(Matrix_initializer<T, N>);            // initialize from list
        Matrix &operator=(Matrix_initializer<T, N>); // assign from list

        template <typename U>
        Matrix(initializer_list<U>) = delete; // don't use {} initializers except for elements

        // ------------------------------
        // Properties of the matrix

        // Return number of dimensions
        static constexpr size_t order() { return N; }

        // Return #elements in the nth dimension
        size_t extent(size_t n) const { return desc.extents[n]; }

        // Return total number of elements
        size_t size() const { return elems.size(); }

        // Return the slice defining subscripting
        const Matrix_slice<N> &descriptor() const { return desc; }

        // ------------------------------
        // Element Accessors

        // Get element access
        T *data() { return elems.data(); }

        const T *data() const { return elems.data(); }

        // m(i,j,k) subscripting with integers
        template <typename... Args>
        Enable_if<Matrix_impl::Requesting_element<Args...>(), T &> operator()(Args... args);

        template <typename... Args>
        Enable_if<Matrix_impl::Requesting_element<Args...>(), const T &> operator()(Args... args) const;

        // m(s1, s2, s3) subscripting with slices
        template <typename... Args>
        Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<T, N>> operator()(const Args &...args);

        template <typename... Args>
        Enable_if<Matrix_impl::Requesting_slice<Args...>(), Matrix_ref<const T, N>> operator()(const Args &...args);

        // m[i] row access
        Matrix_ref<T, N - 1> operator[](size_t i) { return row(i); }
        Matrix_ref<const T, N - 1> operator[](size_t i) { return row(i); }

        // row access
        Matrix_ref<T, N - 1> row(size_t n);
        Matrix_ref<const T, N - 1> row(size_t n) const;

        // column access
        Matrix_ref<T, N - 1> col(size_t n);
        Matrix_ref<const T, N - 1> col(size_t n) const;

        // ...
    };

    /**
     * @brief A reference to a sub-Matrix. This reference does not hold the target Matrix, and thus only have the permission to read.
     * 
     * @tparam T 
     * @tparam N 
     */
    template <typename T, size_t N>
    class Matrix_ref
    {
    private:
        Matrix_slice<N> desc; // the shape of the matrix
        T *ptr;               // The first element in the matrix

    public:
        Matrix_ref(const Matrix_slice<N> &s, T *p) : desc{s}, ptr{p} {}
    };
}

#endif