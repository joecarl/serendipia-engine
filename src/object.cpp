#include <dp/object.h>

namespace dp {

ObjectProperty::operator Object() { 
	return Object(val.as_object());
}



Object::Object(const boost::json::object&& obj) : 
	json_obj(obj) 
{ }

Object::Object(const boost::json::object& obj) : 
	json_obj(obj) 
{ }

Object::Object(const boost::json::value& obj) : 
	json_obj(obj.as_object()) 
{ }

Object::Object(const Object& obj) : 
	json_obj(obj.json_obj) 
{ }




} // namespace dp


/*
bool get_bool(const boost::json::object& obj, const std::string& key, bool default_value) {

	auto iter = obj.find(key);
	
	if (iter == obj.end() || !iter->value().is_bool()) {
		return default_value;
	}

	return iter->value().get_bool();

}

std::string get_string(const boost::json::object& obj, const std::string& key, const std::string& default_value) {

	auto iter = obj.find(key);
	
	if (iter == obj.end() || !iter->value().is_string()) {
		return default_value;
	}

	return iter->value().get_string().c_str();

}


template<typename T>
bool get_number(const boost::json::object& obj, const std::string& key, T default_value) {

	auto iter = obj.find(key);
	
	if (iter == obj.end() || !iter->value().is_number()) {
		return default_value;
	}

	return iter->value().to_number<T>();

}

boost::json::object get_object(const boost::json::object& obj, const std::string& key, const boost::json::object& default_value) {
	
	auto iter = obj.find(key);
	
	if (iter == obj.end() || !iter->value().is_object()) {
		return default_value;
	}

	return iter->value().get_object();

}
*/