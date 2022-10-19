#include "core/std/containers/cstring.h"
#include <cstring>

namespace tundra::core {

bool CString::operator==(const CString& rhs) noexcept
{
    const usize lhs_size = this->size();
    const usize rhs_size = rhs.size();
    if (lhs_size != rhs_size) {
        return false;
    }
    return std::strncmp(begin(), rhs.begin(), lhs_size) == 0;
}

bool CString::operator!=(const CString& rhs) noexcept
{
    return !(*this == rhs);
}

} // namespace tundra::core
