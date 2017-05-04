#ifndef MESHLOADER2D_H
#define MESHLOADER2D_H

#include <glm.hpp>
#include <vector>

bool meshLoader2D(const char * path, std::vector<glm::vec3> & out_vertices);

#endif