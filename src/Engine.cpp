#include "Engine.h"
#include "Utilities/ECS/Transform_C.h"
#include "LinearMath/btScalar.h"
#include <direct.h>

// OpenGL Dependent Systems //
#include "GL/glad/glad.h"
#include "GLFW/glfw3.h"

// Importers Used //
#include "Utilities/IO/Image_IO.h"
#include "Utilities/IO/Mesh_IO.h"

// Starting States Used
#include "States/MainMenuState.h"


constexpr int DESIRED_OGL_VER_MAJOR = 4;
constexpr int DESIRED_OGL_VER_MINOR = 5;

/***************************
----START GLFW CALLBACKS----
***************************/
static void GLFW_Callback_WindowResize(GLFWwindow * window, int width, int height)
{
	auto & preferences = ((Engine*)glfwGetWindowUserPointer(window))->getPreferenceState();
	preferences.setValue(PreferenceState::C_WINDOW_WIDTH, width);
	preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, height);
}
static void GLFW_Callback_Char(GLFWwindow* window, unsigned int character)
{
	((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyChar(character);
}
static void GLFW_Callback_Key(GLFWwindow* window, int a, int b, int c, int d)
{
	((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyKey(a,b,c,d);
}
/***************************
-----END GLFW CALLBACKS-----
***************************/

Rendering_Context::~Rendering_Context()
{
	glfwDestroyWindow(window);
}

Rendering_Context::Rendering_Context(Engine * engine)
{
	// Begin Initialization
	if (!glfwInit()) {
		engine->getManager_Messages().error("GLFW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Create main window
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
	glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	window = glfwCreateWindow(1, 1, "", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowIcon(window, 0, NULL);
	glfwSetCursorPos(window, 0, 0);

	// Initialize GLAD
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		engine->getManager_Messages().error("GLAD unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}
}

Auxilliary_Context::Auxilliary_Context(const Rendering_Context & otherContext)
{
	// Create an invisible window for multi-threaded GL operations
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
	glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	window = glfwCreateWindow(1, 1, "", NULL, otherContext.window);
}

Engine::~Engine()
{
	// Update indicator
	m_aliveIndicator = false;
	m_messageManager.statement("Shutting down...");
	Image_IO::Deinitialize();
	glfwTerminate();
}

Engine::Engine() :
	// Initialize engine-dependent members first
	m_soundManager(),
	m_assetManager(),
	m_inputBindings(this),
	m_preferenceState(this),
	m_renderingContext(this),
	m_materialManager(this)
{
	// Output engine boiler-plate
	m_messageManager.statement("**************************************************");
	m_messageManager.statement("Engine Version " + std::string(ENGINE_VERSION));
	m_messageManager.statement("");
	m_messageManager.statement("Library Info:");
	m_messageManager.statement("ASSIMP       " + Mesh_IO::Get_Version());
	m_messageManager.statement("Bullet       " + std::to_string(BT_BULLET_VERSION));
	m_messageManager.statement("FreeImage    " + Image_IO::Get_Version());
	m_messageManager.statement("GLAD         " + std::to_string(GLVersion.major) + "." + std::to_string(GLVersion.minor));
	m_messageManager.statement("GLFW         " + std::string(glfwGetVersionString(), 5));
	m_messageManager.statement("GLM          " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
	m_messageManager.statement("SoLoud       " + std::to_string(m_soundManager.GetVersion()));
	m_messageManager.statement("");
	m_messageManager.statement("Graphics Info:");
	m_messageManager.statement(std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
	m_messageManager.statement("OpenGL " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
	m_messageManager.statement("GLSL " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	m_messageManager.statement("**************************************************");

	Image_IO::Initialize();

	// Preference Values
	m_refreshRate = float(glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_windowSize.x);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_windowSize.y);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_refreshRate);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_FULLSCREEN, m_useFullscreen);
	m_preferenceState.getOrSetValue(PreferenceState::C_VSYNC, m_vsync);

	// Preference Callbacks
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_windowSize.x = int(f);
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_windowSize.y = int(f);
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_REFRESH_RATE, m_aliveIndicator, [&](const float &f) {
		m_refreshRate = f;
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_FULLSCREEN, m_aliveIndicator, [&](const float &f) {
		m_useFullscreen = f;
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_VSYNC, m_aliveIndicator, [&](const float &f) {
		m_vsync = f;
		glfwSwapInterval((int)f);
	});

	// Configure Rendering Context
	configureWindow();
	glfwSetInputMode(m_renderingContext.window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwSetWindowUserPointer(m_renderingContext.window, this);
	glfwSetWindowSizeCallback(m_renderingContext.window, GLFW_Callback_WindowResize);
	glfwSetCharCallback(m_renderingContext.window, GLFW_Callback_Char);
	glfwSetKeyCallback(m_renderingContext.window, GLFW_Callback_Key);
	glfwMakeContextCurrent(m_renderingContext.window);
	glfwSwapInterval((int)m_vsync);

	// Initialize Members
	registerECSConstructor("Transform_Component", new Transform_Constructor());
	m_inputBindings.loadFile("binds");
	m_modelManager.initialize();
	m_moduleGraphics.initialize(this);
	m_modulePProcess.initialize(this);
	m_moduleUI.initialize(this);
	m_modulePhysics.initialize(this);
	m_moduleWorld.initialize(this);

	const unsigned int maxThreads = std::max(1u, std::thread::hardware_concurrency());
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		std::thread workerThread(&Engine::tickThreaded, this, std::move(exitObject), std::move(Auxilliary_Context(m_renderingContext)));
		workerThread.detach();
		m_threads.push_back(std::move(std::make_pair(std::move(workerThread), std::move(exitSignal))));
	}

	// Initialize starting state LAST
	m_engineState = new MainMenuState(this);
}

void Engine::tick()
{
	const float thisTime = (float)glfwGetTime();
	const float deltaTime = thisTime - m_lastTime;
	m_lastTime = thisTime;

	// Update Managers
	m_assetManager.notifyObservers();
	m_modelManager.update();

	// Update persistent binding state
	if (const auto &bindings = m_inputBindings.getBindings())
		if (bindings->existsYet())
			for each (const auto &pair in bindings.get()->m_configuration) {
				// If Key is pressed, set state to 1, otherwise set to 0
				m_actionState[pair.first] = glfwGetKey(m_renderingContext.window, (int)pair.second) ? 1.0f : 0.0f;
			}
	// Updated hard-coded bindings
	double mouseX, mouseY;
	glfwGetCursorPos(m_renderingContext.window, &mouseX, &mouseY);
	m_actionState[ActionState::MOUSE_X] = (float)mouseX;
	m_actionState[ActionState::MOUSE_Y] = (float)mouseY;
	m_actionState[ActionState::MOUSE_L] = (float)glfwGetMouseButton(m_renderingContext.window, GLFW_MOUSE_BUTTON_LEFT);
	m_actionState[ActionState::MOUSE_R] = (float)glfwGetMouseButton(m_renderingContext.window, GLFW_MOUSE_BUTTON_RIGHT);

	// Update Engine State
	if (auto * state = m_engineState->handleInput(m_actionState)) {
		if (state != m_engineState)
			delete m_engineState;
		m_engineState = state;
	}
	m_engineState->handleTick(deltaTime);

	// End Frame
	glfwSwapBuffers(m_renderingContext.window);
	glfwPollEvents();
}

void Engine::tickThreaded(std::future<void> exitObject, const Auxilliary_Context && context)
{
	glfwMakeContextCurrent(context.window);

	// Check if thread should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
		m_assetManager.beginWorkOrder();
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_renderingContext.window);
}

void Engine::shutDown()
{
	glfwSetWindowShouldClose(m_renderingContext.window, GLFW_TRUE);
}

void Engine::registerECSConstructor(const char * name, BaseECSComponentConstructor * constructor)
{
	m_ecs.registerConstructor(name, constructor);
}

GLFWwindow * Engine::getRenderingContext() const
{
	return m_renderingContext.window;
}

std::vector<glm::ivec3> Engine::getResolutions() const
{
	int count(0);
	const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
	std::vector<glm::ivec3> resolutions(count);
	for (int x = 0; x < count; ++x)
		resolutions[x] = { modes[x].width, modes[x].height, modes[x].refreshRate };
	return resolutions;
}

std::string Engine::Get_Current_Dir()
{
	// Technique to return the running directory of the application
	char cCurrentPath[FILENAME_MAX];
	if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
		cCurrentPath[sizeof(cCurrentPath) - 1ull] = char('\0');
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string & name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer;
	return (stat((Engine::Get_Current_Dir() + name).c_str(), &buffer) == 0);
}

void Engine::configureWindow()
{
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_renderingContext.window, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_renderingContext.window, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowMonitor(
		m_renderingContext.window,
		m_useFullscreen ? glfwGetPrimaryMonitor() : NULL,
		0, 0,
		m_windowSize.x, m_windowSize.y,
		(int)m_refreshRate
	);
}
