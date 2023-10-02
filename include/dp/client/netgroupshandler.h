#ifndef NETGROUPSHANDLER_H
#define NETGROUPSHANDLER_H

#include <dp/serendipia.h>
#include <dp/client/netgroup.h>
#include <dp/neteventslistenershandler.h>
#include <boost/json.hpp>
#include <map>
#include <memory>

namespace dp::client {
	class BaseClient;
}

namespace dp::client {

typedef struct {

	std::string id;

	std::string owner_name;

	// people, size...

} GroupInfo;

class NetGroupsHandler {

	/**
	 * The list of groups available for the client.
	 */
	std::map<std::string, GroupInfo> groups;

	BaseClient* client;

	Connection* net;

	NetEventsListenersHandler* nelh;
	
	/** 
	 * Creates the event handlers for the events scoped with `groups`
	 */
	void setup_nelh();

protected:

	/**
	 * The current group where the client belongs. If the client never joined
	 * any group or left the group, this value should be nullptr
	 */
	std::unique_ptr<NetGroup> group = nullptr;

	virtual void create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members);	

public:

	NetGroupsHandler(BaseClient* client);

	/**
	 * Sends the `join_group` request to the server
	 */
	void join_group(const std::string& id);

	NetGroup* get_current_group() { return this->group.get(); }

	BaseClient* get_client() { return this->client; }

};

} // namespace dp::client

#endif