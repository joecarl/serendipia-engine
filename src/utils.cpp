
#include <dp/utils.h>
//#include "appinfo.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <chrono>
#include <iomanip>

#ifdef __ANDROID__
#include <android/log.h>
#endif

using std::string;
using std::chrono::system_clock;
using std::cout;
using std::endl;

namespace dp {

void log(const string& txt) {

#ifdef __ANDROID__

	//__android_log_print(ANDROID_LOG_VERBOSE, APP_PKGNAME, "%s", txt.c_str());

#else
	
	cout << txt << endl;
	
#endif

}


string date() {

	time_t t = system_clock::to_time_t(system_clock::now());

	std::stringstream outbuff;	
	outbuff << std::put_time(localtime(&t), "%FT%T%z");
	return outbuff.str();

}


const string& get_wait_string() {

	static string pts;
	static uint8_t resize_timer = 0;
	static uint8_t pts_len = 0;
	
	resize_timer++;
	resize_timer = resize_timer % 30;
	
	if (resize_timer == 19) {
		pts = "...";
		pts.resize(++pts_len % 4);
	}
	
	return pts;
}


string extract_pkg(string& buffer) {
	
	string pkg = "";

	auto pos = buffer.find("\r\n\r\n");

	if (pos != string::npos) {
		pkg = buffer.substr(0, pos);
		buffer.erase(0, pos + 4);
	}

	return pkg;

}


string file_get_contents(const string& filepath) {

	std::ifstream ifs(filepath);
	string content;
	
	content.assign( 
		(std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()) 
	);

	return content;

}


bool file_put_contents(const string& filepath, const string& contents) {

	std::ofstream file;
	file.open(filepath);
	file << contents;
	file.close();

	return file.good();
	
}


bool file_exists(const string& name) {

	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}

}


string exec(const char* cmd) {

	std::array<char, 128> buffer;
	string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;

}


string ltrim(string s) {

	s.erase(s.begin(), find_if (s.begin(), s.end(), [] (unsigned char ch) {
		return !isspace(ch);
	}));

	return s;

}


string rtrim(string s) {

	s.erase(find_if(s.rbegin(), s.rend(), [] (unsigned char ch) {
		return !isspace(ch);
	}).base(), s.end());

	return s;

}


string trim(string s) {

	s = ltrim(s);
	s = rtrim(s);

	return s;

}

} // namespace dp
