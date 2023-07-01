#include "util.hpp"
#include "types.hpp"

#include <locale>
#include <sstream>

// CREDIT: https://stackoverflow.com/a/17530535
class pretty_num_punct : public std::numpunct<char>
{
  protected:
    virtual char        do_thousands_sep() const { return ','; }
    virtual std::string do_grouping() const { return "\03"; }
};

// print int with a "pretty" locale without changing behavior everywhere
str pretty_int(std::intmax_t to_pretty)
{
    std::ostringstream oss{};

    // stackoverflow says locales are ref counted so 'new' should be okay here
    oss.imbue(std::locale(std::locale::classic(), new pretty_num_punct));

    oss << to_pretty;

    return oss.str();
}