#include "../CastDoc/includes.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <queue>
#include <algorithm>


std::unordered_map<std::string, std::vector<std::string>> types_map;

char current_dir[1024];
std::string files_dir;

/* portul folosit */
#define PORT 2908

void raspunde(int sd, int id);
void make_map();
int get_cast_path(const char* type1, const char* type2, std::vector<std::string>& cast_path);

int main()
{
	struct sockaddr_in server; // structura folosita de server
	struct sockaddr_in from;
	int nr; // mesajul primit de trimis la client
	int sd; // descriptorul de socket
	int pid;
	
	int id = 0;

	make_map();

	getcwd(current_dir, sizeof(current_dir));

	/* crearea unui socket */
	ASSERT((sd = socket(AF_INET, SOCK_STREAM, 0)) != -1, "[server]Eroare la socket().\n");


	/* utilizarea optiunii SO_REUSEADDR */
	int on = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* pregatirea structurilor de date */
	memset(&server, 0, sizeof(server));
	memset(&from, 0, sizeof(from));

	/* umplem structura folosita de server */
	/* stabilirea familiei de socket-uri */
	server.sin_family = AF_INET;

	/* acceptam orice adresa */
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/* utilizam un port utilizator */
	server.sin_port = htons(PORT);

	/* atasam socketul */
	ASSERT(bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != -1, "[server]Eroare la bind().\n");


	/* punem serverul sa asculte daca vin clienti sa se conecteze */
	ASSERT(listen(sd, 2) != -1, "[server]Eroare la listen().\n");


	while (1)
	{
		int client;
		int length = sizeof(from);

		printf("[server]Asteptam la portul %d...\n", PORT);
		fflush(stdout);


		if ((client = accept(sd, (struct sockaddr *)&from, (socklen_t*)&length)) < 0)
		{
			perror("[server]Eroare la accept().\n");
			continue;
		}


		int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} 
		else if (pid > 0) {
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
			++id;
    		continue;
    	} 
		else if (pid == 0) {
			files_dir = std::string(current_dir) + "/files";
			files_dir += std::to_string(id);

			printf("[Proces-%d]: Asteptam mesajul...\n", id);
			fflush(stdout);


			raspunde(client, id);


			close(client);

			chdir(current_dir);
			if(access(files_dir.c_str(), F_OK) != -1){
				char comm[256];
				sprintf(comm, "rm -r '%s'", files_dir.c_str());
				system(comm);
			}

			return 0;
		}

	}
}


void make_map()
{
	int fd;
	if((fd = open("ConfigFile", O_RDONLY)) == -1){
		fprintf(stderr, "Nu sa putut deschide fisierul ConfigFile!!");
        return;
	}

	int bytes = 0;
	char line[1000] = "";
	char* conv = NULL;
	std::string type1;
	std::string type2;

	while(1){
		memset(line, 0, 1000);

		bytes = read_line(fd, line);

		if(bytes <= 0){
			close(fd);
			return;
		}

		line[bytes] = '\0';

		conv = strtok(line, " ");
		type1 = strtok(conv, "-");
		type2 = strtok(NULL, "-");

		if(types_map.find(type1) == types_map.end())
			types_map[type1] = {type2};
		else
			types_map[type1].push_back(type2);
	}

	close(fd);
}

int get_cast_path(const char* type1, const char* type2, std::vector<std::string>& cast_path)
{
	std::vector<std::string> visited;
	std::queue<std::string> queue;
	queue.push(type1);

	std::unordered_map<std::string, std::string> parent;
	parent[type1] = "root";

	visited.push_back(type1);

	int found = -1;

	while (!queue.empty()) {
		std::string current_type = queue.front();
		queue.pop();

		if (current_type == type2){
			found = 0;
			break;
		}
		else {
			auto to_types = types_map[current_type];
			for(int i = 0; i < to_types.size(); ++i) {
				if (std::find_if(visited.begin(), visited.end(), [&](const std::string& str) { return str == to_types[i]; } ) == visited.end()){
					queue.push(to_types[i]);
					parent[to_types[i]] = current_type;
					visited.push_back(to_types[i]);
				}
			}
		}
	}

	if(found == -1)
		return -1;

	std::string current_type = type2;

	while (current_type != "root") {
		cast_path.push_back(current_type);
		current_type = parent[current_type];
	}

	return 0;
}

int verify_hash(const char* hash)
{
	return access(hash, F_OK);
}

int rename_file(const char* file_name, const char* new_name)
{
	char command[1000];
	sprintf(command, "mv '%s' '%s'", file_name, new_name);
	system(command);

	return 0;
}

int cast(const char* utility, const char* file_input, const char* file_output, char* hash_output)
{
	char com[1000] = "";

	if(strcmp(utility, "ffmpeg") == 0 && strstr(file_input, ".mp4") != NULL && strstr(file_output, ".mp3") != NULL)
		sprintf(com, "ffmpeg -i '%s' -vn -acodec libmp3lame -ac 2 -ab 160k -ar 48000 '%s' > /dev/null 2>&1", file_input, file_output);
	else if(strcmp(utility, "ffmpeg") == 0)
		sprintf(com, "ffmpeg -i '%s' -codec copy '%s' > /dev/null 2>&1", file_input, file_output);
	else if(strcmp(utility, "pandoc") == 0)
		sprintf(com, "%s '%s' -o '%s' > /dev/null 2>&1", utility, file_input, file_output);
	else if(strcmp(utility, "gs") == 0 && strstr(file_output, ".jpg"))
		sprintf(com, "gs -sDEVICE=jpeg -o '%s' '%s' > /dev/null 2>&1", file_output, file_input);
	else if(strcmp(utility, "gs") == 0 && strstr(file_output, ".png"))
		sprintf(com, "gs -sDEVICE=pngalpha -o '%s' '%s' > /dev/null 2>&1", file_output, file_input);
	else
		sprintf(com, "%s '%s' '%s' > /dev/null 2>&1", utility, file_input, file_output);

	system(com);

	sleep(0);
	wait(NULL);

	char hash[500];
	get_hash(file_output, hash);

	rename_file(file_output, hash);
	strcpy(hash_output, hash);

	return 0;
}

int get_new_file_name(const char* file_hash, const char* extension, char* new_name)
{
	char hash[1000];
	strcpy(hash, file_hash);

	char* name1 = strtok(hash, " ");
	name1 = strtok(NULL, " ");
	char* name2 = strtok(name1, ".");

	sprintf(new_name, "%s.%s", name2, extension);

	return 0;
}


int get_utility(const char* type1, const char* type2, char* utility)
{
	char types[100];
	sprintf(types, "%s-%s", type1, type2);

	int fd;
	if((fd = open("ConfigFile", O_RDONLY)) == -1){
		fprintf(stderr, "Nu sa putut deschide fisierul ConfigFile!!");
        return -1;
	}

	int bytes = 0;
	char line[1000] = "";

	while(1){
		memset(line, 0, 1000);

		bytes = read_line(fd, line);

		if(bytes <= 0){
			close(fd);
			return -1;
		}

		line[bytes] = '\0';

		if(strstr(line, types) != NULL){
			break;
		}
	}

	close(fd);


	char* u = strtok(line, " ");
	u = strtok(NULL, " ");

	strcpy(utility, u);

	return 0;
}

void raspunde(int sd, int id)
{
	std::string files_dir = std::string(current_dir) + "/files";
	files_dir += std::to_string(id);

	if(access(files_dir.c_str(), F_OK) == -1)
		mkdir(files_dir.c_str(), 0755);
	chdir(files_dir.c_str());

	char type1[50];
	char type2[50];
	char utility[50];

	int bytes = 0;

	int r = -1;
	read(sd, &r, 4);
	while(r != -1)
	{
		bytes = 0;

		read(sd, &bytes, 4);
		memset(type1, 0, 50);
		int rez = read(sd, type1, bytes);
		if(rez == -1)
			return;

		bytes = 0;
		read(sd, &bytes, 4);
		memset(type2, 0, 50);
		rez = read(sd, type2, bytes);
		if(rez == -1)
			return;


		printf("[Proces-%d]: Cast from:%s to:%s\n", id, type1, type2);

		std::vector<std::string> cast_path;
		rez = get_cast_path(type1, type2, cast_path);

		//rez = get_utility(type1, type2, utility);
		write(sd, &rez, 4);

		if(rez == -1){
			r = -1;
			read(sd, &r, 4);
			continue;
		}

		std::cout << "[Proces-"<< id << "]: Cast path: ";
		for(auto t = cast_path.rbegin(); t != cast_path.rend(); ++t)
			std::cout << *t << "->";
		std::cout << "\n";

		std::vector<std::string> utility_path;

		chdir(current_dir);
		for(int i = 0; i < cast_path.size() - 1; ++i){
			get_utility(cast_path[i+1].c_str(), cast_path[i].c_str(), utility);
			utility_path.push_back(utility);
		}
		chdir(files_dir.c_str());
		
		rez = 0;
		read(sd, &rez, 4);

		if(rez == -1){
			r = -1;
			read(sd, &r, 4);
			continue;
		}

		rez = 0;
		read(sd, &rez, 4);

		if(rez == -1){
			r = -1;
			read(sd, &r, 4);
			continue;
		}

		char file_hash[1000];
		memset(file_hash, 0, 1000);

		if(read(sd, file_hash, 1000) == -1){
			return;
		}

		rez = 0;
		rez = verify_hash(file_hash);
		write(sd, &rez, 4);

		if(rez == -1)
			receive_file(sd, file_hash);


		char cast_file_name[255];
		char cast_file_hash[500];

		strcpy(cast_file_hash, file_hash);


		for(int i = utility_path.size() - 1; i >= 0; --i){
			memset(file_hash, 0, 1000);
			strcpy(file_hash, cast_file_hash);

			get_new_file_name(file_hash, cast_path[i].c_str(), cast_file_name);

			if(verify_hash(file_hash) == -1){
				--i;
				continue;
			}

			cast(utility_path[i].c_str(), file_hash, cast_file_name, cast_file_hash);
		}

		sleep(3);
		
		send_file(sd, cast_file_hash, cast_file_name);

		r = -1;
		read(sd, &r, 4);
	}
}

