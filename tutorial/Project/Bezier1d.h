#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <vector>
#include <Eigen/Core>

template<typename TScalar, int PDim, int PFree> class Bezier1d;
using Bezier1d_D_3_2 = Bezier1d<double, 3, 2>;

template<typename TScalar, int PDim, int PFree> class Bezier1d
{
public:
    const constexpr static int PDim = PDim;
    const constexpr static int PhDim = PDim + 1;
    const constexpr static int PFree = PFree;
    const constexpr static int nControlPoints = PFree + 2;

    using ph_matrix = Eigen::Matrix<TScalar, nControlPoints, PhDim>;
    using p_vector = Eigen::Matrix<TScalar, PDim, 1>;

// TOOD: Initialize M and the default segment somehow
//public:
//    double dt;
//    int degree;
//    std::vector<ph_matrix> segments;
//
//    const static ph_matrix M;

public:
    //Bezier1d(int polyDegree, double dt = 1.0 / 16.0);

    ph_matrix GetControlPoints(int segment);
    void TranslateControlPoint(int segment, int index, p_vector translation);
    void CreateSegment();

    p_vector GetPoint(int segment, float t);
    bool GetVelocity(int &segment, double &t, p_vector &p);
    void GetEdges(Eigen::Matrix<TScalar, Eigen::Dynamic, PDim> &P, Eigen::Matrix<int, Eigen::Dynamic, 2> &E);
};

template<> class Bezier1d<double, 3, 2>
{
public:
    const constexpr static int PDim = 3;
    const constexpr static int PhDim = PDim + 1;
    const constexpr static int PFree = 2;
    const constexpr static int nControlPoints = 2 + 2;

    using ph_matrix = Eigen::Matrix<double, nControlPoints, PhDim>;
    using p_vector = Eigen::Matrix<double, PDim, 1>;

private:
    double dt;
    int nPoints;
    std::vector<ph_matrix> segments;

    static ph_matrix M;
    static ph_matrix DefaultSegment;

public:
    Bezier1d(int polyDegree, double dt = 1.0 / 16.0) : dt{ dt }, nPoints{ polyDegree + 1 }, segments{ DefaultSegment }
    {
    }

    ph_matrix GetControlPoints(int segment)
    {
        return segments[segment];
    }
};

Bezier1d<double, 3, 2>::ph_matrix Bezier1d<double, 3, 2>::M = (ph_matrix{} <<
    -1,  3, -3, 1,
     3, -6,  3, 0,
    -3,  3,  0, 0,
     1,  0,  0, 0
).finished();
Bezier1d<double, 3, 2>::ph_matrix Bezier1d<double, 3, 2>::DefaultSegment = (ph_matrix{} <<
    -1,   0,    0, 1,
    -0.5, 0.75, 0, 1,
     0.5, 0.75, 0, 1,
     1,   0,    0, 1
).finished();

#endif
