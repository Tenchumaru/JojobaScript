#pragma once

class FiberRunner {
public:
	~FiberRunner();
	void AddWaitableHandle(void* handle, std::function<void(void*)> fn);
	void Await(void* handle);
	int Launch(std::function<void()>&& fn);
	int Run();
	static FiberRunner& get_Instance();
	__declspec(property(get = get_Instance)) FiberRunner& Instance;

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
