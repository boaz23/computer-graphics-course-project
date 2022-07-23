#pragma once
#ifndef ENIGEREWORK_IGL_OPENGL_UTIL_H
#define ENIGEREWORK_IGL_OPENGL_UTIL_H

template<typename T> size_t mathModulo(T n, size_t m)
{
	return n >= 0 ? (static_cast<size_t>(n) % m) : ((m - 1) - (static_cast<size_t>((-n) - 1) % m));
}

template<typename T> size_t addCyclic(T index, T delta, size_t max)
{
	return mathModulo<T>(index + delta, max);
}

#endif
