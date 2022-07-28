#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <vector>
#include <Eigen/Core>

using t_dim = int;
using t_t = double;

template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d;
// using Bezier1d_D_3_2 = Bezier1d<t_t, 3, 2>;

template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d
{
public:
    const constexpr static t_dim PDim = PDim;
    const constexpr static t_dim PhDim = PDim + 1;
    const constexpr static t_dim PFree = PFree;
    const constexpr static t_dim nControlPoints = PFree + 2;

    using t_scalar = TScalar;
    using phc_matrix = Eigen::Matrix<t_scalar, nControlPoints, PhDim>;
    using p_vector = Eigen::Matrix<t_scalar, PDim, 1>;
    using ph_vector = Eigen::Matrix<t_scalar, PhDim, 1>;
    using pcr_vector = Eigen::Matrix<t_scalar, 1, nControlPoints>;

// TOOD: Initialize M and the default segment somehow
protected:
    t_t dt;
    std::vector<phc_matrix> segments;

public:
    static phc_matrix M;
    static phc_matrix DefaultSegment;

public:
    Bezier1d(t_t dt = 1.0 / 16.0) : dt{ dt }, segments{ DefaultSegment } {}

    phc_matrix GetControlPoints(int segment);
    void TranslateControlPoint(int segment, int index, const p_vector &translation);
    void CreateSegment();

    p_vector GetPoint(int segment, t_t t);
    bool GetVelocity(int &segment, t_t &t, p_vector &v);
    void GetEdges(Eigen::Matrix<t_scalar, Eigen::Dynamic, PDim> &P, Eigen::Matrix<int, Eigen::Dynamic, 2> &E);
};

class Bezier1d_D_3_2 : public Bezier1d<double, 3, 2>
{
public:
    Bezier1d_D_3_2(t_t dt = 1.0 / 16.0) : Bezier1d<double, 3, 2>(dt)
    {
    }

    phc_matrix GetControlPoints(int segment)
    {
        return segments[segment];
    }

    void TranslateControlPoint(int segment, int index, const p_vector &translation)
    {
        if (segments.size() > 1)
        {
            throw std::exception("Operation not supported yet");
        }

        ph_vector t;
        t << translation, 0;
        segments[segment].row(index) += t;
    }

    p_vector GetPoint(int segment, t_t t)
    {
        pcr_vector T = CalcTVector(t);
        phc_matrix segmentM = GetControlPoints(segment);
        return (T * M * segmentM).head<3>();
    }

    // TODO: Do with iterator instead
    bool GetVelocity(int &segment, t_t &t, p_vector &v)
    {
        if (t >= 1.0)
        {
            ++segment;
            if (segment >= segments.size())
            {
                return false;
            }
            t = 0.0;
        }

        t_t next_t = std::min(t + dt, 1.0);
        p_vector p_current = GetPoint(segment, t);
        p_vector p_next = GetPoint(segment, next_t);
        v = p_next - p_current;
        t = next_t;

        return true;
    }

private:
    pcr_vector CalcTVector(t_t t)
    {
        pcr_vector T{};
        t_t e = 1.0;
        for (int i = nControlPoints - 1; i >= 0; --i)
        {
            T(i) = e;
            e *= t;
        }
        return T;
    }
};

Bezier1d<t_t, 3, 2>::phc_matrix Bezier1d<t_t, 3, 2>::M = (phc_matrix{} <<
    -1,  3, -3, 1,
     3, -6,  3, 0,
    -3,  3,  0, 0,
     1,  0,  0, 0
).finished();
Bezier1d<t_t, 3, 2>::phc_matrix Bezier1d<t_t, 3, 2>::DefaultSegment = (phc_matrix{} <<
    -1,   0,    0, 1,
    -0.5, 0.75, 0, 1,
     0.5, 0.75, 0, 1,
     1,   0,    0, 1
).finished();

#endif
