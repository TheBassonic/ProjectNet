// server_api.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "tcp.h"

#define SERVER_TYPE 0x02
#define CONTROL_IP "192.168.1.17"

#if SERVER_TYPE == 0x01 //DEFAULT | DEBUG
void action(SOCKET sc.sock) {
#endif
#if SERVER_TYPE == 0x02 //HTTP
void action(struct http_serverconn sc, struct http_config config) {
#endif
#if SERVER_TYPE == 0x03 //FTP
void action(SOCKET sc.sock, struct ftp_config config) {
#endif
	char buf[0xFFFF];
	//char phpname[0xFFFF];
	memset(buf, 0x00, sizeof(buf));
#if SERVER_TYPE == 0x01 //DEFAULT | DEBUG
	//std::cout << "Called action asynchronosly" << std::endl;
	//svr.send("connected", 10, 0);
	recv(sock, buf, sizeof(buf), 0);
	SetConsoleColor(THR);
	printf("<%s> ", thrtotxt());
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf(buf);
#endif
#if SERVER_TYPE == 0x02 //HTTP
	std::ifstream file;
	struct httpdata_in data;
	struct httpdata_out out;
	memset(&data, 0x00, sizeof(httpdata_in));
	bool isphp = false;
	sc.http_serverconn::recv(data);

#pragma region Status
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("METHOD: ");
	SetConsoleColor(INT);
	printf("%u\n", data.method);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("PATH: ");
	SetConsoleColor(STR);
	printf("\"%s\"\n", data.path);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("ARGS: ");
	SetConsoleColor(STR);
	printf("\"%s\"\n", data.args);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("VERSION: ");
	SetConsoleColor(INT);
	printf("%u\n", data.ver);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("HOST: ");
	SetConsoleColor(STR);
	printf("\"%s\"\n", data.host);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("USER_AGENT: ");
	SetConsoleColor(STR);
	printf("\"%s\"\n", data.useragent);
	SetConsoleColor(THR);
	printf("<0x%X> ", THREADID);
	SetConsoleColor(TAG);
	printf("[0x%X:data] ", sc.sock);
	SetConsoleColor(NONE);
	printf("IF_NONE_MATCH: ");
	SetConsoleColor(STR);
	printf("\"%s\"\n", data.ifnonematch.c_str());
	SetConsoleColor(NONE);
#pragma endregion

	if (data.ver != 2) {
		out.ver = 2;
		out.status = 505;
		sc.status(out);
		closesocket(sc.sock);
		return;
	}

	if (data.method == HTTPREQ_UNKNOWN) {
		out.ver = 2;
		out.status = 400;
		sc.status(out);
		closesocket(sc.sock);
		return;
	}

	else if (config.blocked_methods[data.method]) {
		file.open("err/405.html");
		out.ver = 2;
		out.status = 405;
		out.mime = getmime(".html");
		sc.send(out, file);
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X:err] ", sc.sock);
		SetConsoleColor(ERR);
		printf("405: Attempted method \"%s\"\n", data.method);
		SetConsoleColor(NONE);
		closesocket(sc.sock);
		return;
	}

	else if (data.method == HTTPREQ_GET) {
		if (allowmethod(data.path)) {
			isphp = false;
			/*if (strstr(data.path, ".php") != NULL) {
				file.open(data.path, std::ios::binary);

				if (file.tellg() != -1) {
					file.close();
					isphp = true;
					memset(phpname, 0x00, sizeof(phpname));
					sprintf(phpname, "%u.html", sc.sock);

					sprintf(buf, "php -f %s %s >> %s", data.path, data.args, phpname);
					system(buf);
					memset(buf, 0x00, sizeof(buf));
					CUR_THREAD::sleep_for(std::chrono::milliseconds(500));
					file.open(phpname, std::ios::binary);
				}
			}

			else*/ file.open(data.path, std::ios::binary);

			if (file.tellg() == -1) {
				file.close();

				char namebuf[0xFF];
				memset(namebuf, 0, sizeof(namebuf));

				strcpy(namebuf, data.path);
				strcat(namebuf, config.index);
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[0x%X:err] ", sc.sock);
				SetConsoleColor(ERR);
				printf("404: Attempting path \"%s\"\n", namebuf);
				SetConsoleColor(NONE);
				file.open(namebuf, std::ios::binary);
				if (file.tellg() == -1) {
					file.close();
					memset(namebuf, 0, sizeof(namebuf));
					strcpy(namebuf, data.path);
					strcat(namebuf, "/");
					strcat(namebuf, config.index);
					SetConsoleColor(THR);
					printf("<0x%X> ", THREADID);
					SetConsoleColor(TAG);
					printf("[0x%X:err] ", sc.sock);
					SetConsoleColor(ERR);
					printf("404: Attempting path \"%s\"\n", namebuf);
					SetConsoleColor(NONE);
					file.open(namebuf, std::ios::binary);
					if (file.tellg() == -1) {
						file.close();
						file.open("err/404.html", std::ios::binary);
						out.ver = 2;
						out.status = 404;
						out.mime = getmime(".html");
						sc.send(out, file);
						//sprintf(buffer, "HTTP/1.1 404\r\n\r\n<html><head><title>HTTP 404</title></head><body><h1>PROTOTYPE HTTP SERVER</h1><p>File \"%s\" was not found</p></body></html>", dta.path);
						//_s << buffer;
					}

					else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
						out.ver = 2;
						out.status = 304;
						out.mime = getmime(strchr(namebuf, '.'));
						sc.send(out, file, true);
					}
					else {
						out.ver = 2;
						out.status = 200;
						out.mime = getmime(strchr(namebuf, '.'));
						sc.send(out, file);
					}
				}

				else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
					out.ver = 2;
					out.status = 304;
					out.mime = getmime(strchr(namebuf, '.'));
					sc.send(out, file, true);
				}
				else {
					out.ver = 2;
					out.status = 200;
					out.mime = getmime(strchr(namebuf, '.'));
					sc.send(out, file);
				}
			}

			else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
				out.ver = 2;
				out.status = 304;
				out.mime = getmime(strchr(data.path, '.'));
				sc.send(out, file, true);
			}
			else {
				out.ver = 2;
				out.status = 200;
				out.mime = getmime(strchr(data.path, '.'));
				sc.send(out, file);
			}
		}

		else {
			file.open("err/403.html");
			out.ver = 2;
			out.status = 403;
			out.mime = getmime(".html");
			sc.send(out, file);
		}

		if (isphp) {
			memset(buf, 0x00, sizeof(buf));
			sprintf(buf, "del %u.html", sc.sock);
			system(buf);
			memset(buf, 0x00, sizeof(buf));
		}
	}

	else if (data.method == HTTPREQ_HEAD) {
		if (allowmethod(data.path)) {
			file.open(data.path, std::ios::binary);

			if (file.tellg() == -1) {
				file.close();

				char namebuf[0xFF];
				memset(namebuf, 0, sizeof(namebuf));

				strcpy(namebuf, data.path);
				strcat(namebuf, "index.html");
				SetConsoleColor(THR);
				printf("<0x%X> ", THREADID);
				SetConsoleColor(TAG);
				printf("[0x%X:err] ", sc.sock);
				SetConsoleColor(ERR);
				printf("404: Attempting path \"%s\"\n", namebuf);
				SetConsoleColor(NONE);
				file.open(namebuf, std::ios::binary);
				if (file.tellg() == -1) {
					file.close();
					memset(namebuf, 0, sizeof(namebuf));
					strcpy(namebuf, data.path);
					strcat(namebuf, "/index.html");
					SetConsoleColor(THR);
					printf("<0x%X> ", THREADID);
					SetConsoleColor(TAG);
					printf("[0x%X:err] ", sc.sock);
					SetConsoleColor(ERR);
					printf("404: Attempting path \"%s\"\n", namebuf);
					SetConsoleColor(NONE);
					file.open(namebuf, std::ios::binary);
					if (file.tellg() == -1) {
						file.close();
						file.open("err/404.html", std::ios::binary);
						out.ver = 2;
						out.status = 404;
						out.mime = getmime(".html");
						sc.send(out, file);
						SetConsoleColor(THR);
						printf("<0x%X> ", THREADID);
						SetConsoleColor(TAG);
						printf("[0x%X:err] ", sc.sock);
						SetConsoleColor(ERR);
						printf("404: Attempted path \"%s\"\n", data.path);
						SetConsoleColor(NONE);
					}

					else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
						out.ver = 2;
						out.status = 304;
						out.mime = getmime(strchr(namebuf, '.'));
						sc.send(out, file, true);
					}
					else {
						out.ver = 2;
						out.status = 204;
						out.mime = getmime(strchr(namebuf, '.'));
						sc.send(out, file);
					}
				}

				else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
					out.ver = 2;
					out.status = 304;
					out.mime = getmime(strchr(namebuf, '.'));
					sc.send(out, file, true);
				}
				else {
					out.ver = 2;
					out.status = 204;
					out.mime = getmime(strchr(namebuf, '.'));
					sc.send(out, file);
				}
			}

			else if (strstr(data.ifnonematch.c_str(), md5(file).c_str()) != NULL) {
				out.ver = 2;
				out.status = 304;
				out.mime = getmime(strchr(data.path, '.'));
				sc.send(out, file, true);
			}
			else {
				out.ver = 2;
				out.status = 204;
				out.mime = getmime(strchr(data.path, '.'));
				sc.send(out, file);
			}
		}

		else {
			file.open("err/403.html");
			out.ver = 2;
			out.status = 403;
			out.mime = getmime(".html");
			sc.send(out, file);
			SetConsoleColor(THR);
			printf("<0x%X> ", THREADID);
			SetConsoleColor(TAG);
			printf("[0x%X:err] ", sc.sock);
			SetConsoleColor(ERR);
			printf("403: Attempted path: \"%s\"\n", data.path);
			SetConsoleColor(NONE);
		}
	}

	else if (data.method == HTTPREQ_TRACE) {
		std::string str = data.source;
		std::istringstream iss(str, std::ios::binary);
		out.ver = 2;
		out.status = 200;
		sc.trace(out, data.source);
	}

	else {
		file.open("err/501.html");
		out.ver = 2;
		out.status = 501;
		out.mime = getmime(".html");
		sc.send(out, file);
		SetConsoleColor(THR);
		printf("<0x%X> ", THREADID);
		SetConsoleColor(TAG);
		printf("[0x%X:err] ", sc.sock);
		SetConsoleColor(ERR);
		printf("500: Attempting path \"%s\"\n", data.path);
		SetConsoleColor(NONE);
	}
#endif
	closesocket(sc.sock);
	sprintf(buf, "Disconnected | %s:%u", inet_ntoa(sc.addr_in), ntohs(sc.remotePORT));
	SetConsoleTitleA(buf);
}

int main()
{
	std::vector<std::thread> threads;
	std::ifstream config("settings.json", std::ios::binary);
	WSAData wsadata;
	char buf[0xFFFF];
	memset(buf, 0x00, sizeof(buf));
	WSAStartup(MAKEWORD(1, 1), &wsadata);

	SetConsoleColor(THR);
	printf("<THREAD> ");
	SetConsoleColor(TAG);
	printf("[SOCKET] ");
	SetConsoleColor(NONE);
	printf("DATA\n");

#if SERVER_TYPE == 0x01 //DEFAULT | DEBUG
	server _server = server();
	_server.config.address = inet_addr(CONTROL_IP);
	_server.config.port = htons(8000);

	if (_server._initialize() > 0) {
		system("pause");
		return -1;
	}
#endif
#if SERVER_TYPE == 0x02 //HTTP
	http_server _server("settings.json");
	std::ifstream blocked(_server.config.blacklistfile, std::ios::binary);
	getblockedips(blocked, _server.config);
	_server.httpconfig = readhttp_config(config);
	if (_server.s_wsaerr > 0 || _server.s_svrerr > 0) {
		return -1;
	}
#endif
#if SERVER_TYPE == 0x03 //FTP
	ftp_server _server = ftp_server();
	std::ifstream file;
	struct ftpdata_in data;
	_server.config.address = inet_addr(CONTROL_IP);
	_server.config.port = htons(21);

	if (_server._initialize() > 0) {
		system("pause");
		return -1;
	}
#endif

	config.clear();
	config.close();

	for (;;) {
#if SERVER_TYPE == 0x01 //DEFAULT | DEBUG
		unsigned int err = 0;
		err = _server._connect();
		switch (err) {
		case WSAENOTSOCK:
			break;
		default:
			if (_server.sock_last != INVALID_SOCKET) {
				threads.push_back(std::thread(action, _server.sock_last));
				//action(_server.sock_last);
			}
			break;
		}
		
		/*_server._connect();
		_server.send("connected", 10, 0);
		_server._recv(buf, sizeof(buf), 0);
		SetConsoleColor(TAG);
		printf("[0x%X:data] ", _server.sock_session);
		SetConsoleColor(NONE);
		printf(buf);*/
#endif
#if SERVER_TYPE == 0x02 //HTTP
		int err = 0;
		http_serverconn sc;
		sc = _server.connect(err);
		switch (err) {
		case CNCT_ERR::ERR_BLOCKED:
			break;
		case WSAENOTSOCK:
			break;
		default:
			if (sc.sock != INVALID_SOCKET) {
				threads.push_back(std::thread(action, sc, _server.httpconfig));
				//action(sc, _server.httpconfig);
			}
			break;
		}

#endif
#if SERVER_TYPE == 0x03 //FTP

		_server._connect();

		_server << "220 \r\n";
		SetConsoleColor(TAG);
		printf("[0x%X] ", _server.sock_session);
		SetConsoleColor(NONE);
		printf("Response: ");
		SetConsoleColor(STR);
		printf("\"220\"\n");
		SetConsoleColor(NONE);
		memset(&data, 0x00, sizeof(ftpdata_in));
		data = _server._recv();
		SetConsoleColor(TAG);
		printf("[0x%X:data] ", _server.sock_session);
		SetConsoleColor(NONE);
		printf("%s", data.source);
		_server << "230 \r\n";
		SetConsoleColor(TAG);
		printf("[0x%X] ", _server.sock_session);
		SetConsoleColor(NONE);
		printf("Response: ");
		SetConsoleColor(STR);
		printf("\"230\"\n");
		SetConsoleColor(NONE);

		for (;;) {
			memset(&data, 0x00, sizeof(ftpdata_in));
			data = _server._recv();
			SetConsoleColor(TAG);
			printf("[0x%X:data] ", _server.sock_session);
			SetConsoleColor(NONE);
			printf("%s", data.source);

			if (data.method == FTP_GET_SYSTEM) {
				_server << "215 Windows\r\n";
				SetConsoleColor(TAG);
				printf("[0x%X] ", _server.sock_session);
				SetConsoleColor(NONE);
				printf("Response: ");
				SetConsoleColor(STR);
				printf("\"215 Windows\"\n");
				SetConsoleColor(NONE);
			}

			else {
				_server << "502 \r\n";
				SetConsoleColor(TAG);
				printf("[0x%X] ", _server.sock_session);
				SetConsoleColor(NONE);
				printf("Response: ");
				SetConsoleColor(STR);
				printf("\"502\"\n");
				SetConsoleColor(NONE);
			}
		}

#endif
		CUR_THREAD::sleep_for(std::chrono::milliseconds(125));
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
