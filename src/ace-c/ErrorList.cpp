#include <ace-c/ErrorList.hpp>

ErrorList::ErrorList()
{
}

ErrorList::ErrorList(const ErrorList &other)
    : m_errors(other.m_errors)
{
}

bool ErrorList::HasFatalErrors() const
{
    if (!m_errors.empty()) {
        for (const CompilerError &error : m_errors) {
            if (error.GetLevel() == Level_fatal) {
                return true;
            }
        }
    }
    
    return false;
}
