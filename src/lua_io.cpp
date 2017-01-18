#include "lua_io.h"
#include "exceptions.h"
using namespace std;

namespace Lua
{

Reader::Reader(std::istream& input, size_t sizeNumber)
    : m_input(input), m_sizeNumber(sizeNumber)
{
}

Writer::Writer(std::ostream& output, size_t sizeNumber)
    : m_output(output), m_sizeNumber(sizeNumber)
{
}

void Reader::Read(void* dest, size_t size)
{
    m_input.read((char*)dest, (streamsize)size);
    if (m_input.fail()) {
        throw IOException("Unable to read file");
    }
}

void Writer::Write(const void* src, size_t size)
{
    m_output.write((char*)src, (streamsize)size);
    if (m_output.fail()) {
        throw IOException("Unable to write file");
    }
}

string Reader::ReadString()
{
	int size = ReadInt();
    
    string str;
    if (size > 0)
    {
	    char* data = new char[size + 1];
        try
        {
	        Read(data, size);
            data[size] = '\0';
	        str = data;
	        delete[] data;
        }
        catch (...)
        {
            delete[] data;
            throw;
        }
    }
	return str;
}

void Writer::WriteString(const string& str, bool null_if_empty)
{
    if (str.empty() && null_if_empty) {
        WriteInt(0);
    } else {
        WriteInt((int)str.length() + 1);
	    Write(str.c_str(), str.length() + 1);
    }
}

int Reader::ReadInt()
{
	int32_t value;
	m_input.read( (char*)&value, sizeof value );
    if (m_input.fail()) {
        throw IOException("Unable to read file");
    }
	return letohl(value);
}

void Writer::WriteInt(int val)
{
	int32_t value = htolel(val);
	m_output.write( (char*)&value, sizeof value );
    if (m_output.fail()) {
        throw IOException("Unable to read file");
    }
}

int Reader::ReadByte()
{
	uint8_t value;
	m_input.read( (char*)&value, sizeof value );
    if (m_input.fail()) {
        throw IOException("Unable to read file");
    }
	return value;
}

void Writer::WriteByte(int val)
{
	uint8_t value = (uint8_t)val;
	m_output.write( (char*)&value, sizeof value );
    if (m_output.fail()) {
        throw IOException("Unable to read file");
    }
}

double Reader::ReadNumber()
{
    if (m_sizeNumber == 4)
    {
	    float value;
        Read(&value, sizeof value);
        return value;
    }

    double value;
    Read(&value, sizeof value);
	return value;
}

void Writer::WriteNumber(double value)
{
    if (m_sizeNumber == 4) {
	    float v = (float)value;
        Write(&v, sizeof v);
    } else {
	    Write(&value, sizeof value);
    }
}

static Version DetectFileVersion(Reader& reader)
{
    char signature[4];
    reader.Read(signature, 4);
    int version = reader.ReadByte();

    if (strncmp(signature, "\033Lua", 4) == 0) {
        if (version == 0x50) {
            return LUA_50;
        } else if (version == 0x51) {
            int format = reader.ReadByte();
            if (format == 0) {
                return LUA_51;
            } else if (format == 'p') {
                return LUA_UAW;
            }
        }
    } else if (strncmp(signature, "\033Lup", 4) == 0 && version == 0x51) {
        return LUA_EAW;
    }

    return LUA_UNKNOWN;
}

Version DetectFileVersion(istream& input)
{
    Reader reader(input, 4);
    Version version = DetectFileVersion(reader);
    
    // Reset stream
    input.seekg(0);
    return version;
}

}