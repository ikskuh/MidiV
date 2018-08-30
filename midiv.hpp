#ifndef MVISUALIZATIONCONTAINER_HPP
#define MVISUALIZATIONCONTAINER_HPP

#include <vector>
#include <GL/gl3w.h>
#include <chrono>

#ifdef MIDIV_LINUX
#include <drumstick.h>
#undef marker
#endif

#include <mutex>
#include "mvisualization.hpp"
#include <glm/glm.hpp>

namespace MidiV
{
    void Initialize();

    void Resize(int w, int h);

    void Render();
};

#endif // MVISUALIZATIONCONTAINER_HPP
