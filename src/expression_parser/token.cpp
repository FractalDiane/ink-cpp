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

Variant Token::call_function(const std::vector<Variant>& arguments) {
	if (type == TokenType::Function) {
		return (function)(arguments);
	}
	
	return Variant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define VCON(type) Variant::Variant(type val) : value(static_cast<i64>(val)), _has_value(true) {}

Variant::Variant() : value(0i64), _has_value(false) {}
VCON(bool)
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
//Variant::Variant(const VariantValue& val) : value(val), has_value(true) {}

#undef VCON

Variant::Variant(const Variant& from) : value(from.value) {}

Variant& Variant::operator=(const Variant& from) {
	if (this != &from) {
		value = from.value;
	}

	return *this;
}

std::string Variant::to_printable_string() const {
	if (_has_value) {
		switch (value.index()) {
			case Variant_Int:
				return std::to_string(v<i64>(value));
			case Variant_Float:
				return std::to_string(v<double>(value));
			case Variant_String:
				return v<std::string>(value);
			default:
				return std::string();
		}
	} else {
		return std::string();
	}
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

		case Variant_String: {
			if (rhs.value.index() == Variant_String) {
				return static_cast<i64>(v<std::string>(value) == v<std::string>(rhs.value));
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

Variant Variant::operator_intersect(const Variant& rhs) {
	return Variant();
}


Variant::operator bool() const {
	switch (value.index()) {
		case Variant_Int:
			return static_cast<bool>(v<i64>(value));
		case Variant_Float:
			return static_cast<bool>(v<double>(value));
		default:
			return false;
	}
}

Variant::operator i64() const {
	switch (value.index()) {
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

ByteVec Serializer<Variant>::operator()(const Variant& variant) {
	if (variant.has_value()) {
		ByteVec result = {static_cast<std::uint8_t>(variant.index())};
		switch (variant.index()) {
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
				case FunctionFetchType::Immediate:
					return Token::function_immediate(name, nullptr, args);
				case FunctionFetchType::Defer:
					return Token::function_deferred(name, args);
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
