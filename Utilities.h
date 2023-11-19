#pragma once

#include <algorithm>
#include <vector>

template<typename T> void Delete(T*& p) {
	delete p;
	p = nullptr;
}

template<typename T> void DeleteElements(std::vector<T*>& collection) {
	std::for_each(collection.begin(), collection.end(), [](T* p) { delete p; });
	collection.clear();
}
