#include <dp/client/netgroup.h>
#include <dp/client/connection.h>
#include <stdexcept>
#include <iostream>

using std::string;

namespace dp::client {

static GroupMember parse_group_member(const Object& m) {
	return GroupMember {
		.client_id = m["client_id"],
		.name = m["name"],
		.ready = m["ready"],
	};
}
/*
static GroupMember update_group_member(Object m) {

}
*/


NetGroup::NetGroup(Connection* _net, string _id, string _owner_id, boost::json::array& _members) :
	net(_net),
	nelh(net->create_nelh()),
	id(_id),
	owner_id(_owner_id)
{ 
	for (auto& _m: _members) {
		Object m(_m);
		string client_id = m["client_id"];
		this->members[client_id] = parse_group_member(m);
		this->sorted_members_ids.push_back(client_id);
	}

	this->set_group_listeners();

}

NetGroup::~NetGroup() {
	
	this->nelh->dispose();

}

void NetGroup::remove_member(const std::string& id) {
	this->members.erase(id);
	auto pos = std::find(this->sorted_members_ids.begin(), this->sorted_members_ids.end(), id);
	if (pos != this->sorted_members_ids.end()) {
		this->sorted_members_ids.erase(pos, pos + 1);
	}
}

const GroupMember& NetGroup::get_owner() { 
	auto iter = this->members.find(this->owner_id);
	if (iter == this->members.end()) {
		throw std::runtime_error("no matching group owner");
	}
	return iter->second; 
}

void NetGroup::set_group_listeners() {

	this->nelh->add_event_listener("group/member_join", [this] (const Object& data) {
		const Object m = data["member"];
		string client_id = m["client_id"];
		// TODO: check key does not exist...
		this->members[client_id] = parse_group_member(m);
		this->sorted_members_ids.push_back(client_id);
	});

	this->nelh->add_event_listener("group/member_leave", [this] (const Object& data) {
		string client_id = data["client_id"];
		string new_owner_id = data["new_owner_id"];
		this->remove_member(client_id);
		this->owner_id = new_owner_id;
	});

	this->nelh->add_event_listener("group/member_update", [this] (const Object& data) {
		Object m = data["member"];
		string client_id = m["client_id"];
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
