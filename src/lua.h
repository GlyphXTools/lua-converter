#ifndef LUP_H
#define LUP_H
// Petroglyph LUA format

#include <fstream>
#include <vector>
#include <string>
#include "types.h"

namespace Lua
{

enum Type
{
    TNIL = 0,
    TBOOLEAN,
    TLIGHTUSERDATA,
    TNUMBER,
    TSTRING,
    TTABLE,
    TFUNCTION,
    TUSERDATA,
    TTHREAD
};

struct Local
{
    std::string name;
    int         startPC;
    int         endPC;
};

typedef int           Line;
typedef std::string   UpValue;
typedef unsigned long Instruction;

struct Constant
{
    Type        type;
    std::string str;
    double      number;
    bool        boolean;
};

struct Function
{
    std::string    name;
    int            lineDefined;
    int            lastLineDefined;
    unsigned char  nUpvalues;
    unsigned char  nParameters;
    unsigned char  isVararg;
    unsigned char  maxStackSize;
    std::vector<Line>        lines;
    std::vector<Local>       locals;
    std::vector<UpValue>     upvalues;
    std::vector<Constant>    constants;
    std::vector<Function>    functions;
    std::vector<Instruction> instructions;
};

struct File
{
	Function function;
};

namespace Lua50
{
    void ReadFile(std::istream& input, File& file, bool isLup);
    void WriteFile(std::ostream& output, const File& file, bool isLup);
}

namespace Lua51
{
    void ReadFile(std::istream& input, File& file, bool isLup);
    void WriteFile(std::ostream& output, const File& file, bool isLup);
}

enum Version
{
    LUA_UNKNOWN = -1,
    LUA_50,
    LUA_51,
    LUA_EAW,
    LUA_UAW,
};

Version DetectFileVersion(std::istream& input);

}
#endif