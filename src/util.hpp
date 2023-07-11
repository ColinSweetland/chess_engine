#ifndef UTIL_INCL
#define UTIL_INCL

#include "types.hpp"

#include <locale>
#include <sstream>

namespace util
{
// print int with a "pretty" locale without changing behavior everywhere
template <typename T> str pretty_int(T to_pretty)
{
    // CREDIT: https://stackoverflow.com/a/17530535
    class pretty_num_punct : public std::numpunct<char>
    {
      protected:
        virtual char        do_thousands_sep() const { return ','; }
        virtual std::string do_grouping() const { return "\03"; }
    };

    std::ostringstream oss{};

    // stackoverflow says locales are ref counted so 'new' should be okay here
    oss.imbue(std::locale(std::locale::classic(), new pretty_num_punct));

    oss << to_pretty;

    return oss.str();
}

} // namespace util
#endif // UTIL INCL