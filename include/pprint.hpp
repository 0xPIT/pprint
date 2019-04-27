#pragma once
#include <iostream>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <array>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <variant>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <limits>
#include <string_view>
#include <optional>
#include <utility>
#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

// Enum value must be greater or equals than MAGIC_ENUM_RANGE_MIN. By default MAGIC_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MIN.
#if !defined(MAGIC_ENUM_RANGE_MIN)
#  define MAGIC_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than MAGIC_ENUM_RANGE_MAX. By default MAGIC_ENUM_RANGE_MAX = 128.
// If need another max range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX.
#if !defined(MAGIC_ENUM_RANGE_MAX)
#  define MAGIC_ENUM_RANGE_MAX 128
#endif

namespace magic_enum {

// Enum value must be in range [-MAGIC_ENUM_RANGE_MAX, MAGIC_ENUM_RANGE_MIN]. By default  MAGIC_ENUM_RANGE_MIN = -128, MAGIC_ENUM_RANGE_MAX = 128.
// If need another range for all enum types by default, redefine the macro MAGIC_ENUM_RANGE_MAX and MAGIC_ENUM_RANGE_MIN.
// If need another range for specific enum type, add specialization enum_range for necessary enum type.
template <typename E>
struct enum_range final {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_range requires enum type.");
  static constexpr int min = std::is_signed_v<std::underlying_type_t<E>> ? MAGIC_ENUM_RANGE_MIN : 0;
  static constexpr int max = MAGIC_ENUM_RANGE_MAX;
};

static_assert(MAGIC_ENUM_RANGE_MAX > 0,
              "MAGIC_ENUM_RANGE_MAX must be greater than 0.");
static_assert(MAGIC_ENUM_RANGE_MAX < std::numeric_limits<int>::max(),
              "MAGIC_ENUM_RANGE_MAX must be less than INT_MAX.");

static_assert(MAGIC_ENUM_RANGE_MIN <= 0,
              "MAGIC_ENUM_RANGE_MIN must be less or equals than 0.");
static_assert(MAGIC_ENUM_RANGE_MIN > std::numeric_limits<int>::min(),
              "MAGIC_ENUM_RANGE_MIN must be greater than INT_MIN.");

namespace detail {

template <typename E, typename U = std::underlying_type_t<E>>
[[nodiscard]] constexpr int min_impl() {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::min_impl requires enum type.");
  constexpr int min = enum_range<E>::min > (std::numeric_limits<U>::min)() ? enum_range<E>::min : (std::numeric_limits<U>::min)();

  return min;
}

template <typename E, typename U = std::underlying_type_t<E>>
[[nodiscard]] constexpr decltype(auto) range_impl() {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::range_impl requires enum type.");
  static_assert(enum_range<E>::max > enum_range<E>::min, "magic_enum::enum_range requires max > min.");
  constexpr int max = enum_range<E>::max < (std::numeric_limits<U>::max)() ? enum_range<E>::max : (std::numeric_limits<U>::max)();
  constexpr auto range = std::make_integer_sequence<int, max - min_impl<E>() + 1>{};

  return range;
}


[[nodiscard]] constexpr bool is_name_char(char c, bool front) noexcept {
  return (!front && c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

template <typename E, E V>
[[nodiscard]] constexpr std::string_view name_impl() noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::name_impl requires enum type.");
#if defined(__clang__)
  std::string_view name{__PRETTY_FUNCTION__};
  constexpr auto suffix = sizeof("]") - 1;
#elif defined(__GNUC__) && __GNUC__ >= 9
  std::string_view name{__PRETTY_FUNCTION__};
  constexpr auto suffix = sizeof("; std::string_view = std::basic_string_view<char>]") - 1;
#elif defined(_MSC_VER)
  std::string_view name{__FUNCSIG__};
  constexpr auto suffix = sizeof(">(void) noexcept") - 1;
#else
  return {}; // Unsupported compiler.
#endif

#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 9) || defined(_MSC_VER)
  name.remove_suffix(suffix);
  for (std::size_t i = name.size(); i > 0; --i) {
    if (!is_name_char(name[i - 1], false)) {
      name.remove_prefix(i);
      break;
    }
  }

  if (name.length() > 0 && is_name_char(name.front(), true)) {
    return name;
  } else {
    return {}; // Value does not have name.
  }
#endif
}

template <typename E, int... I>
[[nodiscard]] constexpr decltype(auto) strings_impl(std::integer_sequence<int, I...>) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::strings_impl requires enum type.");
  constexpr std::array<std::string_view, sizeof...(I)> names{{name_impl<E, static_cast<E>(I + min_impl<E>())>()...}};

  return names;
}

template <typename E>
[[nodiscard]] constexpr std::string_view name_impl(int value) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::name_impl requires enum type.");
  constexpr auto names = strings_impl<E>(range_impl<E>());
  const int i = value - min_impl<E>();

  if (i >= 0 && static_cast<std::size_t>(i) < names.size()) {
    return names[i];
  } else {
    return {}; // Value out of range.
  }
}

template <typename E, int... I>
[[nodiscard]] constexpr decltype(auto) values_impl(std::integer_sequence<int, I...>) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::values_impl requires enum type.");
  constexpr int n = sizeof...(I);
  constexpr std::array<bool, n> valid{{!name_impl<E, static_cast<E>(I + min_impl<E>())>().empty()...}};
  constexpr int num_valid = ((valid[I] ? 1 : 0) + ...);

  std::array<E, num_valid> enums{};
  for (int i = 0, v = 0; i < n && v < num_valid; ++i) {
    if (valid[i]) {
      enums[v++] = static_cast<E>(i + min_impl<E>());
    }
  }

  return enums;
}

template <typename E, std::size_t... I>
[[nodiscard]] constexpr decltype(auto) names_impl(std::integer_sequence<std::size_t, I...>) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::names_impl requires enum type.");
  constexpr auto enums = values_impl<E>(range_impl<E>());
  constexpr std::array<std::string_view, sizeof...(I)> names{{name_impl<E, enums[I]>()...}};

  return names;
}

template <typename E>
[[nodiscard]] constexpr std::optional<E> enum_cast_impl(std::string_view value) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::detail::enum_cast_impl requires enum type.");
  constexpr auto values = values_impl<E>(range_impl<E>());
  constexpr auto count = values.size();
  constexpr auto names = names_impl<E>(std::make_index_sequence<count>{});

  for (std::size_t i = 0; i < count; ++i) {
    if (names[i] == value) {
      return values[i];
    }
  }

  return std::nullopt; // Invalid value or out of range.
}

template<typename T>
using enable_if_enum_t = typename std::enable_if<std::is_enum_v<T>>::type;

template<typename T, bool = std::is_enum_v<T>>
struct is_scoped_enum_impl : std::false_type {};

template<typename T>
struct is_scoped_enum_impl<T, true> : std::bool_constant<!std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template<typename T, bool = std::is_enum_v<T>>
struct is_unscoped_enum_impl : std::false_type {};

template<typename T>
struct is_unscoped_enum_impl<T, true> : std::bool_constant<std::is_convertible_v<T, std::underlying_type_t<T>>> {};

} // namespace magic_enum::detail

// Checks whether T is an Unscoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Unscoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumeration) type. Otherwise, value is equal to false.
template <typename T>
struct is_unscoped_enum : detail::is_unscoped_enum_impl<T> {};

template <typename T>
inline constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

// Checks whether T is an Scoped enumeration type.
// Provides the member constant value which is equal to true, if T is an [Scoped enumeration](https://en.cppreference.com/w/cpp/language/enum#Scoped_enumerations) type. Otherwise, value is equal to false.
template <typename T>
struct is_scoped_enum : detail::is_scoped_enum_impl<T> {};

template <typename T>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// Obtains enum value from enum string name.
template <typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr std::optional<E> enum_cast(std::string_view value) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_cast requires enum type.");

  return detail::enum_cast_impl<E>(value);
}

// Obtains enum value from integer value.
template <typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr std::optional<E> enum_cast(std::underlying_type_t<E> value) noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_cast requires enum type.");

  if (detail::name_impl<E>(static_cast<int>(value)).empty()) {
    return std::nullopt; // Invalid value or out of range.
  } else {
    return static_cast<E>(value);
  }
}

// Returns enum value at specified index.
// No bounds checking is performed: the behavior is undefined if index >= number of enum values.
template<typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr E enum_value(std::size_t index) {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_value requires enum type.");
  constexpr auto values = detail::values_impl<E>(detail::range_impl<E>());

  return assert(index < values.size()), values[index];
}

// Obtains value enum sequence.
template <typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr decltype(auto) enum_values() noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_values requires enum type.");
  constexpr auto values = detail::values_impl<E>(detail::range_impl<E>());

  return values;
}

// Returns number of enum values.
template <typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr std::size_t enum_count() noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_count requires enum type.");
  constexpr auto count = detail::values_impl<E>(detail::range_impl<E>()).size();

  return count;
}

// Obtains string enum name from enum value.
template <typename E, typename D = std::decay_t<E>, typename = detail::enable_if_enum_t<D>>
[[nodiscard]] constexpr std::optional<std::string_view> enum_name(E value) noexcept {
  static_assert(std::is_enum_v<D>, "magic_enum::enum_name requires enum type.");
  const auto name = detail::name_impl<D>(static_cast<int>(value));

  if (name.empty()) {
    return std::nullopt; // Invalid value or out of range.
  } else {
    return name;
  }
}

// Obtains string enum name sequence.
template <typename E, typename = detail::enable_if_enum_t<E>>
[[nodiscard]] constexpr decltype(auto) enum_names() noexcept {
  static_assert(std::is_enum_v<E>, "magic_enum::enum_names requires enum type.");
  constexpr auto count = detail::values_impl<E>(detail::range_impl<E>()).size();
  constexpr auto names = detail::names_impl<E>(std::make_index_sequence<count>{});

  return names;
}

namespace ops {

template <typename E, typename D = std::decay_t<E>, typename = detail::enable_if_enum_t<E>>
std::ostream& operator<<(std::ostream& os, E value) {
  static_assert(std::is_enum_v<D>, "magic_enum::ops::operator<< requires enum type.");
  const auto name = detail::name_impl<D>(static_cast<int>(value));

  if (!name.empty()) {
    os << name;
  }

  return os;
}

template <typename E, typename = detail::enable_if_enum_t<E>>
std::ostream& operator<<(std::ostream& os, std::optional<E> value) {
  static_assert(std::is_enum_v<E>, "magic_enum::ops::operator<< requires enum type.");

  if (value.has_value()) {
    const auto name = detail::name_impl<E>(static_cast<int>(value.value()));
    if (!name.empty()) {
      os << name;
    }
  }

  return os;
}

} // namespace magic_enum::ops

} // namespace magic_enum

namespace pprint {

  // Some utility structs to check template specialization
  template<typename Test, template<typename...> class Ref>
  struct is_specialization : std::false_type {};

  template<template<typename...> class Ref, typename... Args>
  struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

  template<typename ...>
  using to_void = void;
  
  template<typename T, typename = void>
  struct is_container : std::false_type
  {};
  
  template<typename T>
  struct is_container<T,
		    to_void<decltype(std::declval<T>().begin()),
			    decltype(std::declval<T>().end()),
			    typename T::value_type
			    >> : std::true_type // will  be enabled for iterable objects
  {};
  
  class PrettyPrinter {
  public:

    PrettyPrinter() :
      indent_(0),
      newline_(true) {}

    void indent(size_t indent) {
      indent_ = indent;
    }

    void newline(bool newline) {
      newline_ = newline;
    }

    template <typename T>
    void print(T value) {
      print_internal(value, 0, newline_, 0);
    }

  private:

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value == true, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << (newline ? "\n" : "");
    }

    template <typename T>
    typename std::enable_if<std::is_null_pointer<T>::value == true, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << "nullptr" << (newline ? "\n" : "");
    }

    void print_internal(float value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << 'f' << (newline ? "\n" : "");
    }

    void print_internal(double value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << (newline ? "\n" : "");
    }

    void print_internal(const std::string& value, size_t indent = 0, bool newline = false,
			size_t level = 0) {
      std::cout << std::string(indent, ' ') << "\"" << value << "\"" << (newline ? "\n" : "");
    }
    
    void print_internal(const char * value, size_t indent = 0, bool newline = false,
			size_t level = 0) {
      std::cout << std::string(indent, ' ') << "\"" << value << "\"" << (newline ? "\n" : "");
    }

    void print_internal(char value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << "'" << value << "'" << (newline ? "\n" : "");
    }    

    void print_internal_without_quotes(const std::string& value, size_t indent = 0,
				       bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << (newline ? "\n" : "");
    }
    
    void print_internal_without_quotes(const char * value, size_t indent = 0,
				       bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << (newline ? "\n" : "");
    }    

    void print_internal_without_quotes(char value, size_t indent = 0, bool newline = false,
				       size_t level = 0) {
      std::cout << std::string(indent, ' ') << value << (newline ? "\n" : "");
    }        
    
    void print_internal(bool value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') <<
	(value ? "true" : "false") << (newline ? "\n" : "");
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value == true, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      if (value == nullptr) {
	return print_internal(nullptr, indent, newline, level);
      }
      std::cout << std::string(indent, ' ') << "<" << type(value) << " at "
		<< value << ">" << (newline ? "\n" : "");
    }

    std::string demangle(const char* name) {
#ifdef __GNUG__
      int status = -4;      
      std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
	  std::free
	  };
      return (status==0) ? res.get() : name;
#else
      return name;
#endif      
    }

    template <class T>
    std::string type(const T& t) {
      return demangle(typeid(t).name());
    }

    template <typename T>
    typename std::enable_if<std::is_enum<T>::value == true, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      auto enum_string = magic_enum::enum_name(value);
      if (enum_string.has_value()) {
	std::cout << std::string(indent, ' ') << enum_string.value()
		  << (newline ? "\n" : "");
      }
      else {
	std::cout << std::string(indent, ' ') << value
		  << (newline ? "\n" : "");
      }
    }    

    template <typename T>
    typename std::enable_if<std::is_class<T>::value == true &&
			    std::is_enum<T>::value == false &&
			    is_specialization<T, std::variant>::value == false &&
			    is_specialization<T, std::vector>::value == false &&
			    is_specialization<T, std::list>::value == false &&
			    is_specialization<T, std::deque>::value == false &&
			    is_specialization<T, std::set>::value == false &&
			    is_specialization<T, std::multiset>::value == false &&
			    is_specialization<T, std::unordered_set>::value == false &&
			    is_specialization<T, std::unordered_multiset>::value == false &&			    
			    is_specialization<T, std::map>::value == false &&
			    is_specialization<T, std::multimap>::value == false &&
			    is_specialization<T, std::unordered_map>::value == false &&
			    is_specialization<T, std::unordered_multimap>::value == false, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << "<Object " << type(value) << ">"
		<< (newline ? "\n" : "");
    }

    template <typename T>
    typename std::enable_if<std::is_member_function_pointer<T>::value == true, void>::type
    print_internal(T value, size_t indent = 0, bool newline = false, size_t level = 0) {
      std::cout << std::string(indent, ' ') << "<Object.method " << type(value)
		<< ">"
		<< (newline ? "\n" : "");
    }

    template <typename Container>
    typename std::enable_if<is_specialization<Container, std::vector>::value, void>::type    
    print_internal(const Container& value, size_t indent = 0, bool newline = false,
		   size_t level = 0) {
      typedef typename Container::value_type T;
      if (level == 0) {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", 0, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", 0, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", 0, true);
	  print_internal(value.front(), indent + indent_, false, level + 1);
	  if (value.size() > 1 && is_container<T>::value == false)
	    print_internal_without_quotes(", ", 0, true);
	  else if (is_container<T>::value)
	    print_internal_without_quotes(", ", 0, true);
	  for (size_t i = 1; i < value.size() - 1; i++) {
	    print_internal(value[i], indent + indent_, false, level + 1);
	    if (is_container<T>::value == false)
	      print_internal_without_quotes(", ", 0, true);
	    else
	      print_internal_without_quotes(", ", 0, true);	    
	  }
	  if (value.size() > 1) {
	    print_internal(value.back(), indent + indent_, true, level + 1);
	  }
	}
	if (value.size() == 0)
	  print_internal_without_quotes("]\n");
	else if (is_container<T>::value == false)
	  print_internal_without_quotes("]\n");
	else
	  print_internal_without_quotes("\n]\n");
      }
      else {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", indent, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	  if (value.size() > 1)
	    print_internal_without_quotes(", ", 0, false);
	  for (size_t i = 1; i < value.size() - 1; i++) {	      
	    print_internal(value[i], 0, false, level + 1);
	    print_internal_without_quotes(", ", 0, false);
	  }
	  if (value.size() > 1) {
	    print_internal(value.back(), 0, false, level + 1);
	  }
	}
	print_internal_without_quotes("]", 0, false);
      }
      
    }

    template <typename T, unsigned long int S>
    void print_internal(const std::array<T, S>& value, size_t indent = 0, bool newline = false,
		   size_t level = 0) {
      if (level == 0) {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", 0, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", 0, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", 0, true);
	  print_internal(value.front(), indent + indent_, false, level + 1);
	  if (value.size() > 1 && is_container<T>::value == false)
	    print_internal_without_quotes(", ", 0, true);
	  else if (is_container<T>::value)
	    print_internal_without_quotes(", ", 0, true);
	  for (size_t i = 1; i < value.size() - 1; i++) {
	    print_internal(value[i], indent + indent_, false, level + 1);
	    if (is_container<T>::value == false)
	      print_internal_without_quotes(", ", 0, true);
	    else
	      print_internal_without_quotes(", ", 0, true);	    
	  }
	  if (value.size() > 1) {
	    print_internal(value.back(), indent + indent_, true, level + 1);
	  }
	}
	if (value.size() == 0)
	  print_internal_without_quotes("]\n");
	else if (is_container<T>::value == false)
	  print_internal_without_quotes("]\n");
	else
	  print_internal_without_quotes("\n]\n");
      }
      else {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", indent, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	  if (value.size() > 1)
	    print_internal_without_quotes(", ", 0, false);
	  for (size_t i = 1; i < value.size() - 1; i++) {	      
	    print_internal(value[i], 0, false, level + 1);
	    print_internal_without_quotes(", ", 0, false);
	  }
	  if (value.size() > 1) {
	    print_internal(value.back(), 0, false, level + 1);
	  }
	}
	print_internal_without_quotes("]", 0, false);
      }
      
    }    

    template <typename Container>
    typename std::enable_if<is_specialization<Container, std::list>::value ||
			    is_specialization<Container, std::deque>::value, void>::type    
    print_internal(const Container& value, size_t indent = 0, bool newline = false,
		   size_t level = 0) {
      typedef typename Container::value_type T;
      if (level == 0) {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", 0, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", 0, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", 0, true);
	  print_internal(value.front(), indent + indent_, false, level + 1);
	  if (value.size() > 1 && is_container<T>::value == false)
	    print_internal_without_quotes(", ", 0, true);
	  else if (is_container<T>::value)
	    print_internal_without_quotes(", ", 0, true);

	  typename Container::const_iterator iterator;
	  for (iterator = std::next(value.begin()); iterator != std::prev(value.end()); ++iterator) {
	    print_internal(*iterator, indent + indent_, false, level + 1);
	    if (is_container<T>::value == false)
	      print_internal_without_quotes(", ", 0, true);
	    else
	      print_internal_without_quotes(", ", 0, true);	    
	  }
	  
	  if (value.size() > 1) {
	    print_internal(value.back(), indent + indent_, true, level + 1);
	  }
	}
	if (value.size() == 0)
	  print_internal_without_quotes("]\n");
	else if (is_container<T>::value == false)
	  print_internal_without_quotes("]\n");
	else
	  print_internal_without_quotes("\n]\n");
      }
      else {
	if (value.size() == 0) {
	  print_internal_without_quotes("[", indent, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("[", indent, false);
	  print_internal(value.front(), 0, false, level + 1);
	  if (value.size() > 1)
	    print_internal_without_quotes(", ", 0, false);

	  typename Container::const_iterator iterator;
	  for (iterator = std::next(value.begin()); iterator != std::prev(value.end()); ++iterator) {
	    print_internal(*iterator, 0, false, level + 1);
	    print_internal_without_quotes(", ", 0, false);	    
	  }

	  if (value.size() > 1) {
	    print_internal(value.back(), 0, false, level + 1);
	  }
	}
	print_internal_without_quotes("]", 0, false);
      }
      
    }

    template <typename Container>
    typename std::enable_if<is_specialization<Container, std::set>::value ||
			    is_specialization<Container, std::multiset>::value ||
			    is_specialization<Container, std::unordered_set>::value ||
			    is_specialization<Container, std::unordered_multiset>::value, void>::type    
    print_internal(const Container& value, size_t indent = 0, bool newline = false,
		   size_t level = 0) {
      typedef typename Container::value_type T;
      if (level == 0) {
	if (value.size() == 0) {
	  print_internal_without_quotes("{", 0, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("{", 0, false);
	  print_internal(*(value.begin()), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("{", 0, true);
	  print_internal(*(value.begin()), indent + indent_, false, level + 1);
	  if (value.size() > 1 && is_container<T>::value == false)
	    print_internal_without_quotes(", ", 0, true);
	  else if (is_container<T>::value)
	    print_internal_without_quotes(", ", 0, true);

	  typename Container::const_iterator iterator;
	  for (iterator = std::next(value.begin()); iterator != std::prev(value.end()); ++iterator) {
	    print_internal(*iterator, indent + indent_, false, level + 1);
	    if (is_container<T>::value == false)
	      print_internal_without_quotes(", ", 0, true);
	    else
	      print_internal_without_quotes(", ", 0, true);	    
	  }
	  
	  if (value.size() > 1) {
	    print_internal(*(std::prev(value.end())), indent + indent_, true, level + 1);
	  }
	}
	if (value.size() == 0)
	  print_internal_without_quotes("}\n");
	else if (is_container<T>::value == false)
	  print_internal_without_quotes("}\n");
	else
	  print_internal_without_quotes("\n}\n");
      }
      else {
	if (value.size() == 0) {
	  print_internal_without_quotes("{", indent, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("{", indent, false);
	  print_internal(*(value.begin()), 0, false, level + 1);
	}
	else if (value.size() > 0) {
	  print_internal_without_quotes("{", indent, false);
	  print_internal(*(value.begin()), 0, false, level + 1);
	  if (value.size() > 1)
	    print_internal_without_quotes(", ", 0, false);

	  typename Container::const_iterator iterator;
	  for (iterator = std::next(value.begin()); iterator != std::prev(value.end()); ++iterator) {
	    print_internal(*iterator, 0, false, level + 1);
	    print_internal_without_quotes(", ", 0, false);	    
	  }

	  if (value.size() > 1) {
	    print_internal(*(std::prev(value.end())), 0, false, level + 1);
	  }
	}
	print_internal_without_quotes("}", 0, false);
      }
      
    }    

    template <typename T>
    typename std::enable_if<is_specialization<T, std::map>::value == true ||
			    is_specialization<T, std::multimap>::value == true ||
			    is_specialization<T, std::unordered_map>::value == true ||
			    is_specialization<T, std::unordered_multimap>::value == true, void>::type
    print_internal(const T& value, size_t indent = 0, bool newline = false, size_t level = 0) {
      typedef typename T::mapped_type Value;
      if (level == 0) {
	if (value.size() == 0) {
	  print_internal_without_quotes("{", 0, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("{", 0, false);
	  for (auto& kvpair : value) {
	    print_internal(kvpair.first, 0, false, level + 1);
	    print_internal_without_quotes(" : ", 0, false);
	    print_internal(kvpair.second, 0, false, level + 1);
	  }
	}
	else if (value.size() > 0) {
	  size_t count = 0;
	  for (auto& kvpair : value) {
	    if (count == 0) {
	      print_internal_without_quotes("{", 0, true);
	      print_internal(kvpair.first, indent + indent_, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, false, level + 1);
	      if (value.size() > 1 && is_container<Value>::value == false)
		print_internal_without_quotes(", ", 0, true);
	      else if (is_container<Value>::value)
		print_internal_without_quotes(", ", 0, true);
	    }
	    else if (count + 1 < value.size()) {
	      print_internal(kvpair.first, indent + indent_, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, false, level + 1);
	      if (is_container<Value>::value == false)
		print_internal_without_quotes(", ", 0, true);
	      else
		print_internal_without_quotes(", ", 0, true);
	    }
	    else {
	      print_internal(kvpair.first, indent + indent_, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, true, level + 1);
	    }
	    count += 1;
	  }	  
	}
	if (value.size() == 0)
	  print_internal_without_quotes("}\n");
	else if (is_container<Value>::value == false)
	  print_internal_without_quotes("}\n");
	else
	  print_internal_without_quotes("\n}\n");
      }
      
      else {
	if (value.size() == 0) {
	  print_internal_without_quotes("{", indent, false);
	}
	else if (value.size() == 1) {
	  print_internal_without_quotes("{", indent, false);
	  for (auto& kvpair : value) {
	    print_internal(kvpair.first, 0, false, level + 1);
	    print_internal_without_quotes(" : ", 0, false);
	    print_internal(kvpair.second, 0, false, level + 1);
	  }
	}
	else if (value.size() > 0) {
	  size_t count = 0;
	  for (auto& kvpair : value) {
	    if (count == 0) {
	      print_internal_without_quotes("{", indent, false);
	      print_internal(kvpair.first, 0, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, false, level + 1);
	      print_internal_without_quotes(", ", 0, false);
	    }
	    else if (count + 1 < value.size()) {
	      print_internal(kvpair.first, indent + indent_, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, false, level + 1);
	      print_internal_without_quotes(", ", 0, false);
	    }
	    else {
	      print_internal(kvpair.first, 0, false, level + 1);
	      print_internal_without_quotes(" : ", 0, false);
	      print_internal(kvpair.second, 0, false, level + 1);
	    }
	    count += 1;
	  }	  
	}
	print_internal_without_quotes("}", 0, false);
      }
    }

    template <typename Key, typename Value>
    void print_internal(std::pair<Key, Value> value, size_t indent = 0, bool newline = false,
			size_t level = 0) {
      print_internal_without_quotes("(", indent);
      print_internal(value.first, 0, false);
      print_internal_without_quotes(", ");
      print_internal(value.second, 0, false);
      print_internal_without_quotes(")", 0, newline, level);
    }

    template <class ...Ts>
    void print_internal(std::variant<Ts...> value, size_t indent = 0,
			bool newline = false, size_t level = 0) {
      std::visit([=](const auto& value) { print_internal(value, indent, newline, level); }, value);
    }

    template <typename T>
    void print_internal(std::optional<T> value, size_t indent = 0,
			bool newline = false, size_t level = 0) {
      if (value) {
	print_internal(value.value(), indent, newline, level);
      }
      else {
	print_internal_without_quotes("nullopt", indent, newline, level);
      }
    }

    size_t indent_;
    bool newline_;

  };
  
}
