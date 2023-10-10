
#include <dp/server/clients.h>
#include <dp/server/baseserver.h>
#include <dp/utils.h>

#include <iostream>
#include <stdlib.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace dp::server {

uint64_t Client::count_instances = 0;

Client::Client(BaseServer* _server, boost::asio::ip::tcp::socket&& _socket) :
	ConnectionHandler(std::move(_socket)),
	server(_server)
{

	this->id = "C" + std::to_string(1 + count_instances++);
	this->nelh = this->create_nelh();

}


bool Client::validate_app_info(const Object& data) {

	string app_version = data["appVersion"];
	string app_name = data["appName"];
	string app_pkgname = data["appPkgname"];
	auto& app_info = this->server->app_info;

	return 
		app_version == app_info.version &&
		app_name == app_info.name &&
		app_pkgname == app_info.pkgname;

}


bool Client::preprocess_pkg(NetPackage& pkg) {

	if (pkg.type == "net/appInfo") {

		if (!this->validate_app_info(pkg.data)) {
			//throw runtime_error("App info does not match");
			cerr << "App info does not match: " << pkg.data << endl;
			//this->dead = true;
			this->socket->close();
			return false;
		}

		this->app_validated = true;

		Object data = {
			{"client_id", this->id}
		};

		this->send_event("net/set_client_id", data);
		//return false;

	} else {

		if (!this->app_validated) {
			//throw runtime_error("Cannot receive packages before validating app");
			cerr << "Cannot receive packages before validating app" << endl;
			return false;
		}

	}

	return true;

}


} // namespace dp::server