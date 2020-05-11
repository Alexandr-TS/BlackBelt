#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
	// check
	os << "classInstance::print?";
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
	const Class* cur_class_ptr = cls;
	while(cur_class_ptr) {
		if (cur_class_ptr->GetMethod(method)) {
			return true;
		}
		cur_class_ptr = cur_class_ptr->GetParent();
	}
}

const Closure& ClassInstance::Fields() const {
	return fields;
}

Closure& ClassInstance::Fields() {
	return fields;
}

ClassInstance::ClassInstance(const Class& cls) 
	: cls(&cls)
{
}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
	const auto method_ptr = cls->GetMethod(method);
	if (method_ptr == nullptr || method_ptr->formal_params.size() != actual_args.size()) {
		return ObjectHolder::None();
	}

	Closure args;
	for (size_t i = 0; i < method_ptr->formal_params.size(); ++i) {
		args[method_ptr->formal_params[i]] = actual_args[i];
	}
	return method_ptr->body->Execute(args);
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent) 
	: name(name)
	, methods(methods)
	, parent(parent)
{
}

const Method* Class::GetMethod(const std::string& name) const {
	const Class* cur_ptr = this;
	while (cur_ptr) {
		for (const auto& method : cur_ptr->methods) {
			if (method.name == name) {
				return &method;
			}
		}
		cur_ptr = cur_ptr->parent;
	}
	return nullptr;
}

void Class::Print(ostream& os) {
	// check
	os << "Class::Print?";
}

const std::string& Class::GetName() const {
	return name;
}

const Runtime::Class* Class::GetParent() const {
	return paretn;
}

void Bool::Print(std::ostream& os) {
	if (GetValue()) {
		os << "True";
	}
	else {
		os << "False";
	}
}

} /* namespace Runtime */
