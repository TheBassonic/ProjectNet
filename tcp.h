// TODO
//	Create Session Timeout
//	Name server
//	Fix corrupted stack near "buffer" line 614-ish

#pragma once

#pragma warning(disable: 4996)
#pragma warning(disable: 4477)
#pragma warning(disable: 4244)

#define COMPILE_AS 0x01

#define DEBUG
#undef DEBUG

#include "md5.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>

#if COMPILE_AS == 0x01
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <Ws2tcpip.h>

#pragma region Win_Colors

#define BLACK			0x0
#define BLUE			0x1
#define GREEN			0x2
#define AQUA			0x3
#define RED				0x4
#define PURPLE			0x5
#define YELLOW			0x6
#define WHITE			0x7
#define GRAY			0x8
#define LIGHT_BLUE		0x9
#define LIGHT_GREEN		0xA
#define LIGHT_AQUA		0xB
#define LIGHT_RED		0xC
#define LIGHT_PURPLE	0xD
#define LIGHT_YELLOW	0xE
#define BRIGHT_WHITE	0xF

#define COLOR_DEBUG		0xE0
#define CMD				0x07
#define INT				0x02
#define STR				0x03
#define ERR				0x04
#define FTL				0x40
#define TAG				0x08
#define SPCL			0x06
#define THR				0x07
#define NONE			0x0F

#pragma endregion

#endif
#if COMPILE_AS == 0x02
#include <sys/types.h>
#include <sys/socket.h>
#endif
#define HTTPREQ_GET		0x00
#define HTTPREQ_HEAD	0x01
#define HTTPREQ_POST	0x02
#define HTTPREQ_PUT		0x03
#define HTTPREQ_DELETE	0x04
#define HTTPREQ_TRACE	0x05
#define HTTPREQ_OPTIONS	0x06
#define HTTPREQ_CONNECT	0x07
#define HTTPREQ_PATCH	0x08
#define HTTPREQ_UNKNOWN	0x09

#define MTU_SIZE		0x5DC
#define CUR_THREAD		std::this_thread
#define THREADID		std::this_thread::get_id()

#define SERVER_NAME		"\r\nServer: ProjectNet/2.0.0 (Windows)"

typedef unsigned short PORT;
typedef unsigned long IPV4;
typedef unsigned long long fsize_t;

std::thread::id main_thread_id = THREADID;

enum INIT_ERR {
	ERR_NONE,
	ERR_WSA,
	ERR_SOCKET,
	ERR_OPT,
	ERR_BIND,
	ERR_LISTEN,
	ERR_CONNECT
};

enum CNCT_ERR {
	ERR_BLOCKED = 1
};

namespace chr {
	void writeover(char* source, char* output, size_t s_len, size_t o_len) {
		memset(output, 0, o_len);
		for (size_t i = 0; i < s_len && i < o_len; i++) {
			output[i] = source[i];
		}
	}

	void strip(char* output, char* source, size_t offset, size_t length, size_t o_len) {
		memset(output, 0, o_len);
		for (size_t i = offset; i < 0xFFFF && (i - offset) < length; i++)
			output[i - offset] = source[i];
	}

	void stripto(char* out, char* source, size_t offset, char last, size_t o_len) {
		memset(out, 0, o_len);
		size_t index = offset;
		while (source[index] != last && index - offset <= o_len) {
			out[index - offset] = source[index];
			index++;
		}
	}

	bool contains(char* source, char* search, size_t so_len, size_t se_len) {
		size_t streak = 0;
		for (size_t i = 0; i < so_len && streak < se_len; i++) {
			if (streak == sizeof(search)) return true;
			if (source[i] == search[streak]) streak++;
			else streak = 0;
		}
		return false;
	}

	bool contains(char* source, const char* search, size_t so_len, size_t se_len) {
		size_t streak = 0;
		for (size_t i = 0; i < so_len && streak < se_len; i++) {
			if (streak == sizeof(search)) return true;
			if (source[i] == search[streak]) streak++;
			else streak = 0;
		}
		return false;
	}

	template<typename T>
	bool contains(T* source, T search, size_t size_of_source) {
		for (size_t i = 0; i < size_of_source; i++) {
			if (source[i] == search) return true;
		}

		return false;
	}

	size_t indexof(char* source, char search, int tokens) {
		int _t = tokens;
		for (size_t i = 0; i < 0xFFFF; i++) {
			if (_t == 0) {
				if (source[i] == search) return i;
			}
			else if (_t > 0) {
				if (source[i] == search) _t--;
			}
		}
		return strchr(source, search) - source + 1;
	}

	//Returns index of the last character in the query (Ex indexoftext("Find the word hello", "word") will return the index of 'd')
	size_t indexoftext(char* source, const char* search) {
		size_t streak = 0;
		size_t index = 0;
		size_t se_len = strlen(search);
		while (index <= strlen(source)) {
			if (source[index + streak] == search[streak]) streak++;
			else if (streak == se_len) return index + streak - 1;
			else {
				streak = 0;
				index++;
			}
		}

		return index + streak;
	}

	int strtoint(const char* source) {
		int rtn = 0;
		size_t digit = 1;
		for (size_t i = strlen(source) - 1; i >= 0; i--) {
			if (source[i] - 0x30 > 9) return -1;
			rtn += (source[i] - 0x30) * digit;
			digit++;
		}

		std::cout << rtn << std::endl;
		return rtn;
	}
}

#if COMPILE_AS == 0x01
void SetConsoleColor(int foreground, int background) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), foreground + (background * 0x10));
}

void SetConsoleColor(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
#endif

namespace inet {
	int _send(SOCKET s, const char* buf, int len, int flags) {
		int sent = send(s, buf, len, flags);
#ifdef DEBUG
#if COMPILE_AS == 0x01
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", s);
		SetConsoleColor(NONE);
		printf("Sent ");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%i", sent);
		SetConsoleColor(NONE);
		printf("/");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%i ", len);
		SetConsoleColor(NONE);
		printf("bytes\n");
#endif
#if COMPILE_AS == 0x02
		printf("[0x%X] Sent %i/%i bytes\n", s, sent, len);
#endif
#endif // DEBUG
		return sent;
	}
	int _recv(SOCKET s, char* buf, int len, int flags) {
		int recvd = recv(s, buf, len, flags);
#ifdef DEBUG
#if COMPILE_AS == 0x01
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", s);
		SetConsoleColor(NONE);
		printf("Received ");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%i ", recvd);
		SetConsoleColor(NONE);
		printf("bytes\n");
#endif
#if COMPILE_AS == 0x02
		printf("[0x%X] Received %i/%i bytes\n", s, recvd, len);
#endif
#endif // DEBUG
		return recvd;
	}
}

struct server_config {
	char ip[0xF];
	IPV4 ipv4;
	PORT port;
	PORT alt_port;
	PORT end_port;

	char blacklistfile[0xFF];
	char blocked_ips[0x800];
	char blocked_files[0x800];

	//Time the server will wait (in seconds) until it will check for config updates
	unsigned int updateinterval;
};

const server_config legacy_config = {
	"192.168.1.17",
	inet_addr("192.168.1.17"),
	htons(80),
	htons(8008),
	htons(8080),

	"blacklist.txt",
	"",
	"",


	(unsigned int)10
};

server_config read_config(std::ifstream& config) {
	if (config.tellg() == -1)
		return legacy_config;

	server_config rtn;
	char valbuf[0xFF];

	config.seekg(0, std::ios::end);
	unsigned long long size = config.tellg();
	char* buffer = new char[size];
	config.seekg(0, std::ios::beg);
	config.read(buffer, size);

	memset(rtn.ip, 0, sizeof(rtn.ip));
	chr::stripto(rtn.ip, buffer,
		chr::indexoftext(buffer, "\"IP\": ") + 2,
		'"',
		sizeof(server_config::ip));
	rtn.ipv4 = inet_addr(rtn.ip);
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"PORT\":") + 2,
		',',
		sizeof(valbuf));
	rtn.port = htons(atoi(valbuf));
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"ALT_PORT\":") + 2,
		',',
		sizeof(valbuf));
	rtn.alt_port = htons(atoi(valbuf));
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"END_PORT\":") + 2,
		',',
		sizeof(valbuf));
	rtn.end_port = htons(atoi(valbuf));
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"INTERVAL\":") + 2,
		',',
		sizeof(valbuf));
	rtn.updateinterval = atoi(valbuf);
	memset(rtn.blacklistfile, 0, sizeof(rtn.blacklistfile));
	chr::stripto(rtn.blacklistfile, buffer,
		chr::indexoftext(buffer, "\"BLACKLIST\": ") + 2,
		'"',
		sizeof(server_config::blacklistfile));
	delete[] buffer;
	return rtn;
}

void getblockedips(std::ifstream& file, server_config& cfg) {
	char buffer[0xFFFF];
	memset(buffer, NULL, sizeof(buffer));

	file.seekg(0, std::ios::end);
	_fsize_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	file.read(cfg.blocked_ips, size);
	file.seekg(0, std::ios::beg);
	return;
}

IPV4 resolve_ipv4(const char* host_name) {
	struct hostent *remoteHost = gethostbyname(host_name);
	if (remoteHost == NULL) return NULL;
	if (remoteHost[0].h_addrtype == AF_INET) {
		return (IPV4)remoteHost[0].h_addr_list[0];
	}
	else return NULL;
}

#pragma region CORE

struct serverconn {
	SOCKET	sock;
	IN_ADDR addr_in;
	IPV4	remoteIP;
	PORT	remotePORT;

	unsigned int send(const char* buf, int len, int flags = 0) {
		int sent = inet::_send(sock, buf, len, flags);
		return sent;
	}
	fsize_t send(std::ifstream& file, int flags = 0) {
		fsize_t f_size = 0, sent = 0, read = 0;
		char buffer[MTU_SIZE];
		memset(buffer, NULL, sizeof(buffer));

		file.seekg(0, std::ios::end);
		f_size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (f_size < sizeof(buffer)) {
			file.read(buffer, f_size - read);
			sent += inet::_send(sock, buffer, file.gcount(), flags);
			memset(buffer, NULL, sizeof(buffer));
		}

		else while (file.read(buffer, sizeof(buffer))) {
			read += file.gcount();
			sent += inet::_send(sock, buffer, file.gcount(), flags);
			memset(buffer, NULL, sizeof(buffer));

			if (f_size - read < sizeof(buffer)) {
				file.read(buffer, f_size - read);
				sent += inet::_send(sock, buffer, file.gcount(), flags);
				memset(buffer, NULL, sizeof(buffer));
				break;
			}
		}

#ifdef DEBUG
#if COMPILE_AS == 0x01
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(NONE);
		printf("Finished Sending ");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%llu ", sent);
		SetConsoleColor(NONE);
		printf("bytes\n");
#endif
#if COMPILE_AS == 0x02
		printf("[0x%X] Finished Sending %llu bytes\n", sock, sent);
#endif
#endif // DEBUG

		return f_size;
	}
	unsigned int recv(char* buf, int len, int flags = 0) {
		int recvd = inet::_recv(sock, buf, len, flags);
#ifdef DEBUG
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(NONE);
		printf("Received ");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%u ", recvd);
		SetConsoleColor(NONE);
		printf("bytes\n");
#endif // DEBUG
		return recvd;
	}
	fsize_t recv(std::ofstream& file, int flags = 0) {
		fsize_t f_size = 0, recvd = 0;
		int len;

		char buffer[MTU_SIZE];
		memset(buffer, NULL, sizeof(buffer));

		inet::_recv(sock, buffer, sizeof(buffer), flags);
		f_size = atoll(buffer);

		memset(buffer, NULL, sizeof(buffer));
		while ((len = inet::_recv(sock, buffer, sizeof(buffer), flags)) > 0) {
			file.write(buffer, len);
			recvd += len;
		}

#ifdef DEBUG
#if COMPILE_AS == 0x01
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(NONE);
		printf("Finished Receiving ");
		SetConsoleColor(LIGHT_GREEN, BLACK);
		printf("%llu ", recvd);
		SetConsoleColor(NONE);
		printf("bytes\n");
#endif
#if COMPILE_AS == 0x02
		printf("[0x%X] Finished Sending %llu bytes\n", sock, sent);
#endif
#endif // DEBUG
		return recvd;
	}
	serverconn* operator<<(const char* buf) {
		inet::_send(sock, buf, strlen(buf), 0);
		return this;
	}
	serverconn* operator<<(std::ifstream& file) {
		fsize_t f_size, sent, read;
		char buffer[MTU_SIZE];
		memset(buffer, NULL, sizeof(buffer));

		file.seekg(0, std::ios::end);
		f_size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (f_size < sizeof(buffer)) {
			file.read(buffer, f_size - read);
			sent += inet::_send(sock, buffer, file.gcount(), NULL);
			memset(buffer, NULL, sizeof(buffer));
		}

		else while (file.read(buffer, sizeof(buffer))) {
			read += file.gcount();
			sent += inet::_send(sock, buffer, file.gcount(), NULL);
			memset(buffer, NULL, sizeof(buffer));

			if (f_size - read < sizeof(buffer)) {
				file.read(buffer, f_size - read);
				sent += inet::_send(sock, buffer, file.gcount(), NULL);
				memset(buffer, NULL, sizeof(buffer));

				return this;
			}
		}

		return this;
	}
};

struct updatethreaddata {
	std::string configname;
	std::string blockedname;
	std::string confighash;
	std::string blockedhash;
};

// DO NOT CALL THIS FUNCTION ON A THREAD YOU CARE ABOUT, THIS FUNCTION WILL NEVER CLOSE
void updatethread(struct updatethreaddata data, struct server_config cfg) {
	std::ifstream file;
	unsigned int interval = cfg.updateinterval;

	CUR_THREAD::sleep_for(std::chrono::milliseconds(100));

	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[Update] ");
	SetConsoleColor(NONE);
	printf("Refresh Interval: ");
	SetConsoleColor(INT);
	printf("%u", cfg.updateinterval);
	SetConsoleColor(NONE);
	printf(" seconds\n");

	for (;;) {
		CUR_THREAD::sleep_for(std::chrono::seconds(cfg.updateinterval));

		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[Update] ");
		SetConsoleColor(NONE);
		printf("Checking for Updates...\n");

		file.open(data.configname, std::ios::binary);

		if (file.tellg() != -1) {

			if (md5(file) != data.confighash) {
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[Update] ");
				SetConsoleColor(NONE);
				printf("Hash Mismatch, Updating Settings\n");
				cfg = read_config(file);
				data.confighash = md5(file);
			}

			if (cfg.blacklistfile != data.blockedname.c_str()) {
				data.blockedname = cfg.blacklistfile;
			}

			file.clear();
			file.close();
			file.open(data.blockedname, std::ios::binary);

			if (file.tellg() == -1) {
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[Update] ");
				SetConsoleColor(ERR);
				printf("Could not open \"%s\"\n", data.blockedname.c_str());
				SetConsoleColor(NONE);
			}

			else if (md5(file) != data.blockedhash) {
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[Update] ");
				SetConsoleColor(NONE);
				printf("Hash Mismatch, Updating Blocked IPs\n");
				getblockedips(file, cfg);
				data.blockedhash = md5(file);
			}

			file.clear();
			file.close();

			if (interval != cfg.updateinterval) {
				interval = cfg.updateinterval;
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[Update] ");
				SetConsoleColor(NONE);
				printf("Refresh Interval: ");
				SetConsoleColor(INT);
				printf("%u", cfg.updateinterval);
				SetConsoleColor(NONE);
				printf(" seconds\n");
			}
		}

		else {
			SetConsoleColor(THR);
			printf("<0x%X> ", THREADID);
			SetConsoleColor(TAG);
			printf("[Update] ");
			SetConsoleColor(ERR);
			printf("Could not open \"%s\"\n", data.configname.c_str());
			SetConsoleColor(NONE);
		}
	}

	return;
}

class server {
protected:
	std::vector<std::thread> threads;
	std::thread update;
	std::string configname;
	std::string blockedname;
public:
	SOCKET sock_listen;
	struct sockaddr_in address, addr_in;
	struct server_config config;
	int s_wsaerr, s_svrerr;

	//This constructor will not load configurations, and will not initialize automatically
	server() {}
	//This constructor will not load configurations, but will initialize automatically
	//ip	- Your local IP (192.168.1.1 for example)
	//port	- The port you wish to open for service (FTP: 21, HTTP: 80/8008/8080, HTTPS: 443)
	server(IPV4 ip, PORT port) {
		config.ipv4 = ip;
		config.port = port;
		initialize(s_wsaerr, s_svrerr);
	}
	//This constructor will load conifgurations, create an update thread for the configurations, and will initialize automatically
	//config - The file that contains the configurations ("settings.json")
	server(std::string cfg) 
		: configname(cfg)
	{
		std::ifstream file(cfg.c_str(), std::ios::binary);
		config = read_config(file);
		blockedname = config.blacklistfile;
		file.clear();
		file.close();
		file = std::ifstream(blockedname.c_str(), std::ios::binary);
		getblockedips(file, config);
		file.clear();
		file.close();

		initialize(s_wsaerr, s_svrerr);

		updatethreaddata data;
		data.configname = configname;
		data.blockedname = blockedname;

		update = std::thread(updatethread, data, config);
		//updatethread(data, config);

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock_listen);
		SetConsoleColor(NONE);
		printf("Started Update Thread\n");
	}
	~server() {
		closesocket(sock_listen);
	}

	//Will initialize the server
	//&wsaerr - return for a WSA Error, if any occur
	//&svrerr - return for a Server Error, if any occur
	void initialize(int& wsaerr, int& svrerr) {
		int opt = 0;

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = config.ipv4;
		address.sin_port = config.port;

		if ((sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
#if COMPILE_AS == 0x01
			WSAData wsadata;
			if (WSAStartup(MAKEWORD(1, 1), &wsadata) != 0) {
				SetConsoleColor(THR);
				printf("<MAIN> ");
				SetConsoleColor(TAG);
				printf("[null] ");
				SetConsoleColor(ERR);
				printf("Failed to Start WSA\n");
				SetConsoleColor(NONE);

				SetConsoleColor(THR);
				printf("<MAIN> ");
				SetConsoleColor(TAG);
				printf("[null] ");
				SetConsoleColor(ERR);
				printf("Failed to Create Socket\n");
				SetConsoleColor(NONE);

				wsaerr = WSAGetLastError();
				return;
			}

			else {
				if ((sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
					SetConsoleColor(THR);
					printf("<MAIN> ");
					SetConsoleColor(TAG);
					printf("[null] ");
					SetConsoleColor(ERR);
					printf("Failed to Create Socket\n");
					SetConsoleColor(NONE);

					svrerr = ERR_SOCKET;
					return;
				}

				else {
					svrerr = ERR_NONE;
					return;
				}
			}
#endif
#if COMPILE_AS == 0x02
			perror("Failed to Create Socket\n");
#endif
			svrerr = ERR_SOCKET;
			return;
		}
#if COMPILE_AS == 0x01
		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[Initialization] ");
		SetConsoleColor(NONE);
		printf("Socket Created (");
		SetConsoleColor(INT);
		printf("0x%X", sock_listen);
		SetConsoleColor(NONE);
		printf(")\n");
#endif
#if COMPILE_AS == 0x02
		printf("[0x%X] Socket Created\n", sock_listen);
#endif

		if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(int)) != 0) {
#if COMPILE_AS == 0x01
			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[Initialization] ");
			SetConsoleColor(ERR);
			printf("Failed to Modify Socket\n");
			SetConsoleColor(NONE);
#endif
#if COMPILE_AS == 0x02
			perror("Failed to Modify Socket\n");
#endif
			svrerr = ERR_OPT;
			return;
		}
		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[Initialization] ");
		SetConsoleColor(NONE);
		printf("Socket Modified\n");

		if (bind(sock_listen, (struct sockaddr*)&address, sizeof(address)) != 0) {
			address.sin_port = config.alt_port;
			if (bind(sock_listen, (struct sockaddr*)&address, sizeof(address)) != 0) {
				address.sin_port = config.end_port;
				if (bind(sock_listen, (struct sockaddr*)&address, sizeof(address)) != 0) {
					SetConsoleColor(THR);
					printf("<MAIN> ");
					SetConsoleColor(TAG);
					printf("[Initialization] ");
					SetConsoleColor(ERR);
					printf("Failed to Bind Socket\n");
					SetConsoleColor(NONE);
					svrerr = ERR_BIND;
					return;
				}
			}
		}

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[Initialization] ");
		SetConsoleColor(NONE);
		printf("Socket Bound (");
		SetConsoleColor(STR);
		printf("\"%s:%u\"", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		SetConsoleColor(NONE);
		printf(")\n");

		if (listen(sock_listen, 0xFF) != 0) {
			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[Initialization] ");
			SetConsoleColor(ERR);
			printf("Failed to Listen For Connections\n");
			SetConsoleColor(NONE);
			svrerr = ERR_LISTEN;
			return;
		}

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock_listen);
		SetConsoleColor(NONE);
		printf("Listening for Connections\n");

		svrerr = ERR_NONE;
		return;
	}
	serverconn connect(int& svrerr) {
		serverconn sc;

		char buf[0x400];
		unsigned int err = 0;
		socklen_t in_size = sizeof(sockaddr_in);
		sockaddr_in addr_in;
		sc.sock = accept(sock_listen, (struct sockaddr*)&addr_in, &in_size);
		if (sc.sock == -1) {
			err = WSAGetLastError();

			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[0x%X] ", sock_listen);
			SetConsoleColor(ERR);
			printf("Failed to Accept Connection (%i)\n", err);
			svrerr = err;
		}

		sprintf(buf, "Connected | %s:%u", inet_ntoa(addr_in.sin_addr), ntohs(addr_in.sin_port));
		SetConsoleTitleA(buf);

		if (strstr(config.blocked_ips, inet_ntoa(addr_in.sin_addr))) {
			closesocket(sc.sock);

			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[0x%X] ", sock_listen);
			SetConsoleColor(ERR);
			printf("BLOCKED CONNECTION FROM %s\n", inet_ntoa(addr_in.sin_addr));
			SetConsoleColor(NONE);

			svrerr = ERR_BLOCKED;
			return serverconn();
		}

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock_listen);
		SetConsoleColor(NONE);
		printf("Accepted Connection (");
		SetConsoleColor(INT);
		printf("0x%X", sc.sock);
		SetConsoleColor(NONE);
		printf(") | ");
		SetConsoleColor(STR);
		printf("\"%s:%u\"\n", inet_ntoa(addr_in.sin_addr), ntohs(addr_in.sin_port));
		SetConsoleColor(NONE);

		sc.remoteIP = addr_in.sin_addr.s_addr;
		sc.remotePORT = addr_in.sin_port;
		sc.addr_in = addr_in.sin_addr;

		svrerr = 0;
		return sc;
	}
};

#pragma endregion

#pragma region HTTP

//Returns the index of the first character of the body
size_t endofheader(char* source) {
	int streak = 0;
	for (size_t i = 0; i < sizeof(source); i++) {
		if (streak == 4) return i;
		if (source[i] == '\r' && (streak == 0 || streak == 2)) streak++;
		if (source[i] == '\n' && (streak == 1 || streak == 3)) streak++;
	}
	return sizeof(source);
}

struct http_config {
	char index[0xFF];
	bool blocked_methods[0x9];
};

const http_config legacy_httpconf = { 
	"index.html",
	{ false, false, true, true, true, false, true, true, true }
};

http_config readhttp_config(std::ifstream& config) {
	if (config.tellg() == -1)
		return legacy_httpconf;

	http_config rtn;
	std::stringstream ss;
	char valbuf[0xFF];
	char buffer[0xFFFF];

	config.seekg(0, std::ios::end);
	unsigned long long size = config.tellg();
	char* filebuf = new char[size];
	config.seekg(0, std::ios::beg);
	config.read(filebuf, size);

	memset(rtn.index, 0, sizeof(http_config::index));
	chr::stripto(rtn.index, filebuf,
		chr::indexoftext(filebuf, "\"INDEX\": ") + 2,
		'"',
		sizeof(http_config::index));
	memset(buffer, 0, sizeof(buffer));
	chr::stripto(buffer, filebuf,
		chr::indexoftext(filebuf, "\"BLOCKED_METHODS\": ") + 2,
		'}',
		sizeof(buffer));
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"GET\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_GET];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"HEAD\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_HEAD];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"POST\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_POST];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"PUT\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_PUT];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"DELETE\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_DELETE];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"TRACE\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_TRACE];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"OPTIONS\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_OPTIONS];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"CONNECT\":") + 2,
		',',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_CONNECT];
	ss.clear();
	memset(valbuf, 0, sizeof(valbuf));
	chr::stripto(valbuf, buffer,
		chr::indexoftext(buffer, "\"PATCH\":") + 2,
		' ',
		sizeof(valbuf));
	ss << valbuf;
	ss >> std::boolalpha >> rtn.blocked_methods[HTTPREQ_PATCH];
	ss.clear();
	return rtn;
}

struct httpdata_out {
	char source[0xFFFF];

	BYTE ver;
	USHORT status;
	std::string tag = "";
	char server[0xFF];
	std::string mime;
	USHORT ranges;
	USHORT conttype;
	size_t contlen;
	DATE modified;

	char content[0xFFFF];
};

struct httpdata_in {
public:
	char source[0xFFFF];

	BYTE method = 0;
	char path[0xFF];
	char args[0x400];

	BYTE ver = 0;
	char host[0x20];
	std::string ifnonematch;
	char useragent[0xFF];
	size_t contlen;

	char content[0xFFFF];
};

httpdata_out parseas_httpdata_out(char* source) {
	return httpdata_out();
}

void parseas_httpdata_in(char* source, struct httpdata_in& out) {
	char buffer[0xFFFF];
	memset(out.path, 0, 0xFF);
	memset(out.args, 0, 0x400);
	memset(out.host, 0, 0xFF);
	memset(out.source, 0, 0xFFFF);
	memset(buffer, 0, 0xFFFF);

	chr::writeover(source, out.source, 0xFFFF, sizeof(httpdata_in::source));

	memset(buffer, 0, 0xFFFF);
	chr::strip(buffer, source, 0, chr::indexof(source, ' ', 0), sizeof(buffer));
	if (strstr(buffer, "GET") != NULL)			out.method = HTTPREQ_GET;
	else if (strstr(buffer, "HEAD") != NULL)	out.method = HTTPREQ_HEAD;
	else if (strstr(buffer, "POST") != NULL)	out.method = HTTPREQ_POST;
	else if (strstr(buffer, "PUT") != NULL)		out.method = HTTPREQ_PUT;
	else if (strstr(buffer, "DELETE") != NULL)	out.method = HTTPREQ_DELETE;
	else if (strstr(buffer, "TRACE") != NULL)	out.method = HTTPREQ_TRACE;
	else if (strstr(buffer, "OPTIONS") != NULL) out.method = HTTPREQ_OPTIONS;
	else if (strstr(buffer, "CONNECT") != NULL) out.method = HTTPREQ_CONNECT;
	else if (strstr(buffer, "PATCH") != NULL)	out.method = HTTPREQ_PATCH;
	else										out.method = HTTPREQ_UNKNOWN;

	memset(buffer, 0, 0xFFFF);
	chr::strip(buffer, source,
		chr::indexof(source, ' ', 0) + 2,
		chr::indexof(source, ' ', 1) - (chr::indexof(source, ' ', 0) + 2), sizeof(buffer));
	if (strchr(buffer, '?') != NULL) {
		chr::stripto(out.path, buffer, 0, '?', sizeof(httpdata_in::path));
		chr::writeover(strchr(buffer, '?'), out.args, strlen(strchr(buffer, '?')), sizeof(httpdata_in::args));
	}

	else chr::writeover(buffer, out.path, sizeof(buffer), sizeof(httpdata_in::path));

	memset(buffer, 0, 0xFFFF);
	chr::strip(buffer, source, chr::indexof(source, ' ', 1) + 1, chr::indexof(source, '\r', 0) - (chr::indexof(source, ' ', 1) + 1), sizeof(buffer));
	if (strstr(buffer, "HTTP/0.9") != NULL)			out.ver = 0x00;
	else if (strstr(buffer, "HTTP/1.0") != NULL)	out.ver = 0x01;
	else if (strstr(buffer, "HTTP/1.1") != NULL)	out.ver = 0x02;
	else if (strstr(buffer, "HTTP/2.0") != NULL)	out.ver = 0x03;

	chr::stripto(out.host, source,
		chr::indexoftext(source, "Host:") + 2,
		'\r',
		sizeof(httpdata_in::host));

	chr::stripto(out.useragent, source,
		chr::indexoftext(source, "User-Agent:") + 2,
		'\r',
		sizeof(httpdata_in::useragent));

	memset(buffer, 0, 0xFFFF);
	chr::stripto(buffer, source,
		chr::indexoftext(source, "If-None-Match: ") + 2,
		'"',
		sizeof(buffer));
	out.ifnonematch = buffer;
	memset(buffer, 0, 0xFFFF);

	//strstr(data.content, "\r\n\r\n");
	chr::strip(out.content, source, chr::indexoftext(source, "\r\n\r\n"),
		strlen(source) - chr::indexoftext(source, "\r\n\r\n"), sizeof(httpdata_in::content));
	return;
}

bool allowmethod(char* path) {
	if (path[1] == ':') return false; //Protect Root
	return true;
}

char* versiontostr(BYTE ver) {
	switch (ver) {
	case 0:
		return (char*)"HTTP/0.9";
	case 1:
		return (char*)"HTTP/1.0";
	case 2:
		return (char*)"HTTP/1.1";
	case 3:
		return (char*)"HTTP/2.0";
	default:
		return (char*)"NULL";
	}
}

char* getmime(const char* ext) {
	if (strstr(ext, ".a") != NULL ||
		strstr(ext, ".arc") != NULL ||
		strstr(ext, ".arj") != NULL ||
		strstr(ext, ".bin") != NULL ||
		strstr(ext, ".com") != NULL ||
		strstr(ext, ".dump") != NULL ||
		strstr(ext, ".exe") != NULL ||
		strstr(ext, ".lha") != NULL ||
		strstr(ext, ".lhx") != NULL ||
		strstr(ext, ".lza") != NULL ||
		strstr(ext, ".lzx") != NULL ||
		strstr(ext, ".o") != NULL ||
		strstr(ext, ".psd") != NULL ||
		strstr(ext, ".saveme") != NULL ||
		strstr(ext, ".uu") != NULL ||
		strstr(ext, ".zoo") != NULL) return (char*)"application/octet-stream";
	if (strstr(ext, ".xml") != NULL) return (char*)"application/xml";
	if (strstr(ext, ".pdf") != NULL) return (char*)"application/pdf";
	if (strstr(ext, ".zip") != NULL) return (char*)"application/zip";
	if (strstr(ext, ".js") != NULL) return (char*)"application/javascript";
	if (strstr(ext, ".json") != NULL) return (char*)"application/json";
	if (strstr(ext, ".7z") != NULL) return (char*)"application/x-7z-compressed";
	if (strstr(ext, ".tar") != NULL) return (char*)"application/x-tar";

	if (strstr(ext, ".adp") != NULL) return (char*)"audio/adpcm";
	if (strstr(ext, ".au") != NULL) return (char*)"audio/basic";
	if (strstr(ext, ".mid") != NULL ||
		strstr(ext, ".midi") != NULL) return (char*)"audio/midi";
	if (strstr(ext, ".mp4a") != NULL) return (char*)"audio/mp4";
	if (strstr(ext, ".mpga") != NULL) return (char*)"audio/mpeg";
	if (strstr(ext, ".oga") != NULL) return (char*)"audio/ogg";
	if (strstr(ext, ".uva") != NULL) return (char*)"audio/vnd.dece.audio";
	if (strstr(ext, ".eol") != NULL) return (char*)"audio/vnd.digital-winds";
	if (strstr(ext, ".dra") != NULL) return (char*)"audio/vnd.dra";
	if (strstr(ext, ".dts") != NULL) return (char*)"audio/vnd.dts";
	if (strstr(ext, ".dtshd") != NULL) return (char*)"audio/vnd.dts.hd";
	if (strstr(ext, ".lvp") != NULL) return (char*)"audio/vnd.lucent.voice";
	if (strstr(ext, ".pya") != NULL) return (char*)"audio/vnd.ms-playready.media.pya";
	if (strstr(ext, ".ecelp4800") != NULL) return (char*)"audio/vnd.nuera.ecelp4800";
	if (strstr(ext, ".ecelp7470") != NULL) return (char*)"audio/vnd.nuera.ecelp7470";
	if (strstr(ext, ".ecelp9600") != NULL) return (char*)"audio/vnd.nuera.ecelp9600";
	if (strstr(ext, ".rip") != NULL) return (char*)"audio/vnd.rip";
	if (strstr(ext, ".weba") != NULL) return (char*)"audio/webm";
	if (strstr(ext, ".aac") != NULL) return (char*)"audio/x-aac";
	if (strstr(ext, ".aif") != NULL) return (char*)"audio/x-aiff";
	if (strstr(ext, ".m3u") != NULL) return (char*)"audio/x-mpegurl";
	if (strstr(ext, ".wax") != NULL) return (char*)"audio/x-ms-wax";
	if (strstr(ext, ".wma") != NULL) return (char*)"audio/x-ms-wma";
	if (strstr(ext, ".ram") != NULL) return (char*)"audio/x-pn-realaudio";
	if (strstr(ext, ".rmp") != NULL) return (char*)"audio/x-pn-realaudio-plugin";
	if (strstr(ext, ".wav") != NULL) return (char*)"audio/x-wav";

	if (strstr(ext, ".cdx") != NULL) return (char*)"chemical/x-cdx";
	if (strstr(ext, ".cif") != NULL) return (char*)"chemical/x-cif";
	if (strstr(ext, ".cmdf") != NULL) return (char*)"chemical/x-cmdf";
	if (strstr(ext, ".cml") != NULL) return (char*)"chemical/x-cml";
	if (strstr(ext, ".csml") != NULL) return (char*)"chemical/x-csml";
	if (strstr(ext, ".xyz") != NULL) return (char*)"chemical/x-xyz";

	if (strstr(ext, ".bmp") != NULL) return (char*)"image/bmp";
	if (strstr(ext, ".g3") != NULL) return (char*)"image/g3fax";
	if (strstr(ext, ".gif") != NULL) return (char*)"image/gif";
	if (strstr(ext, ".ief") != NULL) return (char*)"image/ief";
	if (strstr(ext, ".jpg") != NULL ||
		strstr(ext, ".jpeg") != NULL) return (char*)"image/jpeg";
	if (strstr(ext, ".ktx") != NULL) return (char*)"image/ktx";
	if (strstr(ext, ".pjpeg") != NULL) return (char*)"image/pjpeg";
	if (strstr(ext, ".png") != NULL) return (char*)"image/png";
	if (strstr(ext, ".btif") != NULL) return (char*)"image/prs.btif";
	if (strstr(ext, ".svg") != NULL) return (char*)"image/svg+xml";
	if (strstr(ext, ".tiff") != NULL) return (char*)"image/tiff";
	if (strstr(ext, ".psd") != NULL) return (char*)"image/vnd.adobe.photoshop";
	if (strstr(ext, ".uvi") != NULL) return (char*)"image/vnd.dece.graphic";
	if (strstr(ext, ".djvu") != NULL) return (char*)"image/vnd.djvu";
	if (strstr(ext, ".sub") != NULL) return (char*)"image/vnd.dvb.subtitle";
	if (strstr(ext, ".dwg") != NULL) return (char*)"image/vnd.dwg";
	if (strstr(ext, ".dxf") != NULL) return (char*)"image/vnd.dxf";
	if (strstr(ext, ".fbs") != NULL) return (char*)"image/vnd.fastbidsheet";
	if (strstr(ext, ".fpx") != NULL) return (char*)"image/vnd.fpx";
	if (strstr(ext, ".fst") != NULL) return (char*)"image/vnd.fst";
	if (strstr(ext, ".mmr") != NULL) return (char*)"image/vnd.fujixerox.edmics-mmr";
	if (strstr(ext, ".rlc") != NULL) return (char*)"image/vnd.fujixerox.edmics-rlc";
	if (strstr(ext, ".mdi") != NULL) return (char*)"image/vnd.ms-modi";
	if (strstr(ext, ".npx") != NULL) return (char*)"image/vnd.net-fpx";
	if (strstr(ext, ".wbmp") != NULL) return (char*)"image/vnd.wap.wbmp";
	if (strstr(ext, ".xif") != NULL) return (char*)"image/vnd.xiff";
	if (strstr(ext, ".webp") != NULL) return (char*)"image/webp";
	if (strstr(ext, ".rac") != NULL) return (char*)"image/x-cmu-raster";
	if (strstr(ext, ".cmx") != NULL) return (char*)"image/x-cmx";
	if (strstr(ext, ".fh") != NULL) return (char*)"image/x-freehand";
	if (strstr(ext, ".ico") != NULL) return (char*)"image/x-icon";
	if (strstr(ext, ".pcx") != NULL) return (char*)"image/x-pcx";
	if (strstr(ext, ".pic") != NULL) return (char*)"image/x-pict";
	if (strstr(ext, ".pnm") != NULL) return (char*)"image/x-portable-anymap";
	if (strstr(ext, ".pbm") != NULL) return (char*)"image/x-portable-bitmap";
	if (strstr(ext, ".pgm") != NULL) return (char*)"image/x-portable-graymap";
	if (strstr(ext, ".ppm") != NULL) return (char*)"image/x-portable-pixmap";
	if (strstr(ext, ".rgb") != NULL) return (char*)"image/x-rgb";
	if (strstr(ext, ".xbm") != NULL) return (char*)"image/x-xbitmap";
	if (strstr(ext, ".xpm") != NULL) return (char*)"image/x-xpixmap";
	if (strstr(ext, ".xwd") != NULL) return (char*)"image/x-xwindowdump";

	if (strstr(ext, ".eml") != NULL) return (char*)"message/rfc822";

	if (strstr(ext, ".igs") != NULL) return (char*)"model/iges";
	if (strstr(ext, ".msh") != NULL) return (char*)"model/mesh";
	if (strstr(ext, ".dae") != NULL) return (char*)"model/vnd.collada+xml";
	if (strstr(ext, ".dwf") != NULL) return (char*)"model/vnd.dwf";
	if (strstr(ext, ".gdl") != NULL) return (char*)"model/vnd.gdl";
	if (strstr(ext, ".gtw") != NULL) return (char*)"model/vnd.gtw";
	if (strstr(ext, ".mts") != NULL) return (char*)"model/vnd.mts";
	if (strstr(ext, ".vtu") != NULL) return (char*)"model/vnd.vtu";
	if (strstr(ext, ".wrl") != NULL) return (char*)"model/vrml";

	if (strstr(ext, ".ics") != NULL) return (char*)"text/calendar";
	if (strstr(ext, ".css") != NULL) return (char*)"text/css";
	if (strstr(ext, ".csv") != NULL) return (char*)"text/csv";
	if (strstr(ext, ".acgi") != NULL ||
		strstr(ext, ".htm") != NULL ||
		strstr(ext, ".html") != NULL ||
		strstr(ext, ".htmls") != NULL ||
		strstr(ext, ".htx") != NULL ||
		strstr(ext, ".php") != NULL ||
		strstr(ext, ".shtml") != NULL) return (char*)"text/html";
	if (strstr(ext, ".n3") != NULL) return (char*)"text/n3";
	if (strstr(ext, ".txt") != NULL) return (char*)"text/plain";
	if (strstr(ext, ".par") != NULL) return (char*)"text/plain-bas";
	if (strstr(ext, ".dsc") != NULL) return (char*)"text/prs.lines.tag";
	if (strstr(ext, ".rtx") != NULL) return (char*)"text/richtext";
	if (strstr(ext, ".sgml") != NULL) return (char*)"text/sgml";
	if (strstr(ext, ".tsv") != NULL) return (char*)"text/tab-separated-values";
	if (strstr(ext, ".t") != NULL) return (char*)"text/troff";
	if (strstr(ext, ".ttl") != NULL) return (char*)"text/turtle";
	if (strstr(ext, ".uri") != NULL) return (char*)"text/uri-list";
	if (strstr(ext, ".curl") != NULL) return (char*)"text/vnd.curl";
	if (strstr(ext, ".dcurl") != NULL) return (char*)"text/vnd.curl.dcurl";
	if (strstr(ext, ".mcurl") != NULL) return (char*)"text/vnd.curl.mcurl";
	if (strstr(ext, ".scurl") != NULL) return (char*)"text/vnd.curl.scurl";
	if (strstr(ext, ".fly") != NULL) return (char*)"text/vnd.fly";
	if (strstr(ext, ".flx") != NULL) return (char*)"text/vnd.fmi.flexstor";
	if (strstr(ext, ".gv") != NULL) return (char*)"text/vnd.graphviz";
	if (strstr(ext, ".3dml") != NULL) return (char*)"text/vnd.in3d.3dml";
	if (strstr(ext, ".spot") != NULL) return (char*)"text/vnd.in3d.spot";
	if (strstr(ext, ".jad") != NULL) return (char*)"text/vnd.sun.j2me.app-descriptor";
	if (strstr(ext, ".wml") != NULL) return (char*)"text/vnd.wap.wml";
	if (strstr(ext, ".wmls") != NULL) return (char*)"text/vnd.wap.wmlscript";
	if (strstr(ext, ".s") != NULL) return (char*)"text/x-asm";
	if (strstr(ext, ".c") != NULL) return (char*)"text/x-c";
	if (strstr(ext, ".f") != NULL) return (char*)"text/x-fortran";
	if (strstr(ext, ".java") != NULL) return (char*)"text/x-java-source";
	if (strstr(ext, ".p") != NULL) return (char*)"text/x-pascal";
	if (strstr(ext, ".etx") != NULL) return (char*)"text/x-setext";
	if (strstr(ext, ".uu") != NULL) return (char*)"text/x-uuencode";
	if (strstr(ext, ".vcs") != NULL) return (char*)"text/x-vcalendar";
	if (strstr(ext, ".vcf") != NULL) return (char*)"text/x-vcard";
	if (strstr(ext, ".yaml") != NULL) return (char*)"text/yaml";

	if (strstr(ext, ".3gp") != NULL) return (char*)"video/3gpp";
	if (strstr(ext, ".3g2") != NULL) return (char*)"video/3gpp2";
	if (strstr(ext, ".h261") != NULL) return (char*)"video/h261";
	if (strstr(ext, ".h263") != NULL) return (char*)"video/h263";
	if (strstr(ext, ".h264") != NULL) return (char*)"video/h264";
	if (strstr(ext, ".jpgv") != NULL) return (char*)"video/jpeg";
	if (strstr(ext, ".jpm") != NULL) return (char*)"video/jpm";
	if (strstr(ext, ".mj2") != NULL) return (char*)"video/mj2";
	if (strstr(ext, ".mp4") != NULL) return (char*)"video/mp4";
	if (strstr(ext, ".mpeg") != NULL) return (char*)"video/mpeg";
	if (strstr(ext, ".ogv") != NULL) return (char*)"video/ogg";
	if (strstr(ext, ".qt") != NULL) return (char*)"video/quicktime";
	if (strstr(ext, ".uvh") != NULL) return (char*)"video/vnd.dece.hd";
	if (strstr(ext, ".uvm") != NULL) return (char*)"video/vnd.dece.mobile";
	if (strstr(ext, ".uvp") != NULL) return (char*)"video/vnd.dece.pd";
	if (strstr(ext, ".uvs") != NULL) return (char*)"video/vnd.dece.sd";
	if (strstr(ext, ".uvv") != NULL) return (char*)"video/vnd.dece.video";
	if (strstr(ext, ".fvt") != NULL) return (char*)"video/vnd.fvt";
	if (strstr(ext, ".mxu") != NULL) return (char*)"video/vnd.mpegurl";
	if (strstr(ext, ".pyv") != NULL) return (char*)"video/vnd.ms-playready.media.pyv";
	if (strstr(ext, ".uvu") != NULL) return (char*)"video/vnd.uvvu.mp4";
	if (strstr(ext, ".viv") != NULL) return (char*)"video/vnd.vivo";
	if (strstr(ext, ".webm") != NULL) return (char*)"video/vnd.webm";
	if (strstr(ext, ".f4v") != NULL) return (char*)"video/x-f4v";
	if (strstr(ext, ".fli") != NULL) return (char*)"video/x-fli";
	if (strstr(ext, ".flv") != NULL) return (char*)"video/x-flv";
	if (strstr(ext, ".m4v") != NULL) return (char*)"video/x-m4v";
	if (strstr(ext, ".asf") != NULL) return (char*)"video/x-ms-asf";
	if (strstr(ext, ".wm") != NULL) return (char*)"video/x-ms-wm";
	if (strstr(ext, ".wmv") != NULL) return (char*)"video/x-ms-wmv";
	if (strstr(ext, ".wmx") != NULL) return (char*)"video/x-ms-wmx";
	if (strstr(ext, ".wvx") != NULL) return (char*)"video/x-ms-wvx";
	if (strstr(ext, ".avi") != NULL) return (char*)"video/x-msvideo";
	if (strstr(ext, ".movie") != NULL) return (char*)"video/x-sgi-movie";

	if (strstr(ext, ".ice") != NULL) return (char*)"x-conference/z-cooltalk";

	else return (char*)"application/octet-stream";
}

std::string filltag(unsigned short status) {
	switch (status) {
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanantly";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 307: return "Temporary Redirect";
	case 308: return "Permanant Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Requred";
	case 412: return "Precondition Failed";
	case 413: return "Payload Too Large";
	case 414: return "URI Too Long";
	case 415: return "Unsupported Media Type";
	case 416: return "Range Not Satisfiable";
	case 417: return "Expectation Failed";
	case 418: return "I'm a teapot";
	case 422: return "Unprocessable Entity";
	case 425: return "Too Early";
	case 426: return "Upgrade Required";
	case 428: return "Precondition Required";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 451: return "Unavailable For Leagal Reasons";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway timeout";
	case 505: return "HTTP Version Not Supported";
	case 511: return "Network Authentication Required";
	default: return "Internal Server Error";
	}
}

struct http_serverconn : public serverconn {
	unsigned int status(httpdata_out header, int flags = 0) {
		char buffer[MTU_SIZE];
		char headerbuf[0xFF];
		memset(buffer, 0, sizeof(buffer));
		memset(headerbuf, 0, sizeof(headerbuf));

		if (header.tag.empty()) {
			//Fill tag based on status
			header.tag = filltag(header.status);
		}

		strcpy(headerbuf, versiontostr(header.ver));
		strcat(headerbuf, " ");
		itoa(header.status, buffer, 10);
		strcat(headerbuf, buffer);
		memset(buffer, 0, sizeof(buffer));
		strcat(headerbuf, " ");
		strcat(headerbuf, header.tag.c_str());
		strcat(headerbuf, "\r\n\r\n");

		unsigned int sent = inet::_send(sock, headerbuf, sizeof(headerbuf), flags);

		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(SPCL);
		printf("Sent Status");
		SetConsoleColor(NONE);
		printf(" (");
		SetConsoleColor(INT);
		printf("%u bytes", sent);
		SetConsoleColor(NONE);
		printf(")\n");

		return sent;
	}

	fsize_t send(httpdata_out header, std::ifstream& file, bool dont_send_file = false, int flags = 0) {
		fsize_t sent = 0, f_size = 0, read = 0;
		char buffer[MTU_SIZE];
		char headerbuf[0xFF];
		memset(buffer, 0, sizeof(buffer));
		memset(headerbuf, 0, sizeof(headerbuf));

		if (header.tag.empty()) {
			//Fill tag based on status
			header.tag = filltag(header.status);
		}

		if (!dont_send_file) {
			if (file.tellg() == -1) {
				// OVERRIDE HEADER
				header.ver = 2;
				header.status = 404;
				header.tag = "Not Found";
				sent = status(header, flags);

				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[0x%X] ", sock);
				SetConsoleColor(SPCL);
				printf("Sent Header");
				SetConsoleColor(NONE);
				printf(" (");
				SetConsoleColor(INT);
				printf("%llu bytes", sent);
				SetConsoleColor(NONE);
				printf(")\n");

				return sent;
			}

			else {
				file.seekg(0, std::ios::end);
				f_size = file.tellg();
				file.seekg(0, std::ios::beg);

				strcpy(headerbuf, versiontostr(header.ver));
				strcat(headerbuf, " ");
				itoa(header.status, buffer, 10);
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));
				strcat(headerbuf, " ");
				strcat(headerbuf, header.tag.c_str());

				strcat(headerbuf, "\r\nContent-Type: ");
				sprintf(buffer, "%s", header.mime.c_str());
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));

				strcat(headerbuf, "\r\nContent-Length: ");
				sprintf(buffer, "%llu", f_size);
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));

				strcat(headerbuf, "\r\nEtag: \"");
				strcat(headerbuf, md5(file).c_str());
				strcat(headerbuf, "\"");

				strcat(headerbuf, SERVER_NAME);
				strcat(headerbuf, "\r\n\r\n");
				sent = inet::_send(sock, headerbuf, strlen(headerbuf), flags);
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[0x%X] ", sock);
				SetConsoleColor(SPCL);
				printf("Sent Header");
				SetConsoleColor(NONE);
				printf(" (");
				SetConsoleColor(INT);
				printf("%llu bytes", sent);
				SetConsoleColor(NONE);
				printf(")\n");

				sent = 0;

				if (f_size < sizeof(buffer)) {
					file.read(buffer, f_size - read);
					sent += inet::_send(sock, buffer, file.gcount(), flags);
					memset(buffer, NULL, sizeof(buffer));
				}

				else while (file.read(buffer, sizeof(buffer))) {
					read += file.gcount();
					sent += inet::_send(sock, buffer, file.gcount(), flags);
					memset(buffer, NULL, sizeof(buffer));

					if (f_size - read < sizeof(buffer)) {
						file.read(buffer, f_size - read);
						sent += inet::_send(sock, buffer, file.gcount(), flags);
						memset(buffer, NULL, sizeof(buffer));
					}
				}
			}

			SetConsoleColor(THR);
			printf("<0x%X> ", THREADID);
			SetConsoleColor(TAG);
			printf("[0x%X] ", sock);
			SetConsoleColor(SPCL);
			printf("Sent Content");
			SetConsoleColor(NONE);
			printf(" (");
			SetConsoleColor(INT);
			printf("%llu/%llu bytes", sent, f_size);
			SetConsoleColor(NONE);
			printf(")\n");

			file.clear();
			file.close();

			return 0;
		}

		else {
			if (file.tellg() == -1);

			else {
				file.seekg(0, std::ios::end);
				f_size = file.tellg();
				file.seekg(0, std::ios::beg);

				strcpy(headerbuf, versiontostr(header.ver));
				strcat(headerbuf, " ");
				itoa(header.status, buffer, 10);
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));
				strcat(headerbuf, " ");
				strcat(headerbuf, header.tag.c_str());

				strcat(headerbuf, "\r\nContent-Type: ");
				sprintf(buffer, "%s", header.mime.c_str());
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));

				strcat(headerbuf, "\r\nContent-Length: ");
				sprintf(buffer, "%llu", f_size);
				strcat(headerbuf, buffer);
				memset(buffer, 0, sizeof(buffer));

				strcat(headerbuf, "\r\nEtag: \"");
				strcat(headerbuf, md5(file).c_str());
				strcat(headerbuf, "\"");

				strcat(headerbuf, SERVER_NAME);
				strcat(headerbuf, "\r\n\r\n");
				sent = inet::_send(sock, headerbuf, strlen(headerbuf), flags);
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[0x%X] ", sock);
				SetConsoleColor(SPCL);
				printf("Sent Header");
				SetConsoleColor(NONE);
				printf(" (");
				SetConsoleColor(INT);
				printf("%llu bytes", sent);
				SetConsoleColor(NONE);
				printf(")\n");
			}

			file.clear();
			file.close();
			return 0;
		}
	}
	void recv(httpdata_in& hdi, int flags = 0) {
		char buf[0xFFFF];
		memset(buf, 0, sizeof(buf));
		inet::_recv(sock, buf, sizeof(buf), flags);
		parseas_httpdata_in(buf, hdi);
	}
	unsigned int trace(httpdata_out header, const char* data, int flags = 0) {
		fsize_t sent = 0, s_size = 0, read = 0;
		std::stringstream iss(data);
		std::stringstream ss;
		char buffer[MTU_SIZE];
		char headerbuf[0xFF];
		memset(buffer, 0, sizeof(buffer));
		memset(headerbuf, 0, sizeof(headerbuf));

		iss.seekg(0, std::ios::end);
		s_size = iss.tellg();
		iss.seekg(0, std::ios::beg);
		strcpy(headerbuf, versiontostr(header.ver));
		strcat(headerbuf, " ");
		strcat(headerbuf, itoa(header.status, NULL, 10));
		strcat(headerbuf, " ");
		strcat(headerbuf, header.tag.c_str());
		strcat(headerbuf, "\r\nContent-Length: ");
		sprintf(buffer, "%llu", s_size);
		strcat(headerbuf, buffer);
		memset(buffer, 0, sizeof(buffer));
		strcat(headerbuf, SERVER_NAME);
		strcat(headerbuf, "\r\n\r\n");
		sent = inet::_send(sock, headerbuf, strlen(headerbuf), flags);
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(SPCL);
		printf("Sent Header");
		SetConsoleColor(NONE);
		printf(" (");
		SetConsoleColor(INT);
		printf("%llu bytes", sent);
		SetConsoleColor(NONE);
		printf(")\n");

		sent = 0;

		if (s_size < sizeof(buffer)) {
			iss.read(buffer, s_size - read);
			sent += inet::_send(sock, headerbuf, iss.gcount(), flags);
			memset(buffer, NULL, sizeof(buffer));
		}

		else while (iss.read(buffer, sizeof(buffer))) {
			read += iss.gcount();
			sent += inet::_send(sock, headerbuf, iss.gcount(), flags);
			memset(buffer, NULL, sizeof(buffer));

			if (s_size - read < sizeof(buffer)) {
				iss.read(buffer, s_size - read);
				sent += inet::_send(sock, headerbuf, iss.gcount(), flags);
				memset(buffer, NULL, sizeof(buffer));
			}
		}

		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock);
		SetConsoleColor(SPCL);
		printf("Sent Trace");
		SetConsoleColor(NONE);
		printf(" (");
		SetConsoleColor(INT);
		printf("%llu bytes", sent);
		SetConsoleColor(NONE);
		printf(")\n");

		return 0;
	}
};

class http_server : public server {
public:
	struct http_config httpconfig;

	//This constructor will not load configurations, and will not initialize automatically
	http_server() {}
	//This constructor will not load configurations, but will initialize automatically
	//ip	- Your local IP (192.168.1.1 for example)
	//port	- The port you wish to open for service (FTP: 21, HTTP: 80/8008/8080, HTTPS: 443)
	http_server(IPV4 ip, PORT port) {
		config.ipv4 = ip;
		config.port = port;
		initialize(s_wsaerr, s_svrerr);
	}
	//This constructor will load conifgurations, and will initialize automatically
	//config - The file that contains the configurations ("settings.json")
	http_server(std::string cfg)
	{
		configname = cfg;
		std::ifstream file(cfg.c_str(), std::ios::binary);
		config = read_config(file);
		blockedname = config.blacklistfile;
		file.clear();
		file.close();
		file = std::ifstream(blockedname.c_str(), std::ios::binary);
		getblockedips(file, config);
		file.clear();
		file.close();

		initialize(s_wsaerr, s_svrerr);

		updatethreaddata data;
		data.configname = configname;
		data.blockedname = blockedname;

		update = std::thread(updatethread, data, config);
		//updatethread(data, config);

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock_listen);
		SetConsoleColor(NONE);
		printf("Started Update Thread\n");
	}
	~http_server() {
		closesocket(sock_listen);
	}

	http_serverconn connect(int& svrerr) {
		struct http_serverconn sc;

		char buf[0x400];
		unsigned int err = 0;
		socklen_t in_size = sizeof(sockaddr_in);
		sockaddr_in addr_in;
		sc.sock = accept(sock_listen, (struct sockaddr*)&addr_in, &in_size);
		if (sc.sock == -1) {
			err = WSAGetLastError();

			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[0x%X] ", sock_listen);
			SetConsoleColor(ERR);
			printf("Failed to Accept Connection (%i)\n", err);
			svrerr = err;
		}

		sprintf(buf, "Connected | %s:%u", inet_ntoa(addr_in.sin_addr), ntohs(addr_in.sin_port));
		SetConsoleTitleA(buf);

		if (strstr(config.blocked_ips, inet_ntoa(addr_in.sin_addr))) {
			closesocket(sc.sock);

			SetConsoleColor(THR);
			printf("<MAIN> ");
			SetConsoleColor(TAG);
			printf("[0x%X] ", sock_listen);
			SetConsoleColor(ERR);
			printf("BLOCKED CONNECTION FROM %s\n", inet_ntoa(addr_in.sin_addr));
			SetConsoleColor(NONE);

			svrerr = ERR_BLOCKED;
			return http_serverconn();
		}

		SetConsoleColor(THR);
		printf("<MAIN> ");
		SetConsoleColor(TAG);
		printf("[0x%X] ", sock_listen);
		SetConsoleColor(NONE);
		printf("Accepted Connection (");
		SetConsoleColor(INT);
		printf("0x%X", sc.sock);
		SetConsoleColor(NONE);
		printf(") | ");
		SetConsoleColor(STR);
		printf("\"%s:%u\"\n", inet_ntoa(addr_in.sin_addr), ntohs(addr_in.sin_port));
		SetConsoleColor(NONE);

		sc.remoteIP = addr_in.sin_addr.s_addr;
		sc.remotePORT = addr_in.sin_port;
		sc.addr_in = addr_in.sin_addr;

		svrerr = 0;
		return sc;
	}
};

//class http_client : public client {
//public:
//	httpdata_out _stdcall _recv() {
//		struct httpdata_out dta = httpdata_out();
//		char buffer[0xFFFF];
//		memset(buffer, 0, sizeof(buffer));
//		recv(sock_session, buffer, 0xFFFF, 0);
//		dta = parseas_httpdata_out(buffer);
//		return dta;
//	}
//};

#pragma endregion

#pragma region FTP

#define FTP_USERNAME "USER"
#define FTP_GET_SYSTEM "SYST"

struct ftpdata_in {
	char source[0xFFFF];

	char method[0x5];
	//char params[0xFF][0xFF];
};

ftpdata_in parseas_ftpdata_in(char* source) {
	struct ftpdata_in rtn = ftpdata_in();

	memset(rtn.source, NULL, sizeof(ftpdata_in::source));
	memset(rtn.method, NULL, sizeof(ftpdata_in::method));
	//memset(rtn.params, NULL, sizeof(ftpdata_in::params));

	chr::writeover(source, rtn.source, strlen(source), sizeof(ftpdata_in::source));

	char buffer[0xFFFF];
	memset(buffer, 0, sizeof(buffer));
	chr::stripto(buffer, source, 0, ' ', sizeof(buffer));
	if (strlen(buffer) > 4) {
		memset(buffer, 0, sizeof(buffer));
		chr::stripto(buffer, source, 0, '\r', sizeof(buffer));
	}

	chr::writeover(buffer, rtn.method, strlen(buffer), sizeof(ftpdata_in::method));

	return rtn;
}

class ftp_server : public server {
public:
	ftpdata_in _stdcall _recv(SOCKET sock) {
		struct ftpdata_in dta = ftpdata_in();
		char buffer[0xFFFF];
		memset(buffer, 0, sizeof(buffer));
		recv(sock, buffer, sizeof(buffer), 0);
		dta = parseas_ftpdata_in(buffer);
		return dta;
	}
};

#pragma endregion