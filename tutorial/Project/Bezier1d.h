#pragma once
#ifndef PROJECT_BEZIER_1D
#define PROJECT_BEZIER_1D

#include <vector>
#include <Eigen/Core>

using t_dim = unsigned long long int;
using t_t = double;
using t_index = size_t;
using t_index_signed = std::make_signed<t_index>::type;

template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d;
// using Bezier1d_D_3_2 = Bezier1d<t_t, 3, 2>;

class Bezier
{
public:
    static t_t dt;
};

// Each segment is implemented with a matrix.
// Each row is a control point using homogeneous coordinates.
template<typename TScalar, t_dim PDim, t_dim PFree> class Bezier1d
{
public:
    // p - points
    // h - homogeneous
    // c - control
    // r - row

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
    // mapping each edge to respective segment
    using ESMAP_Vector = Eigen::Matrix<int, Eigen::Dynamic, 1>;

    // TOOD: Initialize M and the default segment somehow
protected:
    std::vector<phc_matrix> segments;

public:
    static phc_matrix M;
    static phc_matrix DefaultSegment;

public:
    Bezier1d() : segments { DefaultSegment } {}

    phc_matrix &GetControlPoints(t_index segment)
    {
        return segments[segment];
    }

    const phc_matrix &GetControlPoints(t_index segment) const
    {
        return segments[segment];
    }

    void TranslateControlPoint_C0(t_index segment, int index, const p_vector &translation)
    {
        ph_vector translationH{};
        translationH << translation, 0;
        TranslateControlPoint_C0_Core(segment, index, translationH);
    }

    void TranslateControlPoint_C1(t_index segment, int index, const p_vector &translation)
    {
        ph_vector translationH{};
        translationH << translation, 0;
        TranslateControlPoint_C1_Core(segment, index, translationH);
    }

    t_index CreateSegment()
    {
        phc_matrix P{};
        const phc_matrix &lastSegment = GetControlPoints(segments.size() - 1);

        ph_vector p_prev = lastSegment.row(lastControlPointIndex);
        ph_vector p_control = p_prev;
        P.row(0) = p_control;
        for (auto i = static_cast<t_index_signed>(lastControlPointIndex) - 1; i >= 0; --i)
        {
            ph_vector p_current = lastSegment.row(i);
            p_control += p_prev - p_current;
            P.row(lastControlPointIndex - i) = p_control;
            p_prev = p_current;
        }

        t_index i = segments.size();
        segments.push_back(P);
        return i;
    }

    void RemoveLastSegment()
    {
        segments.pop_back();
    }

    p_vector GetPoint(t_index segment, t_t t)
    {
        pcr_vector T = CalcTVector(t);
        const phc_matrix &P = GetControlPoints(segment);
        return (T * M * P).head<PDim>();
    }

    // TODO: Do with iterator instead
    bool GetNextPoint(t_index &segment, t_t &t, p_vector &p)
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

    void GetEdges(P_Matrix &P, E_Matrix &E, ESMAP_Vector &ESMAP)
    {
        ResizeEdgesMatrices(P, E, ESMAP);

        t_t t = 0.0;
        t_index segment = 0;
        p_vector p_prev{ GetControlPoints(0).row(0).head<PDim>() }, p_current{ p_prev };
        Eigen::Index i = 0;

        P.row(i) << p_prev.transpose();
        while (GetNextPoint(segment, t, p_current))
        {
            Eigen::Index next_i = i + 1;
            P.row(next_i) << p_current.transpose();
            E.row(i) << (int)i, (int)next_i;
            ESMAP.row(i) << (int)segment;
            p_prev = p_current;
            i = next_i;
        }
    }

    int SegmentsCount() { return (int)segments.size(); }
private:
    void TranslateControlPoint_C0_Core(t_index segment, int index, const ph_vector &translation)
    {
        segments[segment].row(index) += translation;
        if (index == 0)
        {
            auto prevSegment = static_cast<t_index_signed>(segment) - 1;
            if (prevSegment >= 0)
            {
                segments[prevSegment].row(lastControlPointIndex) += translation;
            }
        }
        else if (index == lastControlPointIndex)
        {
            t_index nextSegment = segment + 1;
            if (nextSegment < segments.size())
            {
                segments[nextSegment].row(0) += translation;
            }
        }
    }
    void TranslateControlPoint_C1_Core(t_index segment, int index, const ph_vector &translation)
    {
        TranslateControlPoint_C0_Core(segment, index, translation);
        if (index == 0)
        {
            if (segment > 0)
            {
                TranslateControlPoint_C0_Core(segment - 1, lastControlPointIndex - 1, 2*translation);
            }
        }
        else if (index == 1)
        {
            if (segment > 0)
            {
                TranslateControlPoint_C0_Core(segment - 1, lastControlPointIndex - 1, -translation);
            }
        }
        else if (index == lastControlPointIndex - 1)
        {
            if (segment < segments.size() - 1)
            {
                TranslateControlPoint_C0_Core(segment + 1, 1, -translation);
            }
        }
        else if (index == lastControlPointIndex)
        {
            if (segment < segments.size() - 1)
            {
                TranslateControlPoint_C0_Core(segment, lastControlPointIndex - 1, 2*translation);
            }
        }
    }

    pcr_vector CalcTVector(t_t t)
    {
        pcr_vector T{};
        t_t e = 1.0;
        for (auto i = static_cast<t_index_signed>(lastControlPointIndex); i >= 0; --i)
        {
            T(i) = e;
            e *= t;
        }
        return T;
    }

    void ResizeEdgesMatrices(P_Matrix &P, E_Matrix &E, ESMAP_Vector& ESMAP)
    {
        auto nPointsPerSegment = static_cast<Eigen::Index>(std::ceil(1.0 / Bezier::dt)) + 1;
        size_t segmentsCount = segments.size();
        Eigen::Index nPoints = nPointsPerSegment*segmentsCount - (segmentsCount-1);
        P.resize(nPoints, Eigen::NoChange);
        E.resize(nPoints - 1, Eigen::NoChange);
        ESMAP.resize(nPoints - 1, Eigen::NoChange);
    }
};

class Bezier1d_D_3_2 : public Bezier1d<double, 3, 2>
{
public:
    Bezier1d_D_3_2() : Bezier1d<double, 3, 2>()
    {}
};
#endif
