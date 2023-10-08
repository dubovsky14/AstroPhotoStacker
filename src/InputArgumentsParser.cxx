#include "../headers/InputArgumentsParser.h"

using namespace std;
using namespace AstroPhotoStacker;

InputArgumentsParser::InputArgumentsParser(int argc, const char **argv) {
    for (int i = 1; i < argc; i++) {
        std::string argument = argv[i];
        if (argument[0] == '-') {
            argument = argument.substr(1);
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                m_arguments[argument] = argv[i + 1];
                i++;
            } else {
                m_arguments[argument] = "";
            }
        }
    }
}
