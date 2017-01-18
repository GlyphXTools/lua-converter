#ifndef LUA_IO_H
#define LUA_IO_H

#include "lua.h"

namespace Lua
{

class Reader
{
    std::istream& m_input;
    size_t        m_sizeNumber;

public:
    void        Read(void* dest, size_t size);
    int         ReadByte();
    int         ReadInt();
    double      ReadNumber();
    std::string ReadString();

    Reader(std::istream& input, size_t sizeNumber);
};

class Writer
{
    std::ostream& m_output;
    size_t        m_sizeNumber;

public:
    void Write(const void* src, size_t size);
    void WriteByte(int val);
    void WriteInt(int val);
    void WriteNumber(double value);
    void WriteString(const std::string& str, bool null_if_empty = false);

    Writer(std::ostream& output, size_t sizeNumber);
};

}

#endif