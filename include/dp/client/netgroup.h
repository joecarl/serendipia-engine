#ifndef NETGROUP_H
#define NETGROUP_H

#include <dp/serendipia.h>
#include <dp/neteventslistenershandler.h>
#include <boost/json.hpp>
#include <unordered_map>

namespace dp::client {
	class Connection;
}

namespace dp::client {

typedef struct {

	std::string client_id;

	std::string name;

	bool ready;

} GroupMember;


class NetGroup {

protected:

	/**
	 * The net service used to receive the group information
	 */
	Connection* net;

	/**
	 * The handler which will allow us to create event listeners 
	 */
	NetEventsListenersHandler* nelh;

	/**
	 * The group id
	 */
	const std::string id;

	/**
	 * Determines the member who is the group owner.
	 * The value will only be modified at the proper server event handler.
	 */
	std::string owner_id;

	/**
	 * The list of members in the group mapped by client_id.
	 * The list will only be modified at the proper server event handler.
	 */
	std::unordered_map<std::string, GroupMember> members;

	std::vector<std::string> sorted_members_ids;

public:

	NetGroup(Connection* net, std::string id, std::string owner_id, boost::json::array& members);

	~NetGroup();

	/**
	 * Retrieves the group owner. A group must always have a owner so if
	 * no owner is found this function will throw a runtime_error.
	 */
	const GroupMember& get_owner();

	/** 
	 * Creates the events handlers for the events emmited by the server which 
	 * will update the group internal data.
	 */
	void set_group_listeners();

	void send_ready_state(bool ready_state);

	NetEventsListenersHandler* get_nelh() { return this->nelh; }

	const GroupMember* get_member_info(const std::string& id);

	const std::vector<GroupMember> get_members();

};

} // namespace dp::client

#endif