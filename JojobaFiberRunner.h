#pragma once

#include <FiberRunner.h>

class JojobaFiberRunner : public Adrezdi::FiberRunner {
public:
	static std::shared_ptr<JojobaFiberRunner> Get();
	JojobaFiberRunner(JojobaFiberRunner const&) = delete;
	JojobaFiberRunner(JojobaFiberRunner&&) noexcept = default;
	JojobaFiberRunner& operator=(JojobaFiberRunner const&) = delete;
	JojobaFiberRunner& operator=(JojobaFiberRunner&&) noexcept = default;
	~JojobaFiberRunner() = default;
	void Await(void* handle);

private:
	std::vector<void*> handles;

	JojobaFiberRunner() = default;
	void InternalWait(std::vector<void*>& candidateFibers, std::vector<void*>& runnableFibers) override;
};
