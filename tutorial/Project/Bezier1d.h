#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <vector>
#include <Eigen/Core>

template<typename TScalar, int PDim> class Bezier1d;
using Bezier1d_D3 = Bezier1d<double, 3>;

template<typename TScalar, int PDim> class Bezier1d
{
public:
    const constexpr static int PDim = PDim;
    const constexpr static int PhDim = PDim + 1;

    using phmatrix = Eigen::Matrix<TScalar, PhDim, PhDim>;
    using pvector = Eigen::Matrix<TScalar, PDim, 1>;

// TOOD: Initialize M and the default segment somehow
//public:
//    double dt;
//    int degree;
//    std::vector<phmatrix> segments;
//
//    const static phmatrix M;

public:
    //Bezier1d(int polyDegree, double dt = 1.0 / 16.0);

    pvector GetPoint(int segment, float t);
    phmatrix GetControlPoints(int segment);
    void TranslateControlPoint(int segment, int index, pvector translation);
    void CreateSegment();
    bool GetVelocity(int &segment, double &t, pvector &p);
    void GetEdges(Eigen::Matrix<TScalar, Eigen::Dynamic, PDim> &P, Eigen::Matrix<int, Eigen::Dynamic, 2> &E);
};

template<> class Bezier1d<double, 3>
{
public:
    const constexpr static int PDim = 3;
    const constexpr static int PhDim = PDim + 1;

    using phmatrix = Eigen::Matrix<double, PhDim, PhDim>;
    using pvector = Eigen::Matrix<double, PDim, 1>;

private:
    double dt;
    int nPoints;
    std::vector<phmatrix> segments;

    static phmatrix M;
    static phmatrix DefaultSegment;

public:
    Bezier1d(int polyDegree, double dt = 1.0 / 16.0) : dt{ dt }, nPoints{ polyDegree + 1 }, segments{ DefaultSegment }
    {
    }

    phmatrix GetControlPoints(int segment)
    {
        return segments[segment];
    }
};

Bezier1d<double, 3>::phmatrix Bezier1d<double, 3>::M = (phmatrix{} <<
    -1,  3, -3, 1,
     3, -6,  3, 0,
    -3,  3,  0, 0,
     1,  0,  0, 0
).finished();
Bezier1d<double, 3>::phmatrix Bezier1d<double, 3>::DefaultSegment = (phmatrix{} <<
    -1,   0,    0, 1,
    -0.5, 0.75, 0, 1,
     0.5, 0.75, 0, 1,
     1,   0,    0, 1
).finished();

#endif
