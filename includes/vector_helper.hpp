#pragma once

#include <vector>


template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b) {
    std::vector<T> result;
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

/*
// Like the cons operator in haskell, but it adds to tail, xs | x
template <typename T>
std::vector<T> operator|(const T& elem, const std::vector<T>& vec) {
    std::vector<T> result;
    result.push_back(elem);
    result.insert(result.end(), vec.begin(), vec.end());
    return result;
}
*/