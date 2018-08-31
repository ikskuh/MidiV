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
#include <json.hpp>

namespace MidiV
{
    void Initialize(nlohmann::json const & config);

	void LoadVisualization(int slot, std::string const & fileName);

    void Resize(int w, int h);

    void Render();
};

#endif // MVISUALIZATIONCONTAINER_HPP
