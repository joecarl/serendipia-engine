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

	Object& operator=(boost::json::object& _json_obj) {
		this->json_obj = _json_obj;
		return *this;
	}


	ObjectProperty operator[](const char* key) const {
		
		auto iter = this->json_obj.find(key);
		return ObjectProperty(iter->value());
	
	}
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

    friend std::ostream& operator<<(std::ostream& os, const Object& obj) {
		os << obj.json_obj;
		return os;
	}

	const boost::json::object& json() const { return this->json_obj; }

	void set(const std::string& key, const boost::json::value& val) {
		this->json_obj[key] = val; 
	}

/*
	std::string sget(const std::string& key, const std::string& default_value = "");

	Object sget(const std::string& key, const Object& default_value = boost::json::object({}));

	bool sget(const std::string& key, const bool& default_value = false);
*/
	/**
	 * Retrieves the boolean value held by the json object property whose key is
	 * `key`. If the value does not exist or is not boolean `default_value`
	 * is returned
	 */
	//bool get_bool(const boost::json::object& obj, const std::string& key, bool default_value = false);

	/**
	 * Retrieves the string value held by the json object property whose key is
	 * `key`. If the value does not exist or is not a string `default_value`
	 * is returned
	 */
	//std::string get_string(const boost::json::object& obj, const std::string& key, const std::string& default_value = "");

	/**
	 * Retrieves the numeric value held by the json object property whose key is
	 * `key`. If the value does not exist or is not a number `default_value`
	 * is returned
	 */
	//template<typename T>
	//T get_number(const boost::json::object& obj, const std::string& key, T default_value = 0);

	/**
	 * Retrieves the object value held by the json object property whose key is
	 * `key`. If the value does not exist or is not an object `default_value`
	 * is returned
	 */
	//boost::json::object get_object(const boost::json::object& obj, const std::string& key, const boost::json::object& default_value = {});

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