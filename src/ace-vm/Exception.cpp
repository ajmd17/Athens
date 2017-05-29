#include <ace-vm/Exception.hpp>

namespace ace {
namespace vm {

Exception::Exception(const utf::Utf8String &str)
    : m_str(str)
{
}

Exception::Exception(const Exception &other)
    : m_str(other.m_str)
{
}

Exception Exception::InvalidComparisonException(const char *left_type_str, const char *right_type_str)
{
    char buffer[256];
    std::snprintf(
        buffer,
        255,
        "cannot compare '%s' with '%s'",
        left_type_str,
        right_type_str
    );
    return Exception(utf::Utf8String(buffer));
}

Exception Exception::InvalidArgsException(int expected, int received, bool variadic)
{
    char buffer[256];
    if (variadic) {
        std::sprintf(buffer, "invalid arguments: expected at least %d, received %d", expected, received);
    } else {
        std::sprintf(buffer, "invalid arguments: expected %d, received %d", expected, received);
    }
    return Exception(utf::Utf8String(buffer));
}

Exception Exception::InvalidArgsException(const char *expected_str, int received)
{
    char buffer[256];
    std::sprintf(buffer, "invalid arguments: expected %s, received %d", expected_str, received);
    return Exception(utf::Utf8String(buffer));
}

Exception Exception::NullReferenceException()
{
    return Exception(utf::Utf8String("null reference exception"));
}

Exception Exception::DivisionByZeroException()
{
    return Exception(utf::Utf8String("attempted to divide by zero"));
}

Exception Exception::OutOfBoundsException()
{
    return Exception(utf::Utf8String("index out of bounds of the array"));
}

Exception Exception::MemberNotFoundException()
{
    return Exception(utf::Utf8String("invalid access to member of object"));
}

Exception Exception::FileOpenException(const char *file_name)
{
    return Exception(utf::Utf8String("could not open file: `") + file_name + "`");
}

Exception Exception::UnopenedFileWriteException()
{
    return Exception(utf::Utf8String("attempted to write to an unopened file"));
}

Exception Exception::UnopenedFileReadException()
{
    return Exception(utf::Utf8String("attempted to read from an unopened file"));
}

Exception Exception::UnopenedFileCloseException()
{
    return Exception(utf::Utf8String("attempted to close an unopened file"));
}

Exception Exception::LibraryLoadException(const char *lib_name)
{
    return Exception(utf::Utf8String("could not load library: `") + lib_name + "`");
}

Exception Exception::LibraryFunctionLoadException(const char *func_name)
{
    return Exception(utf::Utf8String("could not load function: `") + func_name + "`");
}

} // namespace vm
} // namespace ace