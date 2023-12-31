
#include <dp/client/netgroupshandler.h>
#include <dp/client/baseclient.h>
//#include <stdexcept>
#include <iostream>

using std::cerr;
using std::endl;

namespace dp::client {

static GroupInfo parse_group_info(const Object& g) {
	return GroupInfo {
		.id = g["id"],
		.owner_name = g["owner_name"],
	};
}

NetGroupsHandler::NetGroupsHandler(BaseClient* _client) : 
	client(_client),
	net(&(_client->get_io_client())),
	nelh(net->create_nelh())
{
	this->setup_nelh();
}


void NetGroupsHandler::create_group(dp::client::Connection* net, std::string id, std::string owner_id, boost::json::array& members) {
	
	this->group = std::make_unique<NetGroup>(this->net, id, owner_id, members);

}


void NetGroupsHandler::setup_nelh() {

	nelh->add_event_listener("groups/join", [this] (const Object& data) {

		Object g = data["group"];

		std::string id = g["id"];
		std::string owner_id = g["owner_id"];
		boost::json::array members = g["members"];
		this->create_group(this->net, id, owner_id, members);

	});

	nelh->add_event_listener("groups/leave", [this] (const Object& data) {
		// this destroys the group?
		this->group = nullptr;
	});

	// This event is received when the client connects to the server
	nelh->add_event_listener("groups/info", [this] (const Object& data) {
		this->groups.clear();
		boost::json::array g_arr = data["groups"];
		for (auto& g: g_arr) {
			auto g_info = parse_group_info(g);
			this->groups[g_info.id] = g_info;
		}
	});

	auto on_create_or_update = [this] (const Object& data) {
		auto g_info = parse_group_info(data["group"]);
		this->groups[g_info.id] = g_info;
	};
	nelh->add_event_listener("groups/create", on_create_or_update);
	nelh->add_event_listener("groups/upate", on_create_or_update);

	nelh->add_event_listener("groups/destroy", [this] (const Object& data) {
		std::string g_id = data["id"];
		this->groups.erase(g_id);
	});

}

void NetGroupsHandler::join_group(const std::string& id) {

	this->net->send_request("groups/join", { {"id", id} });
	
}

	
} // namespace dp::client
