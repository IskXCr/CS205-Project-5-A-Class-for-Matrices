#ifndef MAT_H
#define MAT_H

#include <iostream>
#include <stdexcept>
#include <memory>
#include <cstddef>

namespace test
{
    template <typename _Tp>
    class Mat
    {
    protected:
        size_t rows;
        size_t cols;
        std::shared_ptr<_Tp[]> data; /* Use shared_ptr to manage data references. */

        bool checkSizeParam(size_t rows, size_t cols) const
        {
            if (rows == 0 || cols == 0)
                return false;
            size_t res = rows * cols;
            if (res / rows != cols)
                return false;
            return true;
        }

    public:
        Mat(size_t rows, size_t cols = 1) : rows(rows), cols(cols)
        {
            if (!checkSizeParam(rows, cols))
                throw new std::invalid_argument("Invalid arguments on creating new matrix.");
            try
            {
                data = std::make_shared<_Tp[]>(new _Tp[rows * cols]{});
            }
            catch (const std::bad_alloc &e)
            {
                throw;
            }
        }

        Mat(const Mat&other) : rows(other.rows), cols(other.cols)
        {
            data = other.data;
        }

        /**
         * Release the underlying data pointer.
         * 
        */
        void release() no except
        {
            ~data();
            data = nullptr;
        }
    };
}

#endif