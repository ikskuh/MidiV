#include "midiv.hpp"

#include <GL/gl3w.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "die.h"
#include "debug.hpp"
#include "hal.hpp"
#include "utils.hpp"

#include <json.hpp>

[[noreturn]] void _die(char const * context, char const * msg)
{
    fprintf(stderr, "%s: %s\n", context, msg);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

	std::string configFile;
	if(argc > 1)
	{
		configFile = argv[1];
	}
	else
	{
#ifdef MIDIV_DEBUG
		configFile = HAL::GetWorkingDirectory() + "/midiv.cfg";
#else
		configFile = HAL::GetDirectoryOf(argv[0]) + "/midiv.cfg";
#endif
	}

	// load config
	auto const data = Utils::LoadFile(configFile);
	if(!data)
	{
		Log() << "Config file " << configFile << " not found!";
		return EXIT_FAILURE;
	}

	auto const config = nlohmann::json::parse(data->begin(), data->end());

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        die(SDL_GetError());
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    auto * const window = SDL_CreateWindow(
        "Midi-V",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        Utils::get(config, "width", 1280),
		Utils::get(config, "height", 720),
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | (Utils::get(config, "fullscreen", false) ? SDL_WINDOW_FULLSCREEN : 0));
    if(window == nullptr)
        die(SDL_GetError());

    auto const context = SDL_GL_CreateContext(window);
    if(context == nullptr)
        die(SDL_GetError());

    if(SDL_GL_MakeCurrent(window, context) < 0)
        die(SDL_GetError());

    switch(gl3wInit())
    {
        case GL3W_OK:
            Log() << glGetString(GL_VERSION) << " ready.";
            break;

        case GL3W_ERROR_LIBRARY_OPEN:
            Log() << "Failed to initialize gl3w: Could not open OpenGL library!";
            return EXIT_FAILURE;

        case GL3W_ERROR_INIT:
            Log() << "Failed to initialize gl3w: Could not initialize OpenGL!";
            return EXIT_FAILURE;

        case GL3W_ERROR_OPENGL_VERSION:
            Log() << "Failed to initialize gl3w: OpenGL version too low!";
            return EXIT_FAILURE;

        default:
            Log() << "Failed to initialize gl3w: Unknown error!";
            return EXIT_FAILURE;
    }

    {
        std::vector<std::pair<char const *, bool>> requiredExtensions =
        {
            std::make_pair("GL_ARB_direct_state_access", false),
        };

        int count;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for(unsigned int i = 0; i < static_cast<unsigned int>(count); i++)
        {
            auto * str = glGetStringi(GL_EXTENSIONS, i);
            for(auto & ext : requiredExtensions)
                ext.second |=  (0 == strcmp(reinterpret_cast<char const *>(str), ext.first));
        }

        for(auto const & ext  : requiredExtensions)
        {
            if(!ext.second)
                Log() << "Extension " << ext.first << " was not found. Program may not function properly!";
        }
    }

    if(glCreateVertexArrays == nullptr)
    {
        Log() << "Monkey patching glCreateVertexArrays with glGenVertexArrays...";
        glCreateVertexArrays = glGenVertexArrays;
    }


    Log() << "Initializing...";
    MidiV::Initialize(config);

	if(Utils::HasError())
		return EXIT_FAILURE;

    Log() << "Initialized!";

	Log() << "Loading visualizations...";

	int index = 0;

	bool good = true;
	for(auto const & vis : config["visualizations"])
	{
		auto file = vis.get<std::string>();

		Utils::ResetError();

		Log() << "Load visualization " << file << "...";
		MidiV::LoadVisualization(index++, file);

		good &= !Utils::HasError();
		if(Utils::HasError())
			Log() << "Failure.";
	}

	if(!good)
		return EXIT_FAILURE;

	Log() << "Visualizations ready!";

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    MidiV::Resize(w, h);

	if(Utils::HasError())
		return EXIT_FAILURE;

    bool done = false;
    do
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                done = true;
            if((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE))
                done = true;
			if((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_SPACE))
                MidiV::TakeScreenshot();
            if((e.type == SDL_WINDOWEVENT) && (e.window.event == SDL_WINDOWEVENT_RESIZED))
            {
                SDL_GetWindowSize(window, &w, &h);
                MidiV::Resize(w, h);
            }
        }

        MidiV::Render();

        SDL_GL_SwapWindow(window);

        SDL_Delay(16);

    } while(!done);

    return EXIT_SUCCESS;
}

static volatile bool error_flag = false;

bool Utils::HasError() { return error_flag; }
void Utils::FlagError() {
	error_flag = true;
}
void Utils::ResetError() { error_flag = false; }
