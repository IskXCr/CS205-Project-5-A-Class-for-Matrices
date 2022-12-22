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
#include <array>
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

    constexpr bool Some() { return false; }

    template <typename... Args>
    constexpr bool Some(bool b, Args... args)
    {
        return b || Some(args...);
    }

    template <typename T, typename T2>
    constexpr bool Same()
    {
        return std::is_same<T, T2>();
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

    struct Slice;

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
         * @brief checks if the arguments can be cnverted to the required `size_t` using `std::is_convertible`.
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

        template <typename... Args>
        constexpr bool Requesting_slice()
        {
            return All((Convertible<Args, size_t>() || Same<Args, Slice>())...) && Some(Same<Args, Slice>()...);
        }

        /**
         * @brief Check whether the given dims is out of bound for a descriptor of a `Mat` object.
         *
         * @tparam N
         * @tparam Dims
         * @param slice
         * @param dims
         * @return true
         * @return false
         */
        template <size_t N, typename... Dims>
        bool check_bounds(const Matrix_slice<N> &slice, Dims... dims)
        {
            size_t indexes[N]{size_t(dims)...};
            return std::equal(indexes, indexes + N, slice.extents.begin(), std::less<size_t>());
        }

        /**
         * @brief Extract target `Matrix` from the source `Matrix`, offset calculated.
         *
         * @tparam Dim if `Dim == 0`, extract a row. Else, extract a column.
         * @tparam N
         * @param n
         * @param desc
         * @param slice
         */
        template <size_t Dim, size_t N>
        void slice_dim(size_t n, Matrix_slice<N> &src, Matrix_slice<N - 1> &dest)
        {
            static_assert(Dim == 1 || Dim == 0, "slice_dim: wrong argument.");
            // debug support
            // static_assert(Dim == 0, "slice_dim: wrong argument.");

            // todo: slice_dim implementation
            if (Dim == 0)
            {
                dest.start = src.start + n * src.strides[0];
                if (src.extents.size() > 1)
                {
                    // for (auto i = src.extents.begin() + 1, j = dest.extents.begin(); i != src.extents.end(); ++i, ++j)
                    //     *j = *i;
                    // for (auto i = src.strides.begin() + 1, j = dest.strides.begin(); i != src.strides.end(); ++i, ++j)
                    //     *j = *i;
                    for (size_t i = 1; i < N; ++i)
                    {
                        dest.extents[i - 1] = src.extents[i];
                        dest.strides[i - 1] = src.strides[i];
                    }
                }
                dest.recalc_size();
            }
            else
            {
                // todo: finish slice_col
                dest.start = src.start + n * src.strides[1];
                if (src.extents.size() > 1)
                {
                    dest.extents[0] = dest.extents[0];
                    dest.strides[0] = src.strides[0];
                    for (size_t i = 2; i < N; ++i)
                    {
                        dest.extents[i - 1] = src.extents[i];
                        dest.strides[i - 1] = src.strides[i];
                    }
                }
                dest.recalc_size();
            }
        }

        template <size_t N>
        bool same_extents(Matrix_slice<N> const &a, Matrix_slice<N> const &b)
        {
            return a.extents == b.extents;
        }

        // template <size_t N, typename T, typename... Dims>
        // size_t do_slice(const Matrix_slice<N> &source, const Matrix_slice<N> &target, const T &s, Dims &...dims)
        // {
        //     size_t m = do_slice_dim<sizeof...(Dims) + 1>(source, target, s);
        //     size_t n = do_slice(source, target, dims...);
        //     return m + n;
        // }

        // template <size_t N>
        // size_t do_slice(const Matrix_slice<N> &source, const Matrix_slice<N> &target)
        // {
        //     return 0;
        // }
    };

    template <typename T, size_t N>
    using Matrix_initializer = typename Matrix_impl::Matrix_init<T, N>::type;

    struct Slice
    {
        // todo: finish direct slice design
        size_t start;
        size_t length;
        size_t stride;

        Slice() : start(-1), length(-1), stride(1) {}
        explicit Slice(size_t s) : start(s), length(-1), stride(1) {}
        Slice(size_t s, size_t l, size_t n = 1) : start(s), length(l), stride(n) {}

        size_t operator()(size_t i) const { return start + i * stride; }
    };

    /**
     * @brief Class that contains critical access information for class `Matrix_ref` and `Matrix`.
     * @note To initialize `desc` of a `Mat` object, you must first call `Matrix_impl::derive_extents`, then call
     *       `Matrix_slice::norm_by_extents, or directly call the constructor. If the target is a new `Matrix` that
     *       owns the elements, you must call `init_full_dim`.
     */
    template <size_t N>
    struct Matrix_slice
    {
        size_t size = 0;               // number of elements in this `Matrix_slice`
        size_t start = 0;              // start offset
        std::array<size_t, N> extents; // #element on each dimension
        std::array<size_t, N> strides; // offset for each dimension (when dimension + 1, offset...)

        // Constructors

        Matrix_slice() = default;
        Matrix_slice(Matrix_slice &&x)
        {
            size = x.size;
            start = x.start;
            extents = std::move(x.extents);
            x.extents = {};
            strides = std::move(x.strides);
            x.strides = {};
            recalc_size();
        };
        Matrix_slice &operator=(Matrix_slice &&x)
        {
            size = x.size;
            start = x.start;
            extents = std::move(x.extents);
            x.extents = {};
            strides = std::move(x.strides);
            x.strides = {};
            recalc_size();
            return *this;
        };
        Matrix_slice(Matrix_slice const &x) : size(x.size), start(x.start), extents(x.extents), strides(x.strides)
        {
            recalc_size();
        }
        Matrix_slice &operator=(Matrix_slice const &x)
        {
            size = x.size;
            start = x.start;
            extents = x.extents;
            strides = x.strides;
            recalc_size();
            return *this;
        };
        ~Matrix_slice() = default;

        /**
         * @brief Construct a new Matrix_slice object from the extents, with start set to 0, and strides and size automatically calculated.
         *
         * @param extents
         */
        Matrix_slice(std::array<size_t, N> extents)
        {
            this->extents = extents;
            recalc_size();
            init_full_dim();
        }

        /**
         * @brief Refresh this slice with only specified extents
         *
         * @param extents
         * @return Matrix_slice&
         */
        Matrix_slice &operator=(std::array<size_t, N> extents)
        {
            *this = Matrix_slice<N>(extents);
            return *this;
        }

        /**
         * @brief Construct a new Matrix_slice object from the template, with start set to 0, and strides and size automatically calculated.
         *
         * @param extents
         */
        template <typename... Exts>
        Matrix_slice(Exts... exts)
        {
            static_assert(Matrix_impl::Requesting_element<Exts...>(), "Matrix_slice: invalid dimension argument.");
            extents = {size_t(exts)...};
            recalc_size();
            init_full_dim();
        }

        template <typename... Exts>
        Matrix_slice &operator=(Exts... exts)
        {
            return *this = Matrix_slice(exts...);
        }

        /**
         * @brief Initialize all strides of a newly created `Matrix`.
         *
         */
        void init_full_dim()
        {
            size_t offset = (extents.size() > 0) ? 1 : 0;

            for (auto i = extents.rbegin(), j = strides.rbegin(); i != extents.rend(); i++, j++)
                *j = offset, offset *= *i;
        }

        /**
         * @brief Recalculate the size of this slice.
         *
         */
        void recalc_size()
        {
            size = 1;
            for (auto &i : extents)
                size *= i;
        }

        /**
         * @brief Return the offset required to access the underlying data.
         *
         * @tparam Dims
         * @param dims
         * @return size_t
         */
        template <typename... Dims>
        size_t operator()(Dims... dims) const
        {
            static_assert(sizeof...(Dims) == N, "Matrix_slice: unmatched subscripting dimension.");

            size_t args[N]{size_t(dims)...}; // copy arguments into an array

            return start + std::inner_product(args, args + N, strides.begin(), size_t(0));
        }
    };

    /**
     * @brief A base for matrices
     *
     */
    template <typename T, size_t N>
    class Matrix_base
    {
    protected:
        Matrix_slice<N> desc; // the descriptor of this matrix

    public:
        // constructors
        Matrix_base() = default;
        Matrix_base(Matrix_base &&) = default; // Move constructor
        Matrix_base &operator=(Matrix_base &&) = default;
        Matrix_base(Matrix_base const &) = default;
        Matrix_base &operator=(Matrix_base const &) = default;
        ~Matrix_base() = default;

        // for convenience
        // Matrix_base

        // properties

        // "flat" element access
        virtual T *data() = 0;
        virtual const T *data() const = 0;

        static constexpr size_t order() { return N; }
        size_t extent(size_t n) const { return desc.extents[n]; }
        size_t rows() const { return desc.extents[0]; }
        size_t columns() const
        {
            if (order() <= 1)
                return order();
            else
                return desc.extents[1];
        }
        size_t size() const { return desc.size; }
        const Matrix_slice<N> descriptor() const { return desc; }

        // Arithmetics template <typename F>
    };

    /**
     * @brief A reference to Matrix, possibly pointing to a submatrix.
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
        // default constructors and assignment operators
        Matrix_ref() = default;
        Matrix_ref(Matrix_ref &&) = default;
        Matrix_ref &operator=(Matrix_ref &&) = default;
        Matrix_ref(Matrix_ref const &) = default;
        Matrix_ref &operator=(Matrix_ref const &) = default;
        ~Matrix_ref() = default;

        Matrix_ref(const Matrix_slice<N> &s, T *p) : ptr{p}
        {
            Matrix_base<T, N>::desc = s;
        }

        // properties

        virtual T *data()
        {
            return ptr;
        };
        virtual const T *data() const { return ptr; };

        // todo: subcripting access

        Matrix_ref<T, N - 1> row(size_t n)
        {
            assert(n < (Matrix_base<T, N>::rows()));
            Matrix_slice<N - 1> row;
            Matrix_impl::slice_dim<0>(n, Matrix_base<T, N>::desc, row);
            // std::cerr << "(Matrix ref row: " << Matrix_base<T, N>::desc.start << ")";
            return Matrix_ref<T, N - 1>(row, data());
        }

        Matrix_ref<T, N - 1> column(size_t n)
        {
            assert(n < (Matrix_base<T, N>::columns()));
            Matrix_slice<N - 1> column;
            Matrix_impl::slice_dim<1>(n, Matrix_base<T, N>::desc, column);
            return Matrix_ref<T, N - 1>(column, data());
        }

        Matrix_ref<T, N - 1> operator[](size_t n) { return row(n); }

        template <typename... Dims>
        Enable_if<Matrix_impl::Requesting_element<Dims...>(), T &> operator()(Dims... dims)
        {
            assert(Matrix_impl::check_bounds(Matrix_base<T, N>::desc, dims...));
            return *(data() + Matrix_base<T, N>::desc(dims...));
        }

        // todo: implement iterators
        // iterator
        /**
         * @brief Matrix_ref_iterator that provide access to member elements that can be read/written.
         * @note The end of this iterator is determined by its degree, with the cursor at the end having its maximum degree exceeded one more.
         *
         */
        class Matrix_ref_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // Tag can impact the performance when used with STL algorithms
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;

            Matrix_ref_iterator() = default;
            Matrix_ref_iterator(Matrix_ref_iterator &&) = default;
            Matrix_ref_iterator &operator=(Matrix_ref_iterator &&) = default;
            Matrix_ref_iterator(Matrix_ref_iterator const &) = default;
            Matrix_ref_iterator &operator=(Matrix_ref_iterator const &) = default;
            ~Matrix_ref_iterator() = default;

            // Initialize
            Matrix_ref_iterator(const Matrix_slice<N> &slice, pointer start, std::array<size_t, N> cursor = {})
                : slice(slice), start(start + slice.start), ptr(start + slice.start), cursor(cursor) {} // Initialize with steps

            virtual reference operator*() const { return *ptr; }
            virtual pointer operator->() { return ptr; }

            // Prefix increment
            virtual Matrix_ref_iterator &operator++()
            {
                // adjusting cursor
                size_t step = N - 1;
                bool overflow = false;
                while (step >= 0)
                {
                    if (++cursor[step] >= slice.extents[step])
                    {
                        if (step == 0)
                        {
                            overflow = true;
                            break;
                        }
                        else
                        {
                            cursor[step] = 0;
                            --step;
                        }
                    }
                    else
                        break;
                }

                // calculate offset
                size_t offset = std::inner_product(slice.strides.begin(), slice.strides.end(), cursor.begin(), 0);
                ptr = start + offset;
                return *this;
            }

            // Postfix increment
            virtual Matrix_ref_iterator operator++(int)
            {
                Matrix_ref_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Matrix_ref_iterator &a, const Matrix_ref_iterator &b) { return a.cursor == b.cursor; }
            friend bool operator!=(const Matrix_ref_iterator &a, const Matrix_ref_iterator &b) { return a.cursor != b.cursor; }

        private:
            const Matrix_slice<N> &slice;
            std::array<size_t, N> cursor; // store the current position in this Matrix, 0 initialized.
            pointer start = nullptr;
            pointer ptr = nullptr;
        };

        virtual Matrix_ref_iterator begin() { return Matrix_ref_iterator(Matrix_base<T, N>::desc, data()); }
        virtual Matrix_ref_iterator end()
        {
            std::array<size_t, N> m_end{};
            m_end[0] = Matrix_base<T, N>::desc.extents[0];
            return Matrix_ref_iterator(Matrix_base<T, N>::desc, data(), std::move(m_end));
        }

        // const iterator
        /**
         * @brief Matrix_ref_const_iterator that provide access to member elements that can be read/written.
         * @note The end of this iterator is determined by its degree, with the cursor at the end having its maximum degree exceeded one more.
         *
         */
        class Matrix_ref_const_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // Tag can impact the performance when used with STL algorithms
            using difference_type = std::ptrdiff_t;
            using value_type = const T;
            using pointer = const T *;
            using reference = const T &;

            Matrix_ref_const_iterator() = default;
            Matrix_ref_const_iterator(Matrix_ref_const_iterator &&) = default;
            Matrix_ref_const_iterator &operator=(Matrix_ref_const_iterator &&) = default;
            Matrix_ref_const_iterator(Matrix_ref_const_iterator const &) = default;
            Matrix_ref_const_iterator &operator=(Matrix_ref_const_iterator const &) = default;
            ~Matrix_ref_const_iterator() = default;

            // Initialize
            Matrix_ref_const_iterator(const Matrix_slice<N> &slice, pointer start, std::array<size_t, N> cursor = {})
                : slice(slice), start(start + slice.start), ptr(start + slice.start), cursor(cursor) {} // Initialize with steps

            virtual const reference operator*() const { return *ptr; }
            virtual pointer operator->() { return ptr; }

            // Prefix increment
            virtual Matrix_ref_const_iterator &operator++()
            {
                // adjusting cursor
                size_t step = N - 1;
                bool overflow = false;
                while (step >= 0)
                {
                    if (++cursor[step] >= slice.extents[step])
                    {
                        if (step == 0)
                        {
                            overflow = true;
                            break;
                        }
                        else
                        {
                            cursor[step] = 0;
                            --step;
                        }
                    }
                    else
                        break;
                }

                // calculate offset
                size_t offset = std::inner_product(slice.strides.begin(), slice.strides.end(), cursor.begin(), 0);
                ptr = start + offset;
                return *this;
            }

            // Postfix increment
            virtual Matrix_ref_const_iterator operator++(int)
            {
                Matrix_ref_const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Matrix_ref_const_iterator &a, const Matrix_ref_const_iterator &b) { return a.cursor == b.cursor; }
            friend bool operator!=(const Matrix_ref_const_iterator &a, const Matrix_ref_const_iterator &b) { return a.cursor != b.cursor; }

        private:
            const Matrix_slice<N> &slice;
            std::array<size_t, N> cursor; // store the current position in this Matrix, 0 initialized.
            pointer start = nullptr;
            pointer ptr = nullptr;
        };

        virtual Matrix_ref_const_iterator cbegin() const { return Matrix_ref_const_iterator(Matrix_base<T, N>::desc, data()); }
        virtual Matrix_ref_const_iterator cend() const
        {
            std::array<size_t, N> m_end{};
            m_end[0] = Matrix_base<T, N>::desc.extents[0];
            return Matrix_ref_const_iterator(Matrix_base<T, N>::desc, data(), std::move(m_end));
        }

        // Arithmetics
        template <typename F>
        auto &apply(F f)
        {
            for (auto &x : (*this))
                f(x);
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix_ref<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix_ref<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix_ref<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix_ref<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix_ref<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
    };

    template <typename T>
    class Matrix_ref<T, 1> : public Matrix_base<T, 1>
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

        Matrix_ref(const Matrix_slice<1> &s, T *p) : ptr{p}
        {
            Matrix_base<T, 1>::desc = s;
        }

        // properties

        virtual T *data() { return ptr; };
        virtual const T *data() const { return ptr; };

        // todo: subcripting access
        T &row(size_t n)
        {
            assert(n < (Matrix_base<T, 1>::rows()));
            // todo: desc access
            // std::cerr << "(Matrix ref row: " << Matrix_base<T, N>::desc.start << ")";
            return *(data() + Matrix_base<T, 1>::desc(n));
        }

        T &operator[](size_t n) { return row(n); }

        template <typename... Dims>
        Enable_if<Matrix_impl::Requesting_element<Dims...>(), T &> &operator()(Dims... dims)
        {
            assert(Matrix_impl::check_bounds(Matrix_base<T, 1>::desc, dims...));
            return *(data() + Matrix_base<T, 1>::desc(dims...));
        }
        // todo: implement iterators
        /**
         * @brief Matrix_ref_iterator that provide access to member elements that can be read/written.
         * @note The end of this iterator is determined by its degree, with the cursor at the end having its maximum degree exceeded one more.
         *
         */
        class Matrix_ref_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // Tag can impact the performance when used with STL algorithms
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;

            Matrix_ref_iterator() = default;
            Matrix_ref_iterator(Matrix_ref_iterator &&) = default;
            Matrix_ref_iterator &operator=(Matrix_ref_iterator &&) = default;
            Matrix_ref_iterator(Matrix_ref_iterator const &) = default;
            Matrix_ref_iterator &operator=(Matrix_ref_iterator const &) = default;
            ~Matrix_ref_iterator() = default;

            // Initialize
            Matrix_ref_iterator(const Matrix_slice<1> &slice, pointer start, std::array<size_t, 1> cursor = {})
                : slice(slice), start(start), ptr(start), cursor(cursor) {} // Initialize with steps

            virtual reference operator*() const { return *ptr; }
            virtual pointer operator->() { return ptr; }

            // Prefix increment
            virtual Matrix_ref_iterator &operator++()
            {
                // adjusting cursor
                size_t step = 0;
                bool overflow = false;
                while (step >= 0)
                {
                    if (++cursor[step] >= slice.extents[step])
                    {
                        if (step == 0)
                        {
                            overflow = true;
                            break;
                        }
                        else
                        {
                            cursor[step] = 0;
                            --step;
                        }
                    }
                    else
                        break;
                }

                // calculate offset
                size_t offset = std::inner_product(slice.strides.begin(), slice.strides.end(), cursor.begin(), 0);
                ptr = start + offset;
                return *this;
            }

            // Postfix increment
            virtual Matrix_ref_iterator operator++(int)
            {
                Matrix_ref_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Matrix_ref_iterator &a, const Matrix_ref_iterator &b) { return a.cursor == b.cursor; }
            friend bool operator!=(const Matrix_ref_iterator &a, const Matrix_ref_iterator &b) { return a.cursor != b.cursor; }

        private:
            const Matrix_slice<1> &slice;
            std::array<size_t, 1> cursor; // store the current position in this Matrix, 0 initialized.
            pointer start = nullptr;
            pointer ptr = nullptr;
        };

        virtual Matrix_ref_iterator begin() { return Matrix_ref_iterator(Matrix_base<T, 1>::desc, data()); }
        virtual Matrix_ref_iterator end()
        {
            std::array<size_t, 1> m_end{};
            m_end[0] = Matrix_base<T, 1>::desc.extents[0];
            return Matrix_ref_iterator(Matrix_base<T, 1>::desc, data(), std::move(m_end));
        }

        // const iterator
        /**
         * @brief Matrix_ref_const_iterator that provide access to member elements that can be read/written.
         * @note The end of this iterator is determined by its degree, with the cursor at the end having its maximum degree exceeded one more.
         *
         */
        class Matrix_ref_const_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag; // Tag can impact the performance when used with STL algorithms
            using difference_type = std::ptrdiff_t;
            using value_type = const T;
            using pointer = const T *;
            using reference = const T &;

            Matrix_ref_const_iterator() = default;
            Matrix_ref_const_iterator(Matrix_ref_const_iterator &&) = default;
            Matrix_ref_const_iterator &operator=(Matrix_ref_const_iterator &&) = default;
            Matrix_ref_const_iterator(Matrix_ref_const_iterator const &) = default;
            Matrix_ref_const_iterator &operator=(Matrix_ref_const_iterator const &) = default;
            ~Matrix_ref_const_iterator() = default;

            // Initialize
            Matrix_ref_const_iterator(const Matrix_slice<1> &slice, pointer start, std::array<size_t, 1> cursor = {})
                : slice(slice), start(start + slice.start), ptr(start + slice.start), cursor(cursor) {} // Initialize with steps

            virtual const reference operator*() const { return *ptr; }
            virtual pointer operator->() { return ptr; }

            // Prefix increment
            virtual Matrix_ref_const_iterator &operator++()
            {
                // adjusting cursor
                size_t step = 0;
                bool overflow = false;
                while (step >= 0)
                {
                    if (++cursor[step] >= slice.extents[step])
                    {
                        if (step == 0)
                        {
                            overflow = true;
                            break;
                        }
                        else
                        {
                            cursor[step] = 0;
                            --step;
                        }
                    }
                    else
                        break;
                }

                // calculate offset
                size_t offset = std::inner_product(slice.strides.begin(), slice.strides.end(), cursor.begin(), 0);
                ptr = start + offset;
                return *this;
            }

            // Postfix increment
            virtual Matrix_ref_const_iterator operator++(int)
            {
                Matrix_ref_const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Matrix_ref_const_iterator &a, const Matrix_ref_const_iterator &b) { return a.cursor == b.cursor; }
            friend bool operator!=(const Matrix_ref_const_iterator &a, const Matrix_ref_const_iterator &b) { return a.cursor != b.cursor; }

        private:
            const Matrix_slice<1> &slice;
            std::array<size_t, 1> cursor; // store the current position in this Matrix, 0 initialized.
            pointer start = nullptr;
            pointer ptr = nullptr;
        };

        virtual Matrix_ref_const_iterator cbegin() const { return Matrix_ref_const_iterator(Matrix_base<T, 1>::desc, data()); }
        virtual Matrix_ref_const_iterator cend() const
        {
            std::array<size_t, 1> m_end{};
            m_end[0] = Matrix_base<T, 1>::desc.extents[0];
            return Matrix_ref_const_iterator(Matrix_base<T, 1>::desc, data(), std::move(m_end));
        }

        // Arithmetics
        template <typename F>
        auto &apply(F f)
        {
            for (auto &x : (*this))
                f(x);
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix_ref<T, 1> &m, const T &val)
        {
            Matrix_ref<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix_ref<T, 1> &m, const T &val)
        {
            Matrix_ref<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix_ref<T, 1> &m, const T &val)
        {
            Matrix_ref<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix_ref<T, 1> &m, const T &val)
        {
            Matrix_ref<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix_ref<T, 1> &m, const T &val)
        {
            Matrix_ref<T, 1> res = m;
            res += val;
            return res;
        }

        template <typename F>
        auto &apply(Matrix_ref<T, 1> &m, F f)
        {
            assert(Matrix_impl::same_extents(m.descriptor(), this->descriptor()));
            for (auto i = begin(), j = m.begin(); i != end(); ++i, ++j)
                f(*i, *j);
            return *this;
        }

        template <typename F>
        auto &apply(Matrix<T, 1> &m, F f)
        {
            assert(Matrix_impl::same_extents(m.descriptor(), this->descriptor()));
            for (auto i = begin(), j = m.begin(); i != end(); ++i, ++j)
                f(*i, *j);
            return *this;
        }
    };

    template <typename T>
    class Matrix_ref<T, 0> : public Matrix_base<T, 0>
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

        Matrix_ref(const Matrix_slice<0> &s, T *p) : ptr{p}
        {
            Matrix_base<T, 0>::desc = s;
        }

        Matrix_ref(const T &x) = delete;
        Matrix_ref &operator=(const T &value)
        {
            *(data() + Matrix_base<T, 0>::desc.start) = value;
            return *this;
        }

        // properties

        virtual T *data() { return ptr; };
        virtual const T *data() const { return ptr; };

        // todo: subcripting access
        T &row()
        {
            // std::cerr << "(Matrix ref row: " << Matrix_base<T, 0>::desc.start << ")";
            return *(data() + Matrix_base<T, 0>::desc.start);
        }

        T &operator()() { return *(data() + Matrix_base<T, 0>::desc.start); }
        const T &operator()() const { return *(data() + Matrix_base<T, 0>::desc.start); }

        operator T &() { return *(data() + Matrix_base<T, 0>::desc.start); }
        operator const T &() { return *(data() + Matrix_base<T, 0>::desc.start); }

        // Arithmetics

        template <typename F>
        auto &apply(F f)
        {
            f(*(data() + Matrix_base<T, 0>::desc.start));
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix_ref<T, 0> &m, const T &val)
        {
            Matrix_ref<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix_ref<T, 0> &m, const T &val)
        {
            Matrix_ref<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix_ref<T, 0> &m, const T &val)
        {
            Matrix_ref<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix_ref<T, 0> &m, const T &val)
        {
            Matrix_ref<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix_ref<T, 0> &m, const T &val)
        {
            Matrix_ref<T, 0> res = m;
            res += val;
            return res;
        }
    };

    template <typename T, size_t N>
    class Matrix : public Matrix_base<T, N>
    {
    protected:
        std::vector<T> elems; // storing the elements of the matrix

    public:
        // variable properties
        using value_type = T;
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;

        // default constructors

        /**
         * @brief Construct a new Matrix object that matches the dimension. You cannot use this raw `Matrix`.
         *
         */
        Matrix() = default;
        Matrix(Matrix &&) = default; // Move constructor
        Matrix &operator=(Matrix &&) = default;
        Matrix(Matrix const &) = default;
        Matrix &operator=(Matrix const &) = default;
        ~Matrix() = default;

        /**
         * @brief Allowing assignment from other Matrices
         *
         */
        template <typename U>
        Matrix(Matrix_ref<U, N> const &x)
        {
            assert((Convertible<T, U>()));
            assert(Matrix_impl::same_extents(x.descriptor(), this->descriptor()));
            elems.reserve(x.size());
            elems.insert(elems.begin(), x.cbegin(), x.cend());
        }

        /**
         * @brief
         *
         */
        template <typename U>
        Matrix(Matrix<U, N> const &x)
        {
            assert((Convertible<T, U>()));
            assert(Matrix_impl::same_extents(x.descriptor(), this->descriptor()));
            elems.reserve(x.size());
            elems.insert(elems.begin(), x.cbegin(), x.cend());
        }

        /**
         * @brief Construct a new Matrix object by specifying the extents.
         * @todo solve problematic definition
         *
         * @tparam Exts
         * @param exts
         */
        template <typename... Exts>
        explicit Matrix(Exts... exts)
        {
            // std::cerr << "Matrix(Exts constructor) Entered." << std::endl;
            Matrix_base<T, N>::desc = Matrix_slice<N>{exts...};
            // std::cerr << "Matrix(Exts constructor): Slice created." << std::endl;
            Matrix_base<T, N>::desc.init_full_dim();
            // std::cerr << "Matrix(Exts constructor): Dimension initialized. The target size is " << Matrix_base<T, N>::desc.size << std::endl;
            elems.resize(Matrix_base<T, N>::desc.size);
        }

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
            Matrix_base<T, N>::desc.recalc_size();
            Matrix_base<T, N>::desc.init_full_dim();
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
        Matrix_ref<T, N - 1> row(size_t n)
        {
            assert(n < (Matrix_base<T, N>::rows()));
            Matrix_slice<N - 1> row;
            Matrix_impl::slice_dim<0>(n, Matrix_base<T, N>::desc, row);
            // std::cerr << "(Matrix src row: " << Matrix_base<T, N>::desc.start << ")";
            return Matrix_ref<T, N - 1>(row, data());
        }

        Matrix_ref<T, N - 1> column(size_t n)
        {
            assert(n < (Matrix_base<T, N>::columns()));
            Matrix_slice<N - 1> column;
            Matrix_impl::slice_dim<1>(n, Matrix_base<T, N>::desc, column);
            return Matrix_ref<T, N - 1>(column, data());
        }

        Matrix_ref<T, N - 1> operator[](size_t n) { return row(n); }

        template <typename... Dims>
        Enable_if<Matrix_impl::Requesting_element<Dims...>(), T &> operator()(Dims... dims)
        {
            assert(Matrix_impl::check_bounds(Matrix_base<T, N>::desc, dims...));
            return *(data() + Matrix_base<T, N>::desc(dims...));
        }

        // template <typename... Dims>
        // Enable_if<Matrix_impl::Requesting_slice<Dims...>(), Matrix_ref<T, N>> operator()(const Dims &...dims)
        // {
        //     Matrix_slice<N> d;
        //     d.start = Matrix_impl::do_slice(Matrix_base<T, N>::desc, d, dims...);
        //     d.recalc_size();
        //     return Matrix_ref{d, data()};
        // }

        // iterator
        iterator begin() { return elems.begin(); }
        iterator end() { return elems.end(); }

        iterator cbegin() const { return elems.cbegin(); }
        iterator cend() const { return elems.cend(); }

        // Arithmetics
        template <typename F>
        auto &apply(F f)
        {
            for (auto &x : (*this))
                f(x);
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix<T, N> &m, const T &val)
        {
            Matrix<T, N> res = m;
            res += val;
            return res;
        }
    };

    template <typename T>
    class Matrix<T, 1> : public Matrix_base<T, 1>
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
         * @brief Allowing assignment from other Matrices
         *
         */
        template <typename U>
        Matrix(Matrix_ref<U, 1> const &x)
        {
            assert((Convertible<T, U>()));
            assert(Matrix_impl::same_extents(Matrix_base<T, 1>::desc, x.descriptor()));
            elems.reserve(x.size());
            elems.insert(elems.begin(), x.cbegin(), x.cend());
        }

        /**
         * @brief
         *
         */
        template <typename U>
        Matrix(Matrix<U, 1> const &x)
        {
            assert((Convertible<T, U>()));
            assert(Matrix_impl::same_extents(Matrix_base<T, 1>::desc, x.descriptor()));
            elems.reserve(x.size());
            elems.insert(elems.begin(), x.cbegin(), x.cend());
        }

        /**
         * @brief Construct a new Matrix object by specifying the extents.
         * @todo solve problematic definition
         *
         * @tparam Exts
         * @param exts
         */
        template <typename... Exts>
        explicit Matrix(Exts... exts)
        {
            // std::cerr << "Matrix(Exts constructor)" << std::endl;
            Matrix_base<T, 1>::desc = Matrix_slice<1>(exts...);
            Matrix_base<T, 1>::desc.init_full_dim();
            elems.resize(Matrix_base<T, 1>::desc.size);
        }

        /**
         * @brief Construct a new Matrix object using list-initialized constructor.
         *
         * @param init
         * @note
         * Rules:
         * If either a default constructor or an initializer-list constructor could be invoked, prefer the default constructor.
         * If both an initializer-list constructor and an "ordinary constructor" could be invoked, prefer the initializer-list constructor.
         */
        Matrix(Matrix_initializer<T, 1> init)
        {
            Matrix_base<T, 1>::desc.extents = Matrix_impl::derive_extents<1>(init);
            Matrix_base<T, 1>::desc.recalc_size();
            Matrix_base<T, 1>::desc.init_full_dim();
            elems.reserve(Matrix_base<T, 1>::desc.size);
            Matrix_impl::insert_flat(init, elems);
            assert((elems.size() == Matrix_base<T, 1>::desc.size));
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

        T &row(size_t n)
        {
            assert(n < (Matrix_base<T, 1>::rows()));
            // todo: desc access
            // std::cerr << "(Matrix ref row: " << Matrix_base<T, N>::desc.start << ")";
            return *(data() + Matrix_base<T, 1>::desc(n));
        }

        T &operator[](size_t n) { return row(n); }

        template <typename... Dims>
        Enable_if<Matrix_impl::Requesting_element<Dims...>(), T &> operator()(Dims... dims)
        {
            assert(Matrix_impl::check_bounds(Matrix_base<T, 1>::desc, dims...));
            return *(data() + Matrix_base<T, 1>::desc(dims...));
        }

        // iterators
        iterator begin() { return elems.begin(); }
        iterator end() { return elems.end(); }

        iterator cbegin() const { return elems.cbegin(); }
        iterator cend() const { return elems.cend(); }

        // todo: Do specializations when N == 1
        // todo: copy implementation to Matrix_ref class

        // todo: arithemtic operations
        // template <typename F>
        // Matrix<T, N> apply(F f)
        // {
        //     for ()
        // }
        template <typename F>
        auto &apply(F f)
        {
            for (auto &x : (*this))
                f(x);
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix<T, 1> &m, const T &val)
        {
            Matrix<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix<T, 1> &m, const T &val)
        {
            Matrix<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix<T, 1> &m, const T &val)
        {
            Matrix<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix<T, 1> &m, const T &val)
        {
            Matrix<T, 1> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix<T, 1> &m, const T &val)
        {
            Matrix<T, 1> res = m;
            res += val;
            return res;
        }
    };

    template <typename T>
    class Matrix<T, 0> : Matrix_base<T, 0>
    {
    private:
        T elem;

    public:
        using value_type = T;

        // default constructors
        Matrix() = default;
        Matrix(Matrix &&) = default; // Move constructor
        Matrix &operator=(Matrix &&) = default;
        Matrix(Matrix const &) = default;
        Matrix &operator=(Matrix const &) = default;
        ~Matrix() = default;

        /**
         * @brief Allowing assignment from other Matrices
         *
         */

        Matrix(const T &x) : elem(x) {}
        Matrix &operator=(const T &value)
        {
            elem = value;
            return *this;
        }

        static constexpr size_t order() { return 0; };

        // properties

        // "flat" element access
        virtual T *data() { return &elem; };
        virtual const T *data() const { return &elem; };

        T &row() { return elem; }
        const T &row() const { return elem; }

        T &operator()() { return elem; }
        const T &operator()() const { return elem; }

        operator T &() { return elem; }
        operator const T &() { return elem; }

        template <typename F>
        auto &apply(F f)
        {
            f(elem);
            return *this;
        }

        auto &operator+=(const T &val)
        {
            return apply([&](T &a)
                         { a += val; });
        }
        auto &operator-=(const T &val)
        {
            return apply([&](T &a)
                         { a -= val; });
        }
        auto &operator*=(const T &val)
        {
            return apply([&](T &a)
                         { a *= val; });
        }
        auto &operator/=(const T &val)
        {
            return apply([&](T &a)
                         { a /= val; });
        }
        auto &operator%=(const T &val)
        {
            return apply([&](T &a)
                         { a %= val; });
        }
        friend auto &operator+(const Matrix<T, 0> &m, const T &val)
        {
            Matrix<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator-(const Matrix<T, 0> &m, const T &val)
        {
            Matrix<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator*(const Matrix<T, 0> &m, const T &val)
        {
            Matrix<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator/(const Matrix<T, 0> &m, const T &val)
        {
            Matrix<T, 0> res = m;
            res += val;
            return res;
        }
        friend auto &operator%(const Matrix<T, 0> &m, const T &val)
        {
            Matrix<T, 0> res = m;
            res += val;
            return res;
        }
    };

};

#endif