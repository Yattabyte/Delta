#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this engine module. */
	inline virtual ~Engine_Module() noexcept = default;
	/** Construct an engine module.
	@param	engine	reference to the engine to use. */
	explicit Engine_Module(Engine& engine) noexcept;


	// Public Interface Declarations
	/** Initialize the module. */
	virtual void initialize() noexcept;
	/** De-initialize the module. */
	virtual void deinitialize() noexcept;


protected:
	Engine& m_engine;
};

#endif // ENGINE_MODULE_H