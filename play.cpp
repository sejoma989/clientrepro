#include<SFML/Audio.hpp>
#include<iostream>

using namespace std;
using namespace sf;

int int main(int argc, char **argv) {
  if(argc !=2){
    cerr << "Must be called:" << argv[0] <<"file.ogg\n";
    return 1;

  }
  cout << "Simple player!\n";
  string fileToplay(argv[1]);

  Music music; //Music es de la libreria sfmlv y crea un hilo para la reproduccion
  music.openFromFile(fileToplay);
  music.play();

  int x;
  cin >> x;
  return 0;

  //if(!music.openFromFIle(fileToplay))


}
