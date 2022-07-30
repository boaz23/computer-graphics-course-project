#include "Bezier1d.h"

t_t Bezier::dt = 1.0 / 32.0;

Bezier1d<t_t, 3, 2>::phc_matrix Bezier1d<t_t, 3, 2>::M = (phc_matrix{} <<
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 3, 0, 0,
    1, 0, 0, 0
    ).finished();
Bezier1d<t_t, 3, 2>::phc_matrix Bezier1d<t_t, 3, 2>::DefaultSegment = (phc_matrix{} <<
    -1, 0, 0, 1,
    -0.5, 0.75, 0, 1,
    0.5, 0.75, 0, 1,
    1, 0, 0, 1
    ).finished();
