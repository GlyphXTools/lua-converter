#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "lua.h"
using namespace std;

class LuaFormat
{
public:
    virtual void Load(istream& input, Lua::File& file) const = 0;
    virtual void Save(ostream& output, const Lua::File& file) const = 0;
};

class SpecificLuaFormat : public LuaFormat
{
    bool m_isLup;
    bool m_isNew;

    void Load(istream& input, Lua::File& file) const
    {
        if (m_isNew) {
            Lua::Lua51::ReadFile(input, file, m_isLup);
        } else {
            Lua::Lua50::ReadFile(input, file, m_isLup);
        }
    }
    
    void Save(ostream& output, const Lua::File& file) const
    {
        if (m_isNew) {
            Lua::Lua51::WriteFile(output, file, m_isLup);
        } else {
            Lua::Lua50::WriteFile(output, file, m_isLup);
        }
    }

public:
    SpecificLuaFormat(bool isNew, bool isLup)
        : m_isNew(isNew), m_isLup(isLup) 
    {}
};

static const SpecificLuaFormat g_FormatLua50 (false, false);
static const SpecificLuaFormat g_FormatLua51 (true,  false);
static const SpecificLuaFormat g_FormatLupEaW(false, true);
static const SpecificLuaFormat g_FormatLupUaW(true,  true);

static const struct {
    const LuaFormat* input;
    const LuaFormat* output;
} LuaFormats[] = {
    {&g_FormatLua50,  &g_FormatLupEaW},
    {&g_FormatLua51,  &g_FormatLupUaW},
    {&g_FormatLupEaW, &g_FormatLua50},
    {&g_FormatLupUaW, &g_FormatLua51},
    {NULL}
};

int main(int argc, char* argv[])
{
    // Parse the arguments
	if (argc != 3)
	{
		cerr << "Lup/Lua converter 1.1, by Mike Lankamp." << endl
		     << "Syntax: luacvt <src-file> <dest-file>" << endl
             << endl
             << "The program will read a Lua or Lup file and convert it to a Lup or Lua file." << endl
             << "The format of the source file is automatically detected and the appropriate" << endl
             << "destination format selected. EaW/FoC Luas will be converted to Lua 5.0 files" << endl
             << "and vica versa. UaW Luas will be converted to Lua 5.1 files and vica versa." << endl;
        return 1;
	}

    const char* src  = argv[1];
    const char* dest = argv[2];

#ifdef NDEBUG
	try
#endif
	{
		ifstream input(src, ios_base::binary | ios_base::in);
		if (!input.is_open())
		{
			cerr << "Unable to open input file \"" << src << "\"" << endl;
            return 1;
		}

        Lua::Version version = Lua::DetectFileVersion(input);
        if (version == Lua::LUA_UNKNOWN)
        {
            cerr << "Input file is not recognized as a supported Lua file" << endl;
            return 1;
        }

        Lua::File file;
        LuaFormats[version].input->Load(input, file);
        input.close();

		ofstream output(dest, ios_base::binary | ios_base::out);
		if (!output.is_open())
		{
			cerr << "Unable to open output file \"" << dest << "\"" << endl;
            return 1;
		}

        LuaFormats[version].output->Save(output, file);
	}
#ifdef NDEBUG
	catch (exception& e)
	{
		cerr << e.what() << endl;
        return 1;
	}
#endif
	return 0;
}