#include "lua_io.h"
#include "exceptions.h"
using namespace std;

namespace Lua {
namespace Lua50 {

//
// Reading
//

// Header structure
#pragma pack(1)
struct Header
{
	char signature[4];
	unsigned char version;
	unsigned char endianness;
	unsigned char sizeInt;
	unsigned char sizeSize_t;
	unsigned char sizeInstruction;
	unsigned char sizeOp;
	unsigned char sizeA;
	unsigned char sizeB;
	unsigned char sizeC;
	unsigned char sizeNumber;
	uint64_t      testNumber;
};
#pragma pack()

//
// Reading
//

static Reader ReadHeader(istream& input, bool isLup)
{
    Header header;
	input.read( (char*)&header, sizeof header );
    if (input.fail()) {
        throw IOException("Unable to read file");
    }

	// Validate header
	const char*   sig = (isLup) ? "\033Lup" : "\033Lua";
	unsigned char ver = (isLup) ? 0x51 : 0x50;

	if ((strncmp(header.signature, sig, 4) != 0) ||
		(header.version         != ver) ||
		(header.endianness      != 1) ||
		(header.sizeInt			!= 4) ||
		(header.sizeSize_t		!= 4) ||
		(header.sizeInstruction != 4) ||
		(header.sizeOp			!= 6) ||
		(header.sizeA			!= 8) ||
		(header.sizeB			!= 9) ||
		(header.sizeC			!= 9) ||
		(header.sizeNumber		!= 8) ||
		(header.testNumber		!= 0x417df5e7689309B6))
	{
		throw BadFileException();
	}
    return Reader(input, header.sizeNumber);
}

static void ReadLines(Reader& reader, vector<Line>& lines)
{
	lines.resize( reader.ReadInt() );
	for (size_t i = 0; i < lines.size(); i++)
	{
		lines[i] = reader.ReadInt();
	}
}

static void ReadLocals(Reader& reader, vector<Local>& locals)
{
	locals.resize( reader.ReadInt() );
	for (size_t i = 0; i < locals.size(); i++)
	{
        Local& local = locals[i];
		local.name    = reader.ReadString();
		local.startPC = reader.ReadInt();
		local.endPC   = reader.ReadInt();
	}
}

static void ReadUpvalues(Reader& reader, vector<UpValue>& upvalues)
{
	upvalues.resize( reader.ReadInt() );
	for (size_t i = 0; i < upvalues.size(); i++)
	{
		upvalues[i] = reader.ReadString();
	}
}

static void ReadConstants(Reader& reader, vector<Constant>& constants)
{
	constants.resize( reader.ReadInt() );
	for (size_t i = 0; i < constants.size(); i++)
	{
		Constant& constant = constants[i];
		constant.type = (Type)reader.ReadByte();
		switch (constant.type)
		{
			case TNUMBER:  constant.number  = reader.ReadNumber(); break;
			case TSTRING:  constant.str     = reader.ReadString(); break;
            case TBOOLEAN: constant.boolean = reader.ReadByte() != 0; break;
			case TNIL:     break;
			default:
				throw BadFileException();
		}
	}
}

static void ReadFunction(Reader& reader, Function& function, bool isLup);

static void ReadFunctions(Reader& reader, vector<Function>& functions, bool isLup)
{
	functions.resize( reader.ReadInt() );
	for (size_t i = 0; i < functions.size(); i++)
	{
		ReadFunction(reader, functions[i], isLup);
	}
}

static void ReadInstructions(Reader& reader, vector<Instruction>& instructions)
{
    instructions.resize( reader.ReadInt() );
	for (size_t i = 0; i < instructions.size(); i++)
	{
		instructions[i] = reader.ReadInt();
	}
}

static void ReadFunction(Reader& reader, Function& function, bool isLup)
{
	function.name            = reader.ReadString();
	function.lineDefined     = reader.ReadInt();
    function.lastLineDefined = -1;
	if (isLup)
	{
        // Read the special Petroglyph integer
		reader.ReadInt();
	}
	function.nUpvalues    = reader.ReadByte();
	function.nParameters  = reader.ReadByte();
	function.isVararg     = reader.ReadByte();
	function.maxStackSize = reader.ReadByte();
    ReadLines       (reader, function.lines);
    ReadLocals      (reader, function.locals);
	ReadUpvalues    (reader, function.upvalues);
	ReadConstants   (reader, function.constants);
	ReadFunctions   (reader, function.functions, isLup);
	ReadInstructions(reader, function.instructions);
}

void ReadFile(istream& input, File& file, bool isLup)
{
    Reader reader = ReadHeader(input, isLup);
	ReadFunction(reader, file.function, isLup);
}

//
// writing
//

static Writer WriteHeader(ostream& output, bool isLup)
{
	// Fill header
    Header header;
	memcpy(header.signature, (isLup) ? "\033Lup" : "\033Lua", 4);
	header.version          = (isLup) ? 0x51 : 0x50;
	header.endianness       = 1;
	header.sizeInt			= 4;
	header.sizeSize_t		= 4;
	header.sizeInstruction  = 4;
	header.sizeOp			= 6;
	header.sizeA			= 8;
	header.sizeB			= 9;
	header.sizeC			= 9;
	header.sizeNumber		= 8;
	header.testNumber		= 0x417df5e7689309B6;

    Writer writer(output, header.sizeNumber);
	writer.Write( (char*)&header, sizeof header );
    return writer;
}

static void WriteLines(Writer& writer, const vector<Line>& lines)
{
	writer.WriteInt((unsigned int)lines.size());
	for (size_t i = 0; i < lines.size(); i++)
	{
		writer.WriteInt(lines[i]);
	}
}

static void WriteLocals(Writer& writer, const vector<Local>& locals)
{
	writer.WriteInt((unsigned int)locals.size());
	for (size_t i = 0; i < locals.size(); i++)
	{
		writer.WriteString(locals[i].name);
		writer.WriteInt(locals[i].startPC);
		writer.WriteInt(locals[i].endPC);
	}
}

static void WriteUpvalues(Writer& writer, const vector<UpValue>& upvalues)
{
	writer.WriteInt((unsigned int)upvalues.size());
	for (size_t i = 0; i < upvalues.size(); i++)
	{
		writer.WriteString(upvalues[i]);
	}
}

static void WriteConstants(Writer& writer, const vector<Constant>& constants )
{
	writer.WriteInt((unsigned int)constants.size());
	for (size_t i = 0; i < constants.size(); i++)
	{
		writer.WriteByte(constants[i].type);
		switch (constants[i].type)
		{
			case TNUMBER:  writer.WriteNumber(constants[i].number); break;
			case TSTRING:  writer.WriteString(constants[i].str); break;
            case TBOOLEAN: writer.WriteByte  (constants[i].boolean ? 1 : 0); break;
			case TNIL:     break;
		}
	}
}

static void WriteFunction(Writer& writer, const Function& function, int* petroValue);

static void WriteFunctions(Writer writer, const vector<Function>& functions, int* petroValue)
{
	writer.WriteInt((unsigned int)functions.size());
	for (size_t i = 0; i < functions.size(); i++)
	{
		WriteFunction(writer, functions[i], petroValue);
	}
}

static void WriteInstructions(Writer& writer, const vector<Instruction>& instructions)
{
	writer.WriteInt((unsigned int)instructions.size());
	for (size_t i = 0; i < instructions.size(); i++)
	{
		writer.WriteInt(instructions[i]);
	}
}

static void WriteFunction( Writer& writer, const Function& function, int* petroValue)
{
	writer.WriteString(function.name, true);
	writer.WriteInt(function.lineDefined);
	if (petroValue != NULL)
	{
		writer.WriteInt((*petroValue)++);
	}
	writer.WriteByte(function.nUpvalues);
	writer.WriteByte(function.nParameters);
	writer.WriteByte(function.isVararg);
	writer.WriteByte(function.maxStackSize);
    WriteLines       (writer, function.lines);
    WriteLocals      (writer, function.locals);
	WriteUpvalues    (writer, function.upvalues);
	WriteConstants   (writer, function.constants);
	WriteFunctions   (writer, function.functions, petroValue);
	WriteInstructions(writer, function.instructions);
}

void WriteFile(ostream& output, const File& file, bool isLup)
{
    int petroValue = 1;
    Writer writer = WriteHeader(output, isLup);
    WriteFunction(writer, file.function, isLup ? &petroValue : NULL);
}

}
}