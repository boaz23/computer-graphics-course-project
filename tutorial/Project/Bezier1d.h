#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <vector>
#include <Eigen/Core>

using t_dim =  unsigned long long int;
using t_t = double;

template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d;
// using Bezier1d_D_3_2 = Bezier1d<t_t, 3, 2>;

class Bezier
{
public:
    static t_t dt;
};

t_t Bezier::dt = 1.0 / 32.0;

template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d
{
public:
    const constexpr static t_dim PDim = PDim;
    const constexpr static t_dim PhDim = PDim + 1;
    const constexpr static t_dim PFree = PFree;
    const constexpr static t_dim nControlPoints = PFree + 2;
    const constexpr static t_dim lastControlPointIndex = nControlPoints - 1;

    using t_scalar = TScalar;
    using phc_matrix = Eigen::Matrix<t_scalar, nControlPoints, PhDim>;
    using p_vector = Eigen::Matrix<t_scalar, PDim, 1>;
    using ph_vector = Eigen::Matrix<t_scalar, PhDim, 1>;
    using pcr_vector = Eigen::Matrix<t_scalar, 1, nControlPoints>;

    using P_Matrix = Eigen::Matrix<t_scalar, Eigen::Dynamic, PDim>;
    using E_Matrix = Eigen::Matrix<int, Eigen::Dynamic, 2>;

// TOOD: Initialize M and the default segment somehow
protected:
    std::vector<phc_matrix> segments;

public:
    static phc_matrix M;
    static phc_matrix DefaultSegment;

public:
    Bezier1d() : segments{ DefaultSegment } {}

    phc_matrix &GetControlPoints(int segment);
    const phc_matrix &GetControlPoints(int segment) const;
    void TranslateControlPoint(int segment, int index, const p_vector &translation, bool shouldPreseveC1 = true);
    void CreateSegment();

    p_vector GetPoint(int segment, t_t t);
    bool GetNextPoint(int &segment, t_t &t, p_vector &p);
    void GetEdges(P_Matrix &P, E_Matrix &E);
};

class Bezier1d_D_3_2 : public Bezier1d<double, 3, 2>
{
public:
    Bezier1d_D_3_2() : Bezier1d<double, 3, 2>()
    {
    }

    phc_matrix &GetControlPoints(int segment)
    {
        return segments[segment];
    }

    const phc_matrix &GetControlPoints(int segment) const
    {
        return segments[segment];
    }

    void TranslateControlPoint(int segment, int index, const p_vector &translation, bool shouldPreseveC1 = true)
    {
        if (segments.size() > 1 && shouldPreseveC1)
        {
            throw std::exception("Operation not supported yet");
        }

        ph_vector t{};
        t << translation, 0;
        segments[segment].row(index) += t;
        if (index == 0)
        {
            int prevSegment = segment - 1;
            if (prevSegment >= 0)
            {
                segments[prevSegment].row(lastControlPointIndex) += t;
            }
        }
        else if (index == lastControlPointIndex)
        {
            int nextSegment = segment + 1;
            if (nextSegment < segments.size())
            {
                segments[nextSegment].row(0) += t;
            }
        }
    }

    p_vector GetPoint(int segment, t_t t)
    {
        pcr_vector T = CalcTVector(t);
        const phc_matrix &segmentM = GetControlPoints(segment);
        return (T * M * segmentM).head<PDim>();
    }

    void CreateSegment(bool shouldPreseveC1 = true)
    {
        phc_matrix lastSegment = GetControlPoints(segments.size() - 1);

        if (shouldPreseveC1)
        {
            throw std::exception("Operation not supported yet");

            //phc_matrix lastSegment = GetControlPoints(segments.size() - 1);
            //ph_vector toLastControlPoint = lastSegment.row(lastControlPointIndex) - lastSegment.row(nControlPoints - 2);
            //double d = toLastControlPoint.norm();
            //ph_vector p1 =
        }

        phc_matrix newSegmentM{};
        ph_vector p_control = lastSegment.row(lastControlPointIndex);
        newSegmentM.row(0) = p_control;

        ph_vector p_prev = DefaultSegment.row(0);
        for (unsigned int i = 1; i < nControlPoints; ++i)
        {
            ph_vector p_current = DefaultSegment.row(i);
            p_control += p_current - p_prev;
            newSegmentM.row(i) = p_control;
            p_prev = p_current;
        }

        segments.push_back(newSegmentM);
    }

    // TODO: Do with iterator instead
    bool GetNextPoint(int &segment, t_t &t, p_vector &p)
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

        t_t next_t = std::min(t + Bezier::dt, 1.0);
        //p_vector p_current = GetPoint(segment, t);
        p_vector p_current = p;
        p_vector p_next = GetPoint(segment, next_t);
        p += p_next - p_current;
        t = next_t;

        return true;
    }

    void GetEdges(P_Matrix &P, E_Matrix &E)
    {
        ResizeEdgesMatrices(P, E);

        t_t t = 0.0;
        int segment = 0;
        p_vector p_prev{ GetControlPoints(0).row(0).head<PDim>() }, p_current{ p_prev };
        Eigen::Index i = 0;

        P.row(i) << p_prev.transpose();
        while (GetNextPoint(segment, t, p_current))
        {
            Eigen::Index next_i = i + 1;
            P.row(next_i) << p_current.transpose();
            E.row(i) << i, next_i;
            p_prev = p_current;
            i = next_i;
        }
    }

private:
    pcr_vector CalcTVector(t_t t)
    {
        pcr_vector T{};
        t_t e = 1.0;
        for (int i = static_cast<int>(lastControlPointIndex); i >= 0; --i)
        {
            T(i) = e;
            e *= t;
        }
        return T;
    }

    void ResizeEdgesMatrices(P_Matrix &P, E_Matrix &E)
    {
        auto nPointsPerSegment = static_cast<Eigen::Index>(std::ceil(1.0 / Bezier::dt)) + 1;
        size_t segmentsCount = segments.size();
        Eigen::Index nPoints = nPointsPerSegment*segmentsCount - (segmentsCount-1);
        P.resize(nPoints, Eigen::NoChange);
        E.resize(nPoints - 1, Eigen::NoChange);
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
