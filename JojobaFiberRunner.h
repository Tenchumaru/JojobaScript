#pragma once

class FiberRunner {
public:
	~FiberRunner();
	void Await(void* handle);
	static void* GetCurrentFiber();
	void* Launch(std::function<void()>&& fn);
	int Run();
	static void SwitchToFiber(void* fiber);
	static FiberRunner& get_Instance();

private:
	struct {
		std::vector<void*> fibers;
		std::vector<void*> handles;
	} currentAwaits, nextAwaits;
	std::vector<void*> availableFibers;
	std::function<void()> fn;
	void* mainFiber;
	void* launchingFiber;
	static FiberRunner instance;

	FiberRunner();

	static void __stdcall RunFiber(void* parameter);
};
