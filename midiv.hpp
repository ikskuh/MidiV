#ifndef MVISUALIZATIONCONTAINER_HPP
#define MVISUALIZATIONCONTAINER_HPP

#include <vector>
#include <GL/gl3w.h>
#include <chrono>

#ifdef MIDIV_LINUX

#endif

#include <mutex>
#include "mvisualization.hpp"
#include <glm/glm.hpp>

namespace MidiV
{
    void Initialize();

	void LoadVisualization(int slot, std::string const & fileName);

    void Resize(int w, int h);

    void Render();
};

#endif // MVISUALIZATIONCONTAINER_HPP
