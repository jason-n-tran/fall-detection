#ifndef MATRIX_H
#define MATRIX_H

#include <array>
#include <stdexcept>
#include <algorithm>

template <size_t Rows, size_t Cols>
class Matrix {
public:
    std::array<double, Rows * Cols> data;

    Matrix() {
        data.fill(0.0);
    }

    static Matrix Identity() {
        static_assert(Rows == Cols, "Identity matrix must be square");
        Matrix m;
        for (size_t i = 0; i < Rows; ++i) {
            m(i, i) = 1.0;
        }
        return m;
    }

    double& operator()(size_t r, size_t c) {
        return data[r * Cols + c];
    }

    const double& operator()(size_t r, size_t c) const {
        return data[r * Cols + c];
    }

    Matrix<Rows, Cols> operator+(const Matrix<Rows, Cols>& other) const {
        Matrix<Rows, Cols> res;
        for (size_t i = 0; i < Rows * Cols; ++i) {
            res.data[i] = data[i] + other.data[i];
        }
        return res;
    }

    Matrix<Rows, Cols> operator-(const Matrix<Rows, Cols>& other) const {
        Matrix<Rows, Cols> res;
        for (size_t i = 0; i < Rows * Cols; ++i) {
            res.data[i] = data[i] - other.data[i];
        }
        return res;
    }

    template <size_t OtherCols>
    Matrix<Rows, OtherCols> operator*(const Matrix<Cols, OtherCols>& other) const {
        Matrix<Rows, OtherCols> res;
        if constexpr (Rows == 2 && Cols == 2 && OtherCols == 2) {
            res(0,0) = (*this)(0,0)*other(0,0) + (*this)(0,1)*other(1,0);
            res(0,1) = (*this)(0,0)*other(0,1) + (*this)(0,1)*other(1,1);
            res(1,0) = (*this)(1,0)*other(0,0) + (*this)(1,1)*other(1,0);
            res(1,1) = (*this)(1,0)*other(0,1) + (*this)(1,1)*other(1,1);
        } else if constexpr (Rows == 2 && Cols == 2 && OtherCols == 1) {
            res(0,0) = (*this)(0,0)*other(0,0) + (*this)(0,1)*other(1,0);
            res(1,0) = (*this)(1,0)*other(0,0) + (*this)(1,1)*other(1,0);
        } else {
            for (size_t i = 0; i < Rows; ++i) {
                for (size_t k = 0; k < Cols; ++k) {
                    double temp = (*this)(i, k);
                    for (size_t j = 0; j < OtherCols; ++j) {
                        res(i, j) += temp * other(k, j);
                    }
                }
            }
        }
        return res;
    }

    Matrix<Rows, Cols> operator*(double scalar) const {
        Matrix<Rows, Cols> res;
        for (size_t i = 0; i < Rows * Cols; ++i) {
            res.data[i] = data[i] * scalar;
        }
        return res;
    }

    Matrix<Cols, Rows> transpose() const {
        Matrix<Cols, Rows> res;
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                res(j, i) = (*this)(i, j);
            }
        }
        return res;
    }

    Matrix<Rows, Cols> inverse() const {
        static_assert(Rows == Cols, "Only square matrices can be inverted");
        if constexpr (Rows == 1) {
            Matrix<1, 1> res;
            res(0, 0) = 1.0 / (*this)(0, 0);
            return res;
        } else if constexpr (Rows == 2) {
            double det = (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
            if (std::abs(det) < 1e-9) throw std::runtime_error("Matrix is singular");
            Matrix<2, 2> res;
            res(0, 0) = (*this)(1, 1) / det;
            res(0, 1) = -(*this)(0, 1) / det;
            res(1, 0) = -(*this)(1, 0) / det;
            res(1, 1) = (*this)(0, 0) / det;
            return res;
        } else {
            throw std::runtime_error("Generic inverse not implemented. Use 2x2.");
        }
    }
};

#endif
