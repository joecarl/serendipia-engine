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

Object& Object::operator=(boost::json::object& _json_obj) {
	this->json_obj = _json_obj;
	return *this;
}

ObjectProperty Object::operator[](const char* key) const {
	auto iter = this->json_obj.find(key);
	return ObjectProperty(iter->value());
}

std::string Object::serialize() {
	return boost::json::serialize(this->json_obj);
}

void Object::set(const std::string& key, const boost::json::value& val) {
	this->json_obj[key] = val; 
}

} // namespace dp
