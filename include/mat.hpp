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
#include <numeric>
#include <cassert>
#include <initializer_list>
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
    // useful routines

    // check if a function should exist
    template <bool B, typename T>
    using Enable_if = typename std::enable_if<B, T>::type; // enable the declaration of a function IFF the boolean condition is true.

    template <typename T, typename T2>
    using Common_type = typename std::common_type<T, T2>::type;

    template <typename T>
    using Value_type = T; // todo: change this

    template <typename T, typename T2>
    using Convertible = std::is_convertible<T, T2>;

    template <typename T>
    bool Is_class()
    {
        return std::is_class<T>::value;
    }

    constexpr bool All() { return true; }

    template <typename... Args>
    constexpr bool All(bool b, Args... args)
    {
        return b && All(args...);
    }

    // ------------------------------
    // Declarations

    template <size_t N>
    struct Matrix_slice;

    template <typename T, size_t N>
    class Matrix_base;

    template <typename T, size_t N>
    class Matrix_ref;

    template <typename T, size_t N>
    class Matrix;

    // ------------------------------

    /**
     * @brief Implementation of some useful routines for the Matrix class.
     *
     */
    namespace Matrix_impl
    {
        /**
         * @brief for initializing a matrix using list initializer
         *
         * @tparam T
         * @tparam N
         */
        template <typename T, size_t N>
        struct Matrix_init
        {
            using type = std::initializer_list<typename Matrix_init<T, N - 1>::type>;
        };

        template <typename T>
        struct Matrix_init<T, 1>
        {
            using type = std::initializer_list<T>;
        };

        template <typename T>
        struct Matrix_init<T, 0>; // Undefined on purpose

        /**
         * @brief Check if the initializer_list is well formed.
         *
         * @tparam List
         * @param list
         * @return true
         * @return false
         */
        template <typename List>
        bool check_non_jagged(const List &list)
        {
            auto i = list.begin();
            for (auto j = i + 1; j != list.end(); ++j)
                if (i->size() != j->size())
                    return false;
            return true;
        }

        template <size_t N, typename l, typename List>
        Enable_if<(N == 1), void> add_extents(l &first, const List &list)
        {
            *first++ = list.size();
        }

        /**
         * @brief Recursively add extents to the array, the outmost extent first.
         *
         * @tparam N
         * @tparam l
         * @tparam List
         */
        template <size_t N, typename l, typename List>
        Enable_if<(N > 1), void> add_extents(l &first, const List &list)
        {
            assert(check_non_jagged(list));
            *first = list.size();
            add_extents<N - 1>(++first, *list.begin());
        }

        /**
         * @brief Return an `array` of extents from an `initializer_list`
         *
         * @tparam N
         * @tparam List
         * @param list
         * @return std::array<size_t, N>
         */
        template <size_t N, typename List>
        std::array<size_t, N> derive_extents(const List &list)
        {
            std::array<size_t, N> a;
            auto f = a.begin();
            add_extents<N>(f, list);
            return a;
        }

        /**
         * @brief Insert a sequence into the vector when dimension is 1.
         *
         * @tparam T
         * @tparam Vec
         * @param first
         * @param last
         * @param vec
         */
        template <typename T, typename Vec>
        void add_list(const T *first, const T *last, Vec &vec)
        {
            vec.insert(vec.end(), first, last);
        }

        template <typename T, typename Vec>
        void add_list(const std::initializer_list<T> *first, const std::initializer_list<T> *last, Vec &vec)
        {
            for (; first != last; ++first)
                add_list(first->begin(), first->end(), vec);
        }

        /**
         * @brief Insert all elements of a `std::initializer_list` into the corresponding vector
         *
         * @tparam T
         * @tparam Vec
         * @param list
         * @param vec
         */
        template <typename T, typename Vec>
        void insert_flat(std::initializer_list<T> list, Vec &vec)
        {
            add_list(list.begin(), list.end(), vec);
        }

        /**
         * @brief checks that the arguments can be cnverted to the required `size_t` using `std::is_convertible`.
         *
         * @tparam Args
         * @return true
         * @return false
         */
        template <typename... Args>
        constexpr bool Requesting_element()
        {
            return All(Convertible<Args, size_t>()...);
        }

        // template <typename... Args>
        // constexpr bool Requesting_slice()
        // {
        //     return All((Convertible<Args, size_t>() || Same<Args, slice>())...) && Some(Same<Args, slice>()...);
        // }

        template <size_t Dim, size_t N>
        void slice_dim(size_t n, Matrix_slice<N> &desc, Matrix_slice<N - 1> &slice)
        {
            static_assert(Dim == 1 || Dim == 0, "Wrong argument.");

            // todo: slice_dim implementation
            if (Dim == 0)
            {
            }
            else
            {
            }
        }

    };

    template <typename T, size_t N>
    using Matrix_initializer = typename Matrix_impl::Matrix_init<T, N>::type;

    /**
     * @brief Class that contains critical access information for class `Matrix_ref` and `Matrix`.
     * @note To initialize `desc` of a matrix, you must first call `Matrix_impl::derive_extents`, then call
     *       `Matrix_slice::norm_by_extents, or directly call the constructor.
     */
    template <size_t N>
    struct Matrix_slice
    {
        size_t size = 0;
        size_t start = 0;
        std::array<size_t, N> extents;
        std::array<size_t, N> strides;

        // Constructors

        Matrix_slice() = default;

        template <typename... Exts>
        Matrix_slice(Exts... exts) : extents{exts...}
        {
            norm_by_extents();
        }

        void norm_by_extents()
        {
            size = (extents.size() > 0) ? 1 : 0;

#pragma omp parallel for
            for (auto &i : extents)
                size *= i;
        }
    };

    // todo: finish template specialization
    template <>
    struct Matrix_slice<1>
    {
        size_t size = 0;
        size_t start = 0;
        std::array<size_t, 1> extents;
        std::array<size_t, 1> strides;

        // Constructors

        Matrix_slice() = default;

        void norm_by_extents()
        {
            size = (extents.size() > 0) ? 1 : 0;

#pragma omp parallel for
            for (auto &i : extents)
                size *= i;
        }

        size_t operator()(size_t i) const { return i; }
    };

    /**
     * @brief A base for matrices
     *
     */
    template <typename T, size_t N>
    class Matrix_base
    {
    protected:
        Matrix_slice<N> desc; // the dimension of matrix

    public:
        // properties

        // "flat" element access
        virtual T *data() = 0;
        virtual const T *data() const = 0;

        static constexpr size_t order() { return N; }
        size_t extent(size_t n) const { return desc.extents[n]; }
        size_t rows() const { return desc.extents[0]; }
        size_t columns() const
        {
            if (order() == 1)
                return 1;
            else
                return desc.extents[1];
        }
        size_t size() const { return desc.size; }
        Matrix_slice<N> descriptor() { return desc; }

        // accessors

        // template <typename... Args>
        // Enable_if<Matrix_impl::Requesting_element<Args...>(), T &>
        // operator()(Args... args)
        // {

        // }

        // todo: const accessors, const Args...

        // Arithmetics
    };

    /**
     * @brief
     *
     * @tparam T
     * @tparam N
     * @note End user is not supposed to declare/define this type by themselves.
     */
    template <typename T, size_t N>
    class Matrix_ref : public Matrix_base<T, N>
    {
    private:
        T *ptr; // The pointer to original elements

    public:
        // default constructors
        Matrix_ref() = default;
        Matrix_ref(Matrix_ref &&) = default;
        Matrix_ref &operator=(Matrix_ref &&) = default;
        Matrix_ref(Matrix_ref const &) = default;
        Matrix_ref &operator=(Matrix_ref const &) = default;
        ~Matrix_ref() = default;

        Matrix_ref(const Matrix_slice<N> &s, T *p) : Matrix_base<T, N>::desc{s}, ptr{p} {}

        // properties

        virtual T *data() { return ptr; };
        virtual const T *data() const { return ptr; };

        // todo: subcripting access
        Matrix_ref<T, N - 1> row(size_t n)
        {
            assert(n < (Matrix_base<T, N>::rows()));
            Matrix_slice<N - 1> row;
            Matrix_impl::slice_dim<0>(n, Matrix_base<T, N>::desc, row);
            return Matrix_ref(row, data());
        }

        // todo: implement iterators
    };

    template <typename T, size_t N>
    class Matrix : public Matrix_base<T, N>
    {
    protected:
        std::vector<T> elems;

    public:
        // variable properties
        using value_type = T;
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;

        // default constructors
        Matrix() = default;
        Matrix(Matrix &&) = default; // Move constructor
        Matrix &operator=(Matrix &&) = default;
        Matrix(Matrix const &) = default;
        Matrix &operator=(Matrix const &) = default;
        ~Matrix() = default;

        /**
         * @brief Construct a new Matrix object by specifying the extents.
         * @todo solve problematic definition
         *
         * @tparam Exts
         * @param exts
         */
        template <typename... Exts>
        explicit Matrix(Exts... exts) : Matrix_base<T, N>::desc{exts...}, elems{Matrix_base<T, N>::desc.size} {}

        /**
         * @brief Construct a new Matrix object using list-initialized constructor.
         *
         * @param init
         * @note
         * Rules:
         * If either a default constructor or an initializer-list constructor could be invoked, prefer the default constructor.
         * If both an initializer-list constructor and an "ordinary constructor" could be invoked, prefer the initializer-list constructor.
         */
        Matrix(Matrix_initializer<T, N> init)
        {
            Matrix_base<T, N>::desc.extents = Matrix_impl::derive_extents<N>(init);
            Matrix_base<T, N>::desc.norm_by_extents();
            elems.reserve(Matrix_base<T, N>::desc.size);
            Matrix_impl::insert_flat(init, elems);
            assert((elems.size() == Matrix_base<T, N>::desc.size));
        }

        // Disallow the usage of direct initialize except for elements.
        // template <typename U>
        // Matrix(std::initializer_list<U>) = delete;
        // template <typename U>
        // Matrix &operator=(std::initializer_list<U>) = delete;

        // Properties

        // "flat" element access
        virtual T *data() { return elems.data(); };
        virtual const T *data() const { return elems.data(); };

        // todo: subcripting access
        // todo: Do specializations when N == 1
        // todo: copy implementation to Matrix_ref class
    };

    template <typename T>
    class Matrix<T, 0>
    {
    private:
        T elem;

    public:
        using value_type = T;

        Matrix(const T &x) : elem(x) {}
        Matrix &operator=(const T &value)
        {
            elem = value;
            return *this;
        }

        T &operator()() { return elem; }
        const T &operator()() const { return elem; }

        static constexpr size_t order() { return 0; };
    };

};
#endif