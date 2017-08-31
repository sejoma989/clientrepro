#include <SFML/Audio.hpp>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <zmqpp/zmqpp.hpp>
#include <mutex>
#include <condition_variable>
#include "safequeue.h"

using namespace std;
using namespace zmqpp;
using namespace sf;

// template <class T, class Container = deque<T> >class queue;

void messageToFile(const message &msg, const string &fileName, bool newFile) {
  const void *data;
  msg.get(&data, 1);
  size_t size = msg.size(1);
  if (newFile) {
      ofstream ofs(fileName, ios::binary);
      ofs.write((char *)data, size);
  }else{
  // ofstream ofs(fileName, ios::binary | ios::ate | ios::app);
  // ofs.seekp(0,ios::end);
  // }
  ofstream ofs(fileName, ios::binary | ios::app);
  ofs.write((char *)data, size);
  }
}

void songplay(Music *musicptr, SafeQueue<string> *qsongsptr, socket &s, bool &stop, bool &pause, int part) {
  cout <<endl<< "Inicia hilo songplay" << endl;
	while (!stop) {
    //while (qsongsptr->empty()) {}
    string songtoplay = qsongsptr->dequeue();
    //qsongsptr->pop();
    cout << "va a sonar:" << songtoplay << endl;
    message m;
		m << "init" << songtoplay;
    s.send(m);
    message answer;
    cout << "ya mande el mensaje de partes" << endl<< endl;
    s.receive(answer);
    //cout << "ya recivi el mensaje" << endl;
    int numberpart;
    answer >> numberpart;
    //int nnumberpart = stoi(numberpart);
    cout << "el numero de partas son:" << numberpart << endl;
    while(part <= numberpart){
      cout << "voy a reproducir las partes:" << part << endl<< endl;
      message m2;
      m2 << "play" << songtoplay << part;
      s.send(m2);
      message answer2;
      cout << "le voy a mandar el siguiente mensaje (play,songtoplay,part)" << endl<<endl;
      s.receive(answer2);
      cout << "recibi la parte" << endl;
      if (part==1) {
        messageToFile(answer2, "song.ogg", true);
        cout << "newFile recibi la parte igual 1--" << part << endl;;
      }else{
      //messageToFile(answer2, "song"+to_string(part)+".ogg", false);
      messageToFile(answer2, "song.ogg", false);
      cout << "openFile mande la parte siguiente" << endl<< endl;
      cout << "recibi la parte" << part<< endl;
      }
      part++;
    }
      musicptr->openFromFile("song.ogg");
      cout << "las partes cargadas" << (part-1) << endl<< endl;
      cout << "todas las partes= " << numberpart<< endl;
      cout << "voy a reproducir las partes" << endl;
      musicptr->play();
      while (musicptr->getStatus() == SoundSource::Status::Playing && !stop && !pause) {}
      part=1;
  }
  return;
}

  void equeue(SafeQueue<string> *qsongseptr, string value){
    SafeQueue<string> auxqueue;
    string songtoerase;
    while(!qsongseptr->qempty()){
      songtoerase = qsongseptr->dequeue();
      if (songtoerase != value){
        auxqueue.enqueue(songtoerase);
      }
    }
    while(!auxqueue.qempty()){
      songtoerase = auxqueue.dequeue();
      qsongseptr->enqueue(songtoerase);
    }
  }
/*
void thread_function(songplay,){ //le paso el objeto music y le paso la cola
        while (music.getStatus() == SoundSource::Playing){
                cout << "suena!" << endl;*/

int main(int argc, char **argv) {
  cout << "\nThis is the client\n";

  context ctx;
  socket s(ctx, socket_type::req);
  cout << "Connecting to tcp port 5555\n";
  s.connect("tcp://localhost:5555");

  //std::queue<string> qsongs; // esta cola almacenara el nombre de las acciones
                             // que el usuario agrega
  SafeQueue<string> qsongs;
  Music music;
	bool stop = false;
  bool pause = false;
  int serverpart = 1;
  thread t(songplay, &music, &qsongs, std::ref(s), std::ref(stop), std::ref(pause), serverpart);

  while (true) {
    cout <<endl<<"Operations available:"<<endl<<"list"<<endl<<"add"<<endl<<"play"<<endl<<"next"<<endl<<"stop"<<endl<<"del"<<endl<<"exit"<<endl;
    cout <<endl<<"Operacion?: ";
    string operation;
    cin >> operation;

    message m;
    // m << operation;

    if (operation == "play") {
      cout <<endl<< "Selecciono (play) digite el nombre de la cancion:";
      string file;
      cin >> file;
      qsongs.enqueue(file);
      //cout << "lo que tengo para play es:" << qsongs.front() << endl;
      //cout << "despues de t.join" << endl;
    } else if (operation == "list") {
      cout <<endl<< "Selecciono (list)" << endl;
      cout <<endl<< "Canciones del directorio (music): " <<endl<<endl;
      m << operation;
      s.send(m);
      message answer;
      s.receive(answer);

      size_t numSongs;
      string songL;

      answer >> numSongs;
      cout << "Available songs: " << numSongs << endl;
      for (int i = 0; i < numSongs; i++) {
        answer >> songL;
        cout << "Name song "<<(i+1)<<": "<< songL << endl;

      }
    } else if (operation == "add") {
      cout <<endl<< "Selecciono (add) digite el nombre de la cancion: ";
      string file;
      cin >> file;
      //cout << "si funciona" << qsongs.qempty() << endl;
      qsongs.enqueue(file);
      //cout << "si funciona2" << qsongs.qempty() << endl;
      /*
      int size;
      size = qsongs.sizequeue(qsongs);
      cout << "el tamaño de qsongs es" << size << endl;
      */
      //cout << "lo que tengo para add es:" << qsongs.front() << endl;
    }else if(operation == "del"){
      cout <<endl<< "Selecciono (del) digite el nombre de la cancion que desea borrar:";
      string element;
      cin >> element;
      equeue(&qsongs, element);
      cout <<endl<< "La cancion se elimino correctamente" << endl;

    } else if (operation == "stop") {
			stop = true;
      t.join();
      return 0;
    }else if(operation == "next"){
      pause = true;
    }else if (operation == "exit") {
      break;
    } else {
      cout << "Don't know what to do!!!" << endl;
    }
    /*
    if(operation !="add" || "list" || "play" ) {
                    cout << "Don't know what to do!!!" << endl;
            }*/
  }
  return 0;
}
