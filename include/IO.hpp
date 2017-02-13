#ifndef IO_HPP_INCLUDED
#define IO_HPP_INCLUDED


#include "glm.hpp"

#include <string>
#include <vector>


namespace IO
{
    /* @return true if success */
    bool readObj(std::string const& filename,
                 std::vector<glm::vec3>& vertices,
                 std::vector<glm::ivec3>& triangles);

    /* @return true if success */
    bool readObj(std::string const& filename,
                 std::vector<glm::vec3>& vertices,
                 std::vector<glm::vec3>& normal,
                 std::vector<glm::ivec3>& triangles);
}

#endif // IO_HPP_INCLUDED
