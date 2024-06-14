#include "expression_parser/token.h"

#include <algorithm>

using namespace ExpressionParserV2;

#define i64 std::int64_t
#define v std::get

std::optional<Variant> StoryVariableInfo::get_variable_value(const std::string& variable) const {
	const std::string* var = &variable;
	while (true) {
		if (auto redirect = redirects.find(*var); redirect != redirects.end()) {
			var = &redirect->second;
		} else {
			break;
		}
	}

	if (auto constant_value = constants.find(*var); constant_value != constants.end()) {
		return constant_value->second;
	}

	if (auto variable_value = variables.find(*var); variable_value != variables.end()) {
		return variable_value->second;
	}

	return std::nullopt;
}

void StoryVariableInfo::set_variable_value(const std::string& variable, const Variant& value) {
	const std::string* var = &variable;
	while (true) {
		if (auto redirect = redirects.find(*var); redirect != redirects.end()) {
			var = &redirect->second;
		} else {
			break;
		}
	}

	if (auto variable_value = variables.find(*var); variable_value != variables.end()) {
		if (variable_value->second != value) {
			variable_value->second = value;
			flag_variable_changed(variable);
		}
	} else {
		variables.emplace(variable, value);
	}
}

void StoryVariableInfo::observe_variable(const std::string& variable_name, VariableObserverFunc callback) {
	if (auto entry = observers.find(variable_name); entry != observers.end()) {
		entry->second.push_back(callback);
	} else {
		observers[variable_name] = {callback};
	}
}

void StoryVariableInfo::unobserve_variable(const std::string& variable_name) {
	if (auto entry = observers.find(variable_name); entry != observers.end()) {
		entry->second.clear();
	}
}

void StoryVariableInfo::unobserve_variable(VariableObserverFunc observer) {
	for (auto& entry : observers) {
		std::erase_if(entry.second,
			[observer](VariableObserverFunc this_observer) { return this_observer == observer; }
		);
	}
}

void StoryVariableInfo::unobserve_variable(const std::string& variable_name, VariableObserverFunc observer) {
	if (auto entry = observers.find(variable_name); entry != observers.end()) {
		std::erase_if(entry->second,
			[observer](VariableObserverFunc this_observer) { return this_observer == observer; }
		);
	}
}

void StoryVariableInfo::execute_variable_observers(const std::string& variable, const Variant& new_value) {
	if (auto entry = observers.find(variable); entry != observers.end()) {
		for (VariableObserverFunc& observer : entry->second) {
			(observer)(variable, new_value);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Token::fetch_variable_value(const StoryVariableInfo& story_vars) {
	if (type == TokenType::Variable) {
		std::optional<Variant> var_value = story_vars.get_variable_value(variable_name);
		if (var_value.has_value()) {
			value = *var_value;
		}
	}
}

void Token::store_variable_value(StoryVariableInfo& story_vars) {
	if (type == TokenType::Variable) {
		story_vars.set_variable_value(variable_name, value);
	}
}

Variant Token::increment(bool post, StoryVariableInfo& story_vars) {
	Variant result;
	if (post) {
		result = value++;
	} else {
		result = ++value;
	}

	store_variable_value(story_vars);
	return result;
}

Variant Token::decrement(bool post, StoryVariableInfo& story_vars) {
	Variant result;
	if (post) {
		result = value--;
	} else {
		result = --value;
	}

	store_variable_value(story_vars);
	return result;
}

void Token::assign_variable(const Token& other, StoryVariableInfo& story_vars) {
	if (type == TokenType::Variable) {
		story_vars.set_variable_value(variable_name, other.value);
		fetch_variable_value(story_vars);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Variant::Variant() : value(0i64), has_value(false) {}
Variant::Variant(bool val) : value(static_cast<i64>(val)), has_value(true) {}
Variant::Variant(i64 val) : value(val), has_value(true) {}
Variant::Variant(double val) : value(val), has_value(true) {}
Variant::Variant(const std::string& val) : value(val), has_value(true) {}
//Variant::Variant(const VariantValue& val) : value(val), has_value(true) {}

Variant::Variant(const Variant& from) : value(from.value) {}

Variant& Variant::operator=(const Variant& from) {
	if (this != &from) {
		value = from.value;
	}

	return *this;
}

Variant Variant::operator+(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) + v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) + v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) + static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) + v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_String: {
			if (rhs.value.index() == Variant_String) {
				return v<std::string>(value) + v<std::string>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator-(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) - v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) - v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) - static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) - v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator*(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) * v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) * v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) * static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) * v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator/(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) / v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) / v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) / static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) / v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator%(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) % v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return std::fmod(static_cast<double>(v<i64>(value)), v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return std::fmod(v<double>(value), static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return std::fmod(v<double>(value), v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator==(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) == v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) == v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) == static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) == v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator!=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) != v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) != v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) != static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) != v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator<(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) < v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) < v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) < static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) < v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator>(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) > v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) > v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) > static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) > v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator<=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) <= v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) <= v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) <= static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) <= v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator>=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<i64>(value) >= v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<i64>(static_cast<double>(v<i64>(value)) >= v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return static_cast<i64>(v<double>(value) >= static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					return static_cast<i64>(v<double>(value) >= v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator&&(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) && v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) && v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) && static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) && v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator||(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<i64>(value) || v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) || v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Int: {
					return v<double>(value) || static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) || v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator-() const {
	switch (value.index()) {
		case Variant_Int: {
			return -v<i64>(value);
		} break;

		case Variant_Float: {
			return -v<double>(value);
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator!() const {
	switch (value.index()) {
		case Variant_Int: {
			return !v<i64>(value);
		} break;

		case Variant_Float: {
			return !v<double>(value);
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant& Variant::operator++() {
	switch (value.index()) {
		case Variant_Int: {
			++v<i64>(value);
			return *this;
		} break;

		case Variant_Float: {
			++v<double>(value);
			return *this;
		} break;

		default: {
			return *this;
		}
	}
}

Variant& Variant::operator--() {
	switch (value.index()) {
		case Variant_Int: {
			--v<i64>(value);
			return *this;
		} break;

		case Variant_Float: {
			--v<double>(value);
			return *this;
		} break;

		default: {
			return *this;
		}
	}
}

Variant Variant::operator++(int) {
	switch (value.index()) {
		case Variant_Int: {
			Variant result = v<i64>(value);
			++v<i64>(value);
			return result;
		} break;

		case Variant_Float: {
			Variant result = v<double>(value);
			++v<double>(value);
			return result;
		} break;

		default: {
			return Variant();
		}
	}
}

Variant Variant::operator--(int) {
	switch (value.index()) {
		case Variant_Int: {
			Variant result = v<i64>(value);
			--v<i64>(value);
			return result;
		} break;

		case Variant_Float: {
			Variant result = v<double>(value);
			--v<double>(value);
			return result;
		} break;

		default: {
			return Variant();
		}
	}
}

Variant Variant::operator_contains(const Variant& rhs) {
	if (value.index() == Variant_String && rhs.value.index() == Variant_String) {
		return static_cast<i64>(v<std::string>(value).contains(v<std::string>(rhs.value)));
	} else {
		return Variant();
	}
}

#undef i64
#undef v
