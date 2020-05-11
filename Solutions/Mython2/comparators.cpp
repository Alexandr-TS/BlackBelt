#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <sstream>

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
	if (lhs.TryAs<Number>()) {
		auto l_val = lhs.TryAs<Number>()->GetValue();
		if (!rhs.TryAs<Number>()) {
			throw runtime_error("Attempt to Equal number and non-number");
		}
		return l_val == rhs.TryAs<Number>()->GetValue();
	}
	else if (lhs.TryAs<String>()) {
		auto l_val = lhs.TryAs<String>()->GetValue();
		if (!rhs.TryAs<String>()) {
			throw runtime_error("Attempt to Equal string and non-string");
		}
		return l_val == rhs.TryAs<String>()->GetValue();
	}
	throw runtime_error("Attemp to Equal non-comparable objects");
}

bool Less(ObjectHolder lhs, ObjectHolder rhs) {
	if (lhs.TryAs<Number>()) {
		auto l_val = lhs.TryAs<Number>()->GetValue();
		if (!rhs.TryAs<Number>()) {
			throw runtime_error("Attempt to Less number and non-number");
		}
		return l_val < rhs.TryAs<Number>()->GetValue();
	}
	else if (lhs.TryAs<String>()) {
		auto l_val = lhs.TryAs<String>()->GetValue();
		if (!rhs.TryAs<String>()) {
			throw runtime_error("Attempt to Less string and non-string");
		}
		return l_val < rhs.TryAs<String>()->GetValue();
	}
	throw runtime_error("Attemp to Less non-comparable objects");
}

} /* namespace Runtime */
