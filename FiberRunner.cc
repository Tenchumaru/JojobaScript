#include "pch.h"
#include <Windows.h>
#include "FiberRunner.h"

namespace {
	constexpr size_t stackSize = 0x1'0000;
}

FiberRunner FiberRunner::instance;

FiberRunner::~FiberRunner() {
	ConvertFiberToThread();
}

void FiberRunner::Await(void* handle) {
	nextAwaits.fibers.push_back(GetCurrentFiber());
	nextAwaits.handles.push_back(handle);
	SwitchToFiber(launchingFiber);
}

void* FiberRunner::GetCurrentFiber() {
	return ::GetCurrentFiber();
}

void* FiberRunner::Launch(std::function<void()>&& newFn) {
	void* fiber;
	if (availableFibers.empty()) {
		fiber = CreateFiber(stackSize, &FiberRunner::RunFiber, this);
		if (!fiber) {
			std::cerr << "CreateFiber failed: " << errno << std::endl;
			return nullptr;
		}
	} else {
		fiber = availableFibers.back();
		availableFibers.pop_back();
	}
	fn = std::move(newFn);
	auto const previousFiber = launchingFiber;
	launchingFiber = GetCurrentFiber();
	SwitchToFiber(fiber);
	launchingFiber = previousFiber;
	return fiber;
}

int FiberRunner::Run() {
	for (;;) {
		if (nextAwaits.fibers.empty()) {
			return 0;
		}
		currentAwaits.fibers.clear();
		currentAwaits.handles.clear();
		std::swap(currentAwaits, nextAwaits);
		DWORD const waitResult = WaitForMultipleObjects(static_cast<DWORD>(currentAwaits.handles.size()), currentAwaits.handles.data(), FALSE, INFINITE);
		if (waitResult < currentAwaits.handles.size() + WAIT_OBJECT_0) {
			DWORD const i = waitResult - WAIT_OBJECT_0;
			void* const currentFiber = currentAwaits.fibers[i];
			SwitchToFiber(currentFiber);
		} else {
			DWORD errorCode = GetLastError();
			std::cerr << "[FiberRunner::Run.WaitForMultipleObjects] error " << errorCode << std::endl;
			return errorCode;
		}
	}
}

void FiberRunner::SwitchToFiber(void* fiber) {
	::SwitchToFiber(fiber);
}

FiberRunner& FiberRunner::get_Instance() {
	return instance;
}

FiberRunner::FiberRunner() {
	mainFiber = ConvertThreadToFiber(nullptr);
	if (!mainFiber) {
		throw std::runtime_error("FiberRunner::FiberRunner.ConvertThreadToFiber");
	}
	launchingFiber = mainFiber;
}

void __stdcall FiberRunner::RunFiber(void* parameter) {
	auto* const p = reinterpret_cast<FiberRunner*>(parameter);
	for (;;) {
		void* launchingFiber = p->launchingFiber;
		try {
#pragma prefast(suppress: 26800, "FiberRunner::Launch will set the function member when this loops")
			std::function<void()> const fn = std::move(p->fn);
			fn();
		} catch (std::exception const& ex) {
			std::cerr << "FiberRunner::RunFiber.InternalHandle:  " << ex.what() << std::endl;
		}
		p->availableFibers.push_back(GetCurrentFiber());
		SwitchToFiber(launchingFiber);
	}
}
