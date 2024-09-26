#include "expression_parser/token.h"

#include <cmath>
#include <algorithm>
#include <format>

#include <stdexcept>

using namespace ExpressionParserV2;

#define i64 std::int64_t
#define v std::get

namespace {
	template <typename T>
	struct CoercionResult {
		T value;
		bool success;
	};

	CoercionResult<i64> try_coerce_to_int(const std::string& what) {
		try {
			i64 coerced = std::stoll(what);
			return {coerced, true};
		} catch (...) {
			return {0, false};
		}
	}

	CoercionResult<double> try_coerce_to_float(const std::string& what) {
		try {
			double coerced = std::stod(what);
			return {coerced, true};
		} catch (...) {
			return {0.0, false};
		}
	}
}

std::optional<Variant> StoryVariableInfo::get_variable_value(const std::string& variable) const {
	std::string final_var = resolve_redirects(variable);

	if (auto constant_value = constants.find(final_var); constant_value != constants.end()) {
		return constant_value->second;
	}

	for (auto it = function_arguments_stack.rbegin(); it != function_arguments_stack.rend(); ++it) {
		if (auto argument_value = it->find(final_var); argument_value != it->end()) {
			return argument_value->second;
		}
	}

	if (auto variable_value = variables.find(final_var); variable_value != variables.end()) {
		return variable_value->second;
	}

	return std::nullopt;
}

void StoryVariableInfo::set_variable_value(const std::string& variable, const Variant& value, bool ignore_redirects) {
	std::string final_var = ignore_redirects ? variable : resolve_redirects(variable);

	for (auto it = function_arguments_stack.rbegin(); it != function_arguments_stack.rend(); ++it) {
		if (auto argument_value = it->find(final_var); argument_value != it->end()) {
			argument_value->second = value;
			return;
		}
	}

	if (auto variable_value = variables.find(final_var); variable_value != variables.end()) {
		variable_value->second = value;
		flag_variable_changed(variable);
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

std::string StoryVariableInfo::resolve_redirects(const std::string& start_var) const {
	const std::string* var = &start_var;
	while (true) {
		bool found_any = false;
		for (auto it = redirects_stack.rbegin(); it != redirects_stack.rend(); ++it) {
			if (auto redirect = it->find(*var); redirect != it->end()) {
				var = &redirect->second;
				found_any = true;
				break;
			}
		}

		if (!found_any) {
			break;
		}
	}
	
	return *var;
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

void Token::fetch_function_value(const StoryVariableInfo& story_vars) {
	if (type == TokenType::Function) {
		if (auto builtin_func = story_vars.builtin_functions.find(value); builtin_func != story_vars.builtin_functions.end()) {
			function = builtin_func->second.first;
		} else if (auto external_func = story_vars.external_functions.find(value); external_func != story_vars.external_functions.end()) {
			function = external_func->second;
		}
	}
}

void Token::increment(bool post, StoryVariableInfo& story_vars) {
	value++;
	store_variable_value(story_vars);
}

void Token::decrement(bool post, StoryVariableInfo& story_vars) {
	value--;
	store_variable_value(story_vars);
}

void Token::add_assign(const Variant& rhs, StoryVariableInfo& story_vars) {
	value += rhs;
	store_variable_value(story_vars);
}

void Token::sub_assign(const Variant& rhs, StoryVariableInfo& story_vars) {
	value -= rhs;
	store_variable_value(story_vars);
}

void Token::mul_assign(const Variant& rhs, StoryVariableInfo& story_vars) {
	value *= rhs;
	store_variable_value(story_vars);
}

void Token::div_assign(const Variant& rhs, StoryVariableInfo& story_vars) {
	value /= rhs;
	store_variable_value(story_vars);
}

void Token::mod_assign(const Variant& rhs, StoryVariableInfo& story_vars) {
	value %= rhs;
	store_variable_value(story_vars);
}

void Token::assign_variable(const Token& other, StoryVariableInfo& story_vars) {
	if (type == TokenType::Variable) {
		story_vars.set_variable_value(variable_name, other.value);
		fetch_variable_value(story_vars);
	}
}

Variant Token::call_function(const std::vector<Variant>& arguments, const StoryVariableInfo& story_variable_info) {
	if (type == TokenType::Function) {
		switch (function_fetch_type) {
			case FunctionFetchType::Builtin: {
				return (function)(arguments);
			} break;

			case FunctionFetchType::External: {
				// TODO: external functions
				/*if (auto func = story_variable_info.deferred_functions.find(value); func != story_variable_info.deferred_functions.end()) {
					return (func->second)(arguments);
				} else {
					throw std::runtime_error("Tried calling an unknown function");
				}*/
			} break;

			case FunctionFetchType::ListSubscript: {
				if (function_argument_count > 0) {
					for (auto& entry : story_variable_info.defined_lists.defined_lists) {
						if (entry.second.get_name() == static_cast<std::string>(value)) {
							return entry.second.get_sublist_from_value(arguments[0], &story_variable_info.defined_lists);
						}
					}
				} else {
					for (auto& entry : story_variable_info.defined_lists.defined_lists) {
						if (entry.second.get_name() == static_cast<std::string>(value)) {
							InkList empty_list{&story_variable_info.defined_lists};
							empty_list.add_origin(entry.first);
							return empty_list;
						}
					}
				}
				
				throw std::runtime_error("Could not find list entry" + std::to_string(static_cast<std::int64_t>(arguments[0])) + " for list " + static_cast<std::string>(value));
			} break;

			case FunctionFetchType::StoryKnot:
			default: {
				throw std::runtime_error("Story knot function was not prepared before evaluating expression");
			} break;
		}
	}
	
	return Variant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define VCON(type) Variant::Variant(type val) : value(static_cast<i64>(val)), _has_value(true) {}

Variant::Variant() : value((std::int64_t)0), _has_value(false) {}
Variant::Variant(bool val) : value(val), _has_value(true) {}
VCON(signed char)
VCON(unsigned char)
VCON(signed short)
VCON(unsigned short)
VCON(signed int)
VCON(unsigned int)
VCON(signed long)
VCON(unsigned long)
VCON(signed long long)
VCON(unsigned long long)
Variant::Variant(float val) : value(static_cast<double>(val)), _has_value(true) {}
Variant::Variant(double val) : value(val), _has_value(true) {}
Variant::Variant(const std::string& val) : value(val), _has_value(true) {}
Variant::Variant(const InkList& val) : value(val), _has_value(true) {}
//Variant::Variant(const VariantValue& val) : value(val), has_value(true) {}

#undef VCON

Variant::Variant(const Variant& from) : value(from.value) {}

Variant& Variant::operator=(const Variant& from) {
	if (this != &from) {
		value = from.value;
		_has_value = from._has_value;
	}

	return *this;
}

std::string Variant::to_printable_string() const {
	if (_has_value) {
		switch (value.index()) {
			case Variant_Bool: {
				return v<bool>(value) ? "true" : "false";
			} break;
				
			case Variant_Int: {
				return std::to_string(v<i64>(value));
			} break;
				
			case Variant_Float: {
				double float_val = v<double>(value);
				constexpr float x = ((8.0f - 2.0f) * 0.3f) + 2.0f;
				// ink prints to 7 figures of precision but ignores trailing zeroes
				if (std::rint(float_val) == float_val) {
					return std::to_string(static_cast<i64>(float_val));
				} else {
					std::string result = std::format("{:.7f}", float_val);
					while (result.back() == '0') {
						result.pop_back();
					}

					return result;
				}
			} break;

			case Variant_String: {
				return v<std::string>(value);
			} break;

			case Variant_List: {
				const InkList& list = v<InkList>(value);

				std::string result;
				result.reserve(list.size() * 16);
				
				std::size_t index = 0;
				for (auto it = list.cbegin(); it != list.cend(); ++it) {
					result += it->label;
					++index;
					if (index < list.size()) {
						result += ", ";
					}
				}

				return result;
			} break;
				
			default: {
				return std::string();
			} break;
		}
	} else {
		return std::string();
	}
}

Variant Variant::operator+(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<i64>(v<bool>(value)) + static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) + static_cast<i64>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) + static_cast<double>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) + static_cast<i64>(v<bool>(rhs.value));
				} break;

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
				case Variant_Bool: {
					return v<double>(value) + static_cast<double>(v<bool>(rhs.value));
				} break;

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

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) + v<InkList>(rhs.value);
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
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<i64>(v<bool>(value)) - static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) - static_cast<i64>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) - static_cast<double>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) - static_cast<i64>(v<bool>(rhs.value));
				} break;

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
				case Variant_Bool: {
					return v<double>(value) - static_cast<double>(v<bool>(rhs.value));
				} break;

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

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) - v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator*(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<i64>(v<bool>(value)) * static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) * static_cast<i64>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) * static_cast<double>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) * static_cast<i64>(v<bool>(rhs.value));
				} break;

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
				case Variant_Bool: {
					return v<double>(value) * static_cast<double>(v<bool>(rhs.value));
				} break;

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
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<i64>(v<bool>(value)) / static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) / static_cast<i64>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) / static_cast<double>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) / static_cast<i64>(v<bool>(rhs.value));
				} break;

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
				case Variant_Bool: {
					return v<double>(value) / static_cast<double>(v<bool>(rhs.value));
				} break;

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
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<i64>(v<bool>(value)) % static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) % static_cast<i64>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return std::fmod(static_cast<double>(v<bool>(value)), static_cast<double>(v<double>(rhs.value)));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) % static_cast<i64>(v<bool>(rhs.value));
				} break;

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
				case Variant_Bool: {
					return std::fmod(v<double>(value), static_cast<double>(v<bool>(rhs.value)));
				} break;

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

void Variant::operator+=(const Variant& rhs) {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) += static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) += v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<bool>(value));
					v<double>(value) += v<double>(rhs.value);
				} break;

				default: {
					
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<i64>(value) += static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<i64>(value) += v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<i64>(value));
					v<double>(value) += v<double>(rhs.value);
				} break;

				default: {

				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<double>(value) += static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<double>(value) += static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					v<double>(value) += v<double>(rhs.value);
				} break;

				default: {
				
				} break;
			}
		} break;

		case Variant_String: {
			if (rhs.value.index() == Variant_String) {
				v<std::string>(value) += v<std::string>(rhs.value);
			} else {
				
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				v<InkList>(value) += v<InkList>(rhs.value);
			} else {
				
			}
		} break;

		default: {
			
		} break;
	}
}

void Variant::operator-=(const Variant& rhs) {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) -= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) -= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<bool>(value));
					v<double>(value) -= v<double>(rhs.value);
				} break;

				default: {
					
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<i64>(value) -= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<i64>(value) -= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<i64>(value));
					v<double>(value) -= v<double>(rhs.value);
				} break;

				default: {

				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<double>(value) -= static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<double>(value) -= static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					v<double>(value) -= v<double>(rhs.value);
				} break;

				default: {
				
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				v<InkList>(value) -= v<InkList>(rhs.value);
			} else {
				
			}
		} break;

		default: {
			
		} break;
	}
}

void Variant::operator*=(const Variant& rhs) {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) *= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) *= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<bool>(value));
					v<double>(value) *= v<double>(rhs.value);
				} break;

				default: {
					
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<i64>(value) *= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<i64>(value) *= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<i64>(value));
					v<double>(value) *= v<double>(rhs.value);
				} break;

				default: {

				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<double>(value) *= static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<double>(value) *= static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					v<double>(value) *= v<double>(rhs.value);
				} break;

				default: {
				
				} break;
			}
		} break;

		default: {
			
		} break;
	}
}

void Variant::operator/=(const Variant& rhs) {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) /= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) /= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<bool>(value));
					v<double>(value) /= v<double>(rhs.value);
				} break;

				default: {
					
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<i64>(value) /= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<i64>(value) /= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<i64>(value));
					v<double>(value) /= v<double>(rhs.value);
				} break;

				default: {

				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<double>(value) /= static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<double>(value) /= static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					v<double>(value) /= v<double>(rhs.value);
				} break;

				default: {
				
				} break;
			}
		} break;

		default: {
			
		} break;
	}
}

void Variant::operator%=(const Variant& rhs) {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) %= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					value = static_cast<i64>(v<bool>(value));
					v<i64>(value) %= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<bool>(value));
					value = std::fmod(v<double>(value), v<double>(rhs.value));
				} break;

				default: {
					
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					v<i64>(value) %= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					v<i64>(value) %= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					value = static_cast<double>(v<i64>(value));
					value = std::fmod(v<double>(value), v<double>(rhs.value));
				} break;

				default: {

				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					value = std::fmod(v<double>(value), static_cast<double>(v<bool>(rhs.value)));
				} break;

				case Variant_Int: {
					value = std::fmod(v<double>(value), static_cast<double>(v<i64>(rhs.value)));
				} break;

				case Variant_Float: {
					value = std::fmod(v<double>(value), v<double>(rhs.value));
				} break;

				default: {
				
				} break;
			}
		} break;

		default: {
			
		} break;
	}
}

Variant Variant::operator==(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) == v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<bool>(value) == static_cast<bool>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<bool>(value) == static_cast<bool>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<i64>(value)) == v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<i64>(value) == v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) == v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<double>(value)) == v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<double>(value) == static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) == v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_String: {
			switch (rhs.value.index()) {
				case Variant_String: {
					return v<std::string>(value) == v<std::string>(rhs.value);
				} break;

				// i hate it too!
				case Variant_Int: {
					CoercionResult<i64> coerced = try_coerce_to_int(v<std::string>(value));
					if (coerced.success) {
						return coerced.value == v<i64>(rhs.value);
					} else {
						return false;
					}
				} break;

				// i hate it too!
				case Variant_Float: {
					CoercionResult<double> coerced = try_coerce_to_float(v<std::string>(value));
					if (coerced.success) {
						return coerced.value == v<double>(rhs.value);
					} else {
						return false;
					}
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) == v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator!=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) != v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<bool>(value) != static_cast<bool>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<bool>(value) != static_cast<bool>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<i64>(value)) != v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<i64>(value) != v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) != v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<double>(value)) != v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<double>(value) != static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) != v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_String: {
			switch (rhs.value.index()) {
				case Variant_String: {
					return v<std::string>(value) != v<std::string>(rhs.value);
				} break;

				// i hate it too!
				case Variant_Int: {
					CoercionResult<i64> coerced = try_coerce_to_int(v<std::string>(value));
					if (coerced.success) {
						return coerced.value != v<i64>(rhs.value);
					} else {
						return true;
					}
				} break;

				// i hate it too!
				case Variant_Float: {
					CoercionResult<double> coerced = try_coerce_to_float(v<std::string>(value));
					if (coerced.success) {
						return coerced.value != v<double>(rhs.value);
					} else {
						return true;
					}
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) != v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator<(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) < v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) < v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) < v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
			
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) < static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<i64>(value) < v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) < v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<double>(value) < static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<double>(value) < static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) < v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) < v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator>(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) > v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) > v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) > v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
			
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) > static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<i64>(value) > v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) > v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<double>(value) > static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<double>(value) > static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) > v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) > v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator<=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) <= v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) <= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) <= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
			
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) <= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<i64>(value) <= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) <= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<double>(value) <= static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<double>(value) <= static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) <= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) <= v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator>=(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) >= v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<i64>(v<bool>(value)) >= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<bool>(value)) >= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
			
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<i64>(value) >= static_cast<i64>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<i64>(value) >= v<i64>(rhs.value);
				} break;

				case Variant_Float: {
					return static_cast<double>(v<i64>(value)) >= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<double>(value) >= static_cast<double>(v<bool>(rhs.value));
				} break;

				case Variant_Int: {
					return v<double>(value) >= static_cast<double>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<double>(value) >= v<double>(rhs.value);
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_List: {
			if (rhs.value.index() == Variant_List) {
				return v<InkList>(value) >= v<InkList>(rhs.value);
			} else {
				return Variant();
			}
		} break;

		default: {
			return Variant();
		} break;
	}
}

Variant Variant::operator&&(const Variant& rhs) const {
	switch (value.index()) {
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) && v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<bool>(value) && static_cast<bool>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<bool>(value) && static_cast<bool>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<i64>(value)) && v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<bool>(v<i64>(value) && v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<bool>(static_cast<double>(v<i64>(value)) && v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<double>(value)) && v<bool>(rhs.value);
				} break;

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
		case Variant_Bool: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return v<bool>(value) || v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return v<bool>(value) || static_cast<bool>(v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return v<bool>(value) || static_cast<bool>(v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Int: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<i64>(value)) || v<bool>(rhs.value);
				} break;

				case Variant_Int: {
					return static_cast<bool>(v<i64>(value) || v<i64>(rhs.value));
				} break;

				case Variant_Float: {
					return static_cast<bool>(static_cast<double>(v<i64>(value)) || v<double>(rhs.value));
				} break;

				default: {
					return Variant();
				} break;
			}
		} break;

		case Variant_Float: {
			switch (rhs.value.index()) {
				case Variant_Bool: {
					return static_cast<bool>(v<double>(value)) || v<bool>(rhs.value);
				} break;

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
		case Variant_Bool: {
			return !v<bool>(value);
		} break;

		case Variant_Int: {
			return !v<i64>(value);
		} break;

		case Variant_Float: {
			return !v<double>(value);
		} break;

		case Variant_List: {
			return v<InkList>(value).empty();
		} break;

		default: {
			return Variant();
		} break;
	}
}

/*Variant& Variant::operator++() {
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
}*/

void Variant::operator++(int) {
	switch (value.index()) {
		case Variant_Int: {
			//Variant result = v<i64>(value);
			++v<i64>(value);
			//return result;
		} break;

		case Variant_Float: {
			//Variant result = v<double>(value);
			++v<double>(value);
			//return result;
		} break;

		case Variant_List: {
			v<InkList>(value)++;
		} break;

		default: {
			//return Variant();
		}
	}
}

void Variant::operator--(int) {
	switch (value.index()) {
		case Variant_Int: {
			//Variant result = v<i64>(value);
			--v<i64>(value);
			//return result;
		} break;

		case Variant_Float: {
			//Variant result = v<double>(value);
			--v<double>(value);
			//return result;
		} break;

		case Variant_List: {
			v<InkList>(value)--;
		} break;

		default: {
			//return Variant();
		}
	}
}

Variant Variant::operator_contains(const Variant& rhs) const {
	if (value.index() == Variant_String && rhs.value.index() == Variant_String) {
		return v<std::string>(value).contains(v<std::string>(rhs.value));
	} else if (value.index() == Variant_List && rhs.value.index() == Variant_List) {
		return v<InkList>(value).contains(v<InkList>(rhs.value));
	} else {
		return Variant();
	}
}

Variant Variant::operator_not_contains(const Variant& rhs) const {
	if (value.index() == Variant_String && rhs.value.index() == Variant_String) {
		return !v<std::string>(value).contains(v<std::string>(rhs.value));
	} else if (value.index() == Variant_List && rhs.value.index() == Variant_List) {
		return !v<InkList>(value).contains(v<InkList>(rhs.value));
	} else {
		return Variant();
	}
}

Variant Variant::operator_intersect(const Variant& rhs) const {
	if (value.index() == Variant_List && rhs.value.index() == Variant_List) {
		return v<InkList>(value).intersect_with(v<InkList>(rhs.value));
	} else {
		return Variant();
	}
}

Variant::operator bool() const {
	switch (value.index()) {
		case Variant_Bool:
			return v<bool>(value);
		case Variant_Int:
			return static_cast<bool>(v<i64>(value));
		case Variant_Float:
			return static_cast<bool>(v<double>(value));
		case Variant_String:
			return !v<std::string>(value).empty();
		case Variant_List:
			return !v<InkList>(value).empty();
		default:
			return false;
	}
}

Variant::operator i64() const {
	switch (value.index()) {
		case Variant_Bool:
			return static_cast<i64>(v<bool>(value));
		case Variant_Int:
			return v<i64>(value);
		case Variant_Float:
			return static_cast<i64>(v<double>(value));
		default:
			return 0;
	}
}

Variant::operator double() const {
	switch (value.index()) {
		case Variant_Bool:
			return static_cast<double>(v<bool>(value));
		case Variant_Int:
			return static_cast<double>(v<i64>(value));
		case Variant_Float:
			return v<double>(value);
		default:
			return 0.0;
	}
}

Variant::operator std::string() const {
	if (value.index() == Variant_String) {
		return v<std::string>(value);
	} else {
		return std::string();
	}
}

Variant::operator InkList() const {
	if (value.index() == Variant_List) {
		return v<InkList>(value);
	} else {
		return InkList();
	}
}

ByteVec Serializer<Variant>::operator()(const Variant& variant) {
	if (variant.has_value()) {
		ByteVec result = {static_cast<std::uint8_t>(variant.index())};
		switch (variant.index()) {
			case Variant_Bool: {
				Serializer<std::uint8_t> s;
				ByteVec result2 = s(static_cast<std::uint8_t>(static_cast<i64>(variant)));
				result.insert(result.end(), result2.begin(), result2.end());
			} break;

			case Variant_Int: {
				Serializer<i64> s;
				ByteVec result2 = s(variant);
				result.insert(result.end(), result2.begin(), result2.end());
			} break;

			case Variant_Float: {
				Serializer<double> s;
				ByteVec result2 = s(variant);
				result.insert(result.end(), result2.begin(), result2.end());
			} break;

			case Variant_String: {
				Serializer<std::string> s;
				ByteVec result2 = s(variant);
				result.insert(result.end(), result2.begin(), result2.end());
			} break;

			default: break;
		}

		return result;
	} else {
		return {};
	}
}

Variant Deserializer<Variant>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds_index;
	std::uint8_t var_index = ds_index(bytes, index);
	switch (var_index) {
		case Variant_Bool: {
			Deserializer<std::uint8_t> ds;
			return Variant(static_cast<bool>(ds(bytes, index)));
		} break;

		case Variant_Int: {
			Deserializer<i64> ds;
			return Variant(ds(bytes, index));
		} break;

		case Variant_Float: {
			Deserializer<double> ds;
			return Variant(ds(bytes, index));
		} break;

		case Variant_String: {
			Deserializer<std::string> ds;
			return Variant(ds(bytes, index));
		} break;

		default: return Variant();
	}
}

ByteVec Serializer<Token>::operator()(const Token& token) {
	ByteVec result = {static_cast<std::uint8_t>(token.type)};
	ByteVec result2;
	switch (token.type) {
		case TokenType::Keyword: {
			Serializer<std::uint8_t> s;
			result2 = s(static_cast<std::uint8_t>(token.keyword_type));
		} break;

		case TokenType::LiteralBool: {
			Serializer<std::uint8_t> s;
			result2 = s(static_cast<std::uint8_t>(static_cast<bool>(token.value)));
		} break;
		
		case TokenType::LiteralNumberInt: {
			Serializer<i64> s;
			result2 = s(token.value);
		} break;

		case TokenType::LiteralNumberFloat: {
			Serializer<double> s;
			result2 = s(token.value);
		} break;

		case TokenType::LiteralString:
		case TokenType::LiteralKnotName: {
			Serializer<std::string> s;
			result2 = s(token.value);
		} break;

		case TokenType::Operator: {
			result2.push_back(static_cast<std::uint8_t>(token.operator_type));
			result2.push_back(static_cast<std::uint8_t>(token.operator_unary_type));
		} break;

		case TokenType::ParenComma: {
			Serializer<std::uint8_t> s;
			result2 = s(static_cast<std::uint8_t>(token.paren_comma_type));
		} break;

		case TokenType::Function: {
			Serializer<std::string> sstr;
			result2 = sstr(token.value);
			result2.push_back(static_cast<std::uint8_t>(token.function_fetch_type));
			result2.push_back(static_cast<std::uint8_t>(token.function_argument_count));
		} break;

		case TokenType::Variable: {
			Serializer<std::string> s;
			result2 = s(token.variable_name);
		} break;

		default: break;
	}

	result.insert(result.end(), result2.begin(), result2.end());
	return result;
}

Token Deserializer<Token>::operator()(const ByteVec& bytes, std::size_t& index) {
	Deserializer<std::uint8_t> ds8;
	Deserializer<std::int64_t> dsi64;
	Deserializer<double> dsdb;
	Deserializer<std::string> dsstring;

	TokenType type = static_cast<TokenType>(ds8(bytes, index));
	switch (type) {
		case TokenType::Nil: {
			return Token::nil();
		} break;

		case TokenType::Keyword: {
			KeywordType keyword_type = static_cast<KeywordType>(ds8(bytes, index));
			return Token::keyword(keyword_type);
		} break;

		case TokenType::LiteralBool: {
			bool value = static_cast<bool>(ds8(bytes, index));
			return Token::literal_bool(value);
		} break;

		case TokenType::LiteralNumberInt: {
			std::int64_t value = dsi64(bytes, index);
			return Token::literal_int(value);
		} break;

		case TokenType::LiteralNumberFloat: {
			double value = dsdb(bytes, index);
			return Token::literal_float(value);
		} break;

		case TokenType::LiteralString:
		case TokenType::LiteralKnotName: {
			std::string value = dsstring(bytes, index);
			return type == TokenType::LiteralString ? Token::literal_string(value) : Token::literal_knotname(value, true);
		} break;

		case TokenType::Operator: {
			OperatorType op_type = static_cast<OperatorType>(ds8(bytes, index));
			OperatorUnaryType unary_type = static_cast<OperatorUnaryType>(ds8(bytes, index));
			return Token::operat(op_type, unary_type);
		} break;

		

		case TokenType::ParenComma: {
			ParenCommaType paren_comma_type = static_cast<ParenCommaType>(ds8(bytes, index));
			return Token::paren_comma(paren_comma_type);
		} break;
			

		case TokenType::Function: {
			std::string name = dsstring(bytes, index);
			FunctionFetchType fetch_type = static_cast<FunctionFetchType>(ds8(bytes, index));
			std::uint8_t args = ds8(bytes, index);
			switch (fetch_type) {
				case FunctionFetchType::Builtin:
					return Token::function_builtin(name, nullptr, args);
				case FunctionFetchType::External:
					return Token::function_external(name, args);
				case FunctionFetchType::StoryKnot:
				default:
					return Token::function_story_knot(name, args);
			}
		} break;

		case TokenType::Variable: {
			std::string name = dsstring(bytes, index);
			return Token::variable(name);
		} break;

		default: {
			return Token::nil();
		} break;
	}	
}

#undef i64
#undef v
