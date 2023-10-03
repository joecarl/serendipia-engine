#include <dp/client/netgroup.h>
#include <dp/client/connection.h>
#include <stdexcept>
#include <iostream>

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


NetGroup::NetGroup(Connection* _net, string _id, string _owner_id, boost::json::array& _members) :
	net(_net),
	nelh(net->create_nelh()),
	id(_id),
	owner_id(_owner_id)
{ 
	for (auto& _m: _members) {
		object m = _m.as_object();
		string client_id = m["client_id"].as_string().c_str();
		this->members[client_id] = parse_group_member(m);
		this->sorted_members_ids.push_back(client_id);
	}

	this->set_group_listeners();

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

void NetGroup::set_group_listeners() {

	this->nelh->add_event_listener("group/member_join", [this] (object& data) {
		object m = data["member"].as_object();
		string client_id = m["client_id"].as_string().c_str();
		this->members[client_id] = parse_group_member(m);
		this->sorted_members_ids.push_back(client_id);
	});

	this->nelh->add_event_listener("group/member_leave", [this] (object& data) {
		string client_id = data["client_id"].as_string().c_str();
		this->members.erase(client_id);
		// TODO: delete from sorted_mem
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

const GroupMember* NetGroup::get_member_info(const std::string& id) {
	auto iter = this->members.find(id);
	if (iter == this->members.end()) {
		return nullptr;
	}
	return &(iter->second);
}


const std::vector<GroupMember> NetGroup::get_members() { 

	std::vector<GroupMember> v;
	for (auto& id: this->sorted_members_ids) {
		auto m = this->members.find(id);
		if (m == this->members.end()) {
			std::cerr << "Group member not found: " << id << std::endl;
			continue;
		}
		v.push_back(m->second);
	}

	return v;

}

} // namespace dp::client
