#ifndef ORIENTATION_FILTER_H
#define ORIENTATION_FILTER_H

#include "matrix.h"
#include <cmath>

class OrientationFilter {
public:
    OrientationFilter(double q_angle = 0.001, double q_bias = 0.003, double r_measure = 0.03) {
        x(0, 0) = 0.0;
        x(1, 0) = 0.0;

        P = Matrix<2, 2>::Identity();

        Q(0, 0) = q_angle;
        Q(1, 1) = q_bias;

        R(0, 0) = r_measure;
    }

    void update(double angle_m, double gyro_rate, double dt) {
        Matrix<2, 2> F = Matrix<2, 2>::Identity();
        F(0, 1) = -dt;

        x(0, 0) += (gyro_rate - x(1, 0)) * dt;
        
        P = F * P * F.transpose() + Q;

        Matrix<1, 2> H;
        H(0, 0) = 1.0;

        double y = angle_m - x(0, 0);

        double S = (H * P * H.transpose())(0, 0) + R(0, 0);

        Matrix<2, 1> K;
        Matrix<2, 1> PHt = P * H.transpose();
        K(0, 0) = PHt(0, 0) / S;
        K(1, 0) = PHt(1, 0) / S;

        x(0,0) += K(0, 0) * y;
        x(1,0) += K(1, 0) * y;

        Matrix<2, 2> I = Matrix<2, 2>::Identity();
        P = (I - (K * H)) * P;
    }

    double getAngle() const { return x(0, 0); }

private:
    Matrix<2, 1> x; 
    Matrix<2, 2> P; 
    Matrix<2, 2> Q; 
    Matrix<1, 1> R; 
};

#endif
