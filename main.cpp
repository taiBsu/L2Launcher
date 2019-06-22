/*
 * Lineage II Launcher for DynDNS servers
 * 
 * © 2019 Tibor Csötönyi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 * 
 * ---------
 *
 * Usage: 
 * 
 * - Place the file into your L2 root directory.
 * - For Lineage II versions >= High Five: rename system/L2.bin -> system/L2.exe
 * - Create a link to the L2Launcher on your desktop. 
 * - Append your host name behind the link path (L2Launcher.exe [hostname]).
 *  -> Example: C:\Path\To\LineageII\L2Launcher.exe example.com
 *  
 * - Run as administrator!
 *  
 *  
 * 
 * This is a VERY simple and unsafe way to start Lineage II using Windows host name resolving.
 * It's a quick and dirty way, please feel free to fix stuff I might have forgotten or coded badly.
 * 
 * The executable modifies the C:\Windows\system32\drivers\etc\hosts file to append (or modify if already exists) the IP
 * for the Lineage II authentification server so you're able to access private Lineage II servers that don't have static IPs.
 * 
 * Again, please keep in mind that this is a 2-hour dirt work of a guy who never worked with Windows Sockets before. Thanks.
 *  
 */

#include <winsock.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#pragma comment(lib, "ws2_32")

void clone_hosts(std::string const& hosts_path, std::string const& l2_authserv, std::string& host_ip)
{
	// open hosts and create a temporary hosts file to modify
	std::ifstream hosts_if(hosts_path.c_str());
	std::ofstream hosts_tmp("hosts_tmp");
	
	if (hosts_if.is_open() && hosts_tmp.is_open()) {
		std::string line;
		bool l2_auth_found = false;

		// copy every single line
		while (hosts_if.good()) {
			std::getline(hosts_if, line);

			// oh, there's already an IP assigned to our Lineage II auth host
			if (line.find(l2_authserv) != std::string::npos) {
				l2_auth_found = true;

				hosts_tmp << host_ip.append("\t").append(l2_authserv);
			}
			// just another random line
			else {
				hosts_tmp << line + "\n";
			}
		}

		// no IP assigned to l2authd.lineage2.com ? let's do this!
		if (!l2_auth_found) {
			hosts_tmp << host_ip.append("\t").append(l2_authserv);
		}

		// close files
		hosts_if.close();
		hosts_tmp.close();
	}
	else {
		throw std::runtime_error("Couldn't read hosts file or couldn't write ./hosts_tmp. Please restart this program as administrator. Exiting.\n");
	}
}

void copy_hosts(std::string const& hosts_path) 
{
	// open hosts file for modification and our temporary file for reading
	std::ofstream hosts_of(hosts_path.c_str());
	std::ifstream hosts_tmp_if("hosts_tmp");

	if (hosts_of.is_open() && hosts_tmp_if.is_open()) {
		// copy content of our temp file into hosts
		hosts_of << hosts_tmp_if.rdbuf();

		// close files
		hosts_of.close();
		hosts_tmp_if.close();

		remove("hosts_tmp");
	}
	else {
		throw std::runtime_error("I either couldn't read the hosts file or couldn't write the temporary file in this folder. Please restart this program as administrator. Exiting.\n");
	}
}

int main()
{
	// create our log file
	std::ofstream logfile("L2Launcher.log");

	if (!logfile.is_open()) {
		std::cout << "Couldn't write log file. This means that you don't have write permissions in this folder. Please fix this and rerun this program. Exiting.\n";
		return -1;
	}

	// get hosts file path
	char* buf = nullptr;
	size_t sz = 0;
	
	std::string hosts_path = _dupenv_s(&buf, &sz, "WINDIR") == 0 && buf ? std::string(buf) : std::string();

	hosts_path.append(R"(\system32\drivers\etc\hosts)");

	// what are we looking for inside our hosts file?
	std::string const l2_authserv = "l2authd.lineage2.com";

	// no arguments -> get Lineage II standard auth host
	std::string const hostname = "taibsu.net";
	std::string host_ip {};

	try {
		// Windows networking stuff
		WSADATA wsa_data;
		WSAStartup(0x202, &wsa_data);

		// this is where the magic happens
		struct hostent* host = gethostbyname(hostname.c_str());

		if (host) {
			// retrieve the IP from our magical object
			struct in_addr* ip_addr = reinterpret_cast<in_addr*>(host->h_addr);

			if (ip_addr) {
				host_ip = std::string(inet_ntoa(*ip_addr));

				std::string const s = "Found hostname " + hostname + " at IP " + host_ip + ".\n";
				std::cout << s;
				logfile << s;
			}
			else {
				std::string const e = "Couldn't convert IP adress to ASCII string. Exiting.\n";
				std::cout << e;
				logfile << e;

				// something went wrong. let's make sure to clean up everything before we leave.
				WSACleanup();
				logfile.close();

				return -1;
			}
		}
		else {
			throw std::runtime_error("Couldn't resolve hostname. Exiting.\n");
		}

		if (host_ip.length() > 0) {	
			std::string const s = "Trying to modify hosts file...\n";
			std::cout << s;
			logfile << s;

			// first, grab all the content out of our hosts file and add our host IP
			clone_hosts(hosts_path, l2_authserv, host_ip);
			// then, replace the existing hosts file with the modified one
			copy_hosts(hosts_path);
		}
		else {
			throw std::runtime_error("Host IP length is zero. Please check your DNS. Exiting.\n");
		}

		std::string const s = "Success. Starting Lineage II.";
		std::cout << s;
		logfile << s;

		// do some cleanup stuff
		WSACleanup();
		logfile.close();

		// start the game!
		ShellExecute(nullptr, "open", "cmd.exe", "/C start system/L2.exe", nullptr, SW_HIDE);
	}
	catch(std::runtime_error const& e) {
		std::cout << e.what();
		return -1;
	}

	// bye.
	return 0;
}
