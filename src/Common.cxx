#include "../headers/Common.h"


#include <string>
#include <cmath>

using namespace std;

float RandomUniform()  {
    // Be carefull about the brackets, otherwise RAND_MAX will overflow
    return (float(rand())) / (float(RAND_MAX)+1);
}

bool Contains(const string &main_string, const string &substring)    {
    return (main_string.find(substring)) != std::string::npos;
};

bool StartsWith(const string &main_string, const string &prefix)    {
    const unsigned int prefix_lenght = prefix.length();
    const unsigned int string_lenght = main_string.length();
    if (prefix_lenght > string_lenght) return false;
    return prefix == main_string.substr(0, prefix_lenght);
};

bool EndsWith(const string &main_string, const string &suffix)    {
    const unsigned int suffix_lenght = suffix.length();
    const unsigned int string_lenght = main_string.length();
    if (suffix_lenght > string_lenght) return false;
    return suffix == main_string.substr(string_lenght-suffix_lenght, string_lenght);
};
