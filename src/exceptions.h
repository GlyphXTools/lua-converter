#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

class IOException : public std::runtime_error
{
public:
	IOException(const std::string& message) : std::runtime_error(message) {}
};

class BadCodeException : public std::runtime_error
{
public:
	BadCodeException(const std::string& message) : std::runtime_error(message) {}
};

class FileNotFoundException : public IOException
{
public:
	FileNotFoundException() : IOException("Unable to find file\n") {}
};

class BadFileException : public IOException
{
public:
	BadFileException() : IOException("Bad or corrupted file\n") {}
};

#endif