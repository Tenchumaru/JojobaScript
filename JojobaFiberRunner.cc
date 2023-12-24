#include "pch.h"
#include <Windows.h>
#include "JojobaFiberRunner.h"

std::shared_ptr<JojobaFiberRunner> JojobaFiberRunner::Get() {
	static std::shared_ptr<JojobaFiberRunner> instance = std::shared_ptr<JojobaFiberRunner>(new JojobaFiberRunner);
	return instance;
}

void JojobaFiberRunner::Await(void* handle) {
	handles.push_back(handle);
	Adrezdi::FiberRunner::Await();
}

void JojobaFiberRunner::InternalWait(std::vector<void*>& candidateFibers, std::vector<void*>& runnableFibers) {
	DWORD const waitResult = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), FALSE, INFINITE);
	if (waitResult < handles.size() + WAIT_OBJECT_0) {
		DWORD const j = waitResult - WAIT_OBJECT_0;
		std::vector<HANDLE> nextHandles;
		std::vector<void*> nonrunnableFibers;
		for (size_t i = 0; i < handles.size(); ++i) {
			if (i == j) {
				runnableFibers.push_back(candidateFibers[i]);
			} else {
				nextHandles.push_back(handles[i]);
				nonrunnableFibers.push_back(candidateFibers[i]);
			}
		}
		std::swap(nextHandles, handles);
		std::swap(nonrunnableFibers, candidateFibers);
	} else {
		DWORD errorCode = GetLastError();
		std::cerr << "[JojobaFiberRunner::Run.WaitForMultipleObjects] error " << errorCode << std::endl;
		throw std::runtime_error("[JojobaFiberRunner::Run.WaitForMultipleObjects] error");
	}
}
