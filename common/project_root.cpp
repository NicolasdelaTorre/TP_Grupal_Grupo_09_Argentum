#include "project_root.h"

#include <unistd.h>

#ifndef PROJECT_ROOT
// Respaldo por si se compila sin la definición de CMake (no debería pasar).
#define PROJECT_ROOT "."
#endif

namespace project_root {

std::string locate() { return PROJECT_ROOT; }

bool chdirToRoot() { return chdir(PROJECT_ROOT) == 0; }

}  // namespace project_root
