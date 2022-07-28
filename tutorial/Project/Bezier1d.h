#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <Eigen/Core>

template<typename TScalar, int PDim> class Bezier1d;
using Bezier1d_D3 = Bezier1d<double, 3>;

template<typename TScalar, int PDim> class Bezier1d
{
public:
    const constexpr static int PhDim = PDim + 1;

    Eigen::Matrix<double, PDim, 1> GetPoint(int segment, float t);
    Eigen::Matrix<double, PhDim, PhDim> GetControlPoints();
    void TranslateControlPoint(int segment, int index, Eigen::Matrix<double, PDim, 1> translation);
    void CreateSegment();
    void GetEdges(Eigen::Matrix<double, Eigen::Dynamic, PDim> &P, Eigen::Matrix<int, Eigen::Dynamic, 2> &E);
};

#endif
