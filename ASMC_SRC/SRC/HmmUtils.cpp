//    This file is part of ASMC, developed by Pier Francesco Palamara.
//
//    ASMC is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ASMC is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ASMC.  If not, see <https://www.gnu.org/licenses/>.

#include "HmmUtils.hpp"

#include <algorithm>
#include <cassert>

namespace asmc
{

std::vector<bool> subsetXorVec(const std::vector<bool>& v1, const std::vector<bool>& v2, const unsigned long from,
                               const unsigned long to) noexcept
{
  const auto min_to = std::min<unsigned long>(v1.size(), to);

  assert(v1.size() == v2.size());
  assert(from < min_to);

  std::vector<bool> ret(min_to - from);

  for (unsigned long i = from; i < min_to; ++i) {
    ret[i - from] = v1[i] ^ v2[i];
  }

  return ret;
}

std::vector<bool> subsetAndVec(const std::vector<bool>& v1, const std::vector<bool>& v2, const unsigned long from,
                               const unsigned long to) noexcept
{
  const auto min_to = std::min<unsigned long>(v1.size(), to);

  assert(v1.size() == v2.size());
  assert(from < min_to);

  std::vector<bool> ret(min_to - from);

  for (unsigned i = from; i < min_to; i++) {
    ret[i - from] = v1[i] & v2[i];
  }

  return ret;
}

} // namespace asmc
