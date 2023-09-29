#include <dp/client/netgroup.h>
#include <dp/client/netservice.h>
#include <stdexcept>

using boost::json::object;
using std::string;

namespace dp::client {

static GroupMember parse_group_member(object m) {
	return GroupMember {
		.client_id = m["client_id"].as_string().c_str(),
		.name = m["name"].as_string().c_str(),
		.ready = m["ready"].as_bool(),
	};
}
/*
static GroupMember update_group_member(object m) {

}
*/


NetGroup::NetGroup(NetService* _net, string _id, string _owner_id, boost::json::array members) :
	net(_net),
	nelh(net->create_nelh()),
	id(_id),
	owner_id(_owner_id)
{ 
	for (auto& _m: members) {
		object m = _m.as_object();
		string client_id = m["client_id"].as_string().c_str();
		this->members[client_id] = parse_group_member(m);
	}
}

NetGroup::~NetGroup() {
	
	this->net->remove_nelh(this->nelh);

}

const GroupMember& NetGroup::get_owner() { 
	auto iter = this->members.find(this->owner_id);
	if (iter == this->members.end()) {
		throw std::runtime_error("no matching group owner");
	}
	return iter->second; 
}

void NetGroup::process_group_event(object& evt) {

	this->nelh->add_event_listener("group/member_join", [this] (object& data) {
		object m = data["member"].as_object();
		string client_id = m["client_id"].as_string().c_str();
		this->members[client_id] = parse_group_member(m);
	});

	this->nelh->add_event_listener("group/member_leave", [this] (object& data) {
		string client_id = data["client_id"].as_string().c_str();
		this->members.erase(client_id);
		this->owner_id = data["new_owner_id"].as_string().c_str();
	});

	this->nelh->add_event_listener("group/member_update", [this] (object& data) {

		object m = data["member"].as_object();
		string client_id = m["client_id"].as_string().c_str();
		auto iter = this->members.find(client_id);
		if (iter != this->members.end()) {
			//update_group_member(iter->second, m);
			iter->second = parse_group_member(m);
		}
	});

}

void NetGroup::send_ready_state(bool ready_state) {

	this->net->send_event("group/set_ready_state", {{"state", ready_state}});

}


} // namespace dp::client
