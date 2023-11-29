#pragma once

class Context;

class Type {
public:
	Type(std::string&& type) : type(std::move(type)) {}
	Type(Type&&) = default;
	~Type() = default;

private:
	std::string type;
};
