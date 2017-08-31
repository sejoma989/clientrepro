#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <cassert>
#include <unordered_map>
#include <fstream>
#include <glob.h>
#include <math.h>

//export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:usr/local/lib

using namespace std;
using namespace zmqpp;

int nparts(string fileName){
	int cut=512*1000;
	//524288
	ifstream ifs(fileName, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();
	int end= ifs.tellg();
	int npart = ceil(end / cut);
	if((pos%cut)!=0){npart++;}
	return npart;
}

vector<char> readFileToBytes(const string& fileName, int part) {
	ifstream ifs(fileName, ios::binary | ios::ate);
	ifstream::pos_type endpos = ifs.tellg();
	int end = ifs.tellg();
	int cut=512*1000;
	int numpart = nparts(fileName);
	int prueba = cut * (part-1);

	//ifs.seekg(0, ios::beg);
	if (endpos<cut) {
		ifs.seekg(0, ios::beg);
		vector<char> result(endpos);
		ifs.read(result.data(),result.size());
		return result;
	}else if (part==1&&(cut>endpos)) {
		ifs.seekg(0, ios::beg);
		vector<char> result(cut);
		ifs.read(result.data(),result.size());
		return result;
	}else if (part == numpart) {
		vector<char> result(end-prueba);
		ifs.seekg(prueba);
		ifs.read(result.data(), result.size() );
		return result;
	} else{
		vector<char> result(cut);
		ifs.seekg(prueba);
		ifs.read(result.data(),result.size());
		return result;
	}
}



void fileToMesage(const string& fileName, message& msg, int part) {
	vector<char> bytes = readFileToBytes(fileName, part);
	msg.add_raw(bytes.data(), bytes.size());
}

// Split all available songs
vector<string> split(string s, char del){
    vector<string> v;
    string nameSong = "";

    for (int i = 0; i < int(s.size()); i++) {
        if (s[i] != del)
            nameSong += s[i];

        else{
            v.push_back(nameSong);
            nameSong = "";
        }
    }

    if (nameSong != "")
        v.push_back(nameSong);

    return v;
}

// Load all files from directory
unordered_map<string,string> fromDirectory(const string& path){
    glob_t glob_result;
    glob(path.c_str(), GLOB_TILDE, NULL, &glob_result);

    vector<string> files;

    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i)
        files.push_back(string(glob_result.gl_pathv[i]));

    globfree(&glob_result);
    unordered_map<string,string> Result;

    for (int i = 0; i < int(files.size()); i++){
        vector<string> vec = split(files[i], '.');

        if (vec[1] == "ogg"){
            vec = split(vec[0], '/');
            Result[vec[1]] = files[i];
        }
    }

    return Result;
}

int main(int argc, char** argv) {
  context ctx;
  socket s(ctx, socket_type::rep);
  s.bind("tcp://*:5555");

  // string path(argv[1]);
  // unordered_map<string,string> songs;
  // songs["s1"] = path + "s1.ogg";
  // songs["s2"] = path + "s2.ogg";
  // songs["s3"] = path + "s3.ogg";
	string path(argv[1]);
	unordered_map<string,string> songs;
	songs = fromDirectory(path + "*");

  cout << "Start serving requests!\n";
  while(true) {
    message m;
    s.receive(m);

    string op;
    m >> op;

    cout << "Action:  " << op << endl;
    if (op == "list") {  // Use case 1: Send the songs
      message n;
      n << songs.size();
      for(const auto& p : songs)
        n << p.first;
      s.send(n);
    } else if(op == "init"){
			string songName;
			int parts;
			m >> songName;
			message n_init;
			parts = nparts(songs[songName]);
			cout << "el numero de partes es:" << parts << endl;
			n_init << parts;
			s.send(n_init);
		}else if(op == "play") {
      // Use case 2: Send song file
      string songName;
			int clientpart;
      m >> songName;
			m >> clientpart;
      cout << "sending song " << songName
           << " at " << songs[songName] << endl;
			message n;
			n << "file";
			fileToMesage(songs[songName], n, clientpart);
			s.send(n);
    } else {
      cout << "Invalid operation requested!!\n";
    }
  }

  cout << "Finished\n";
  return 0;
}
