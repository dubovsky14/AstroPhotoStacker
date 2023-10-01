#pragma once
#include <cmath>
#include <string>

float RandomUniform();

template <class X>
inline X pow2(X x) {return x*x;};

bool Contains(const std::string &main_string, const std::string &substring);

bool StartsWith(const std::string &main_string, const std::string &prefix);

bool EndsWith(const std::string &main_string, const std::string &suffix);