#ifndef OBJECT_H
#define OBJECT_H

#include <boost/json.hpp>

namespace dp {

class Object;

template<typename T>
T get_default();

class ObjectProperty {

	boost::json::value val;

public:

	ObjectProperty(const boost::json::value& _val) : val(_val) { }

	operator Object();

	template<typename T>
	operator T() {
		try {
			return boost::json::value_to<T>(val);
		} catch (std::exception& e) {
			throw std::runtime_error(std::string("Error converting value: ") + e.what());
		}
		throw std::runtime_error("Unknown error converting value");
	}

};

class Object {

	boost::json::object json_obj;

public:

	Object(const boost::json::object&& _json_obj);

	Object(const boost::json::object& _json_obj);
	
	Object(const boost::json::value& obj);

	Object(const Object& obj);

	Object() : json_obj({}) { }

	Object(std::initializer_list<std::pair<boost::json::string_view, boost::json::value_ref>> init) : json_obj(init) { }

	Object& operator=(boost::json::object& _json_obj);

	ObjectProperty operator[](const char* key) const;

	std::string serialize();

/*
	template<typename T>
	T get(const std::string& key) const;

	template<typename T>
	T sget(const std::string& key, const T& default_value = get_default<T>()) const;
*/

	template<typename T>
	T get(const std::string& key) const {

		try {
			auto iter = this->json_obj.find(key);
			return boost::json::value_to<T>(iter->value());
		} catch (std::exception& e) {
			throw std::runtime_error("Error getting key \"" + key + "\": " + e.what());
		}

	}

	template<typename T>
	T sget(const std::string& key, const T& default_value = get_default<T>()) const {

		auto iter = this->json_obj.find(key);
		if (iter == this->json_obj.end()) {
			return default_value;
		}

		auto& v = iter->value();

		if constexpr (std::is_same_v<T, std::string>) {
			if (v.is_string()) return v.get_string().c_str();
		} else if constexpr (std::is_same_v<T, bool>) {
			if (v.is_bool()) return v.get_bool();
		} else if constexpr (std::is_same_v<T, Object>) {
			if (v.is_object()) return Object(v.get_object());
		} else {
			if (v.is_number()) return v.to_number<T>();
		}

		return default_value;

	}

	void set(const std::string& key, const boost::json::value& val);

    friend std::ostream& operator<<(std::ostream& os, const Object& obj) {
		os << obj.json_obj;
		return os;
	}

	const boost::json::object& json() const { return this->json_obj; }

};

template<typename T>
T get_default() {
	if constexpr (std::is_same_v<T, std::string>) {
		return "";
	} else if constexpr (std::is_same_v<T, bool>) {
		return false;
	} else if constexpr (std::is_same_v<T, Object>) {
		return Object({});
	} else {
		return 0;
	}
}

} // namespace dp

#endif