#include <iostream>
#include <thread>
#include <mutex>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fstream>

using namespace std;

class logController {
  string filename; //filename
  mutex mAcess; // mutex
  fstream file; // file
  string name; //name of the monitor

  public: 
  logController(string name_);
  ~logController();
  void write_log(string line);
  void read_log();
};

string GetTime(){
  time_t t ; 
  struct tm *tmp ; 
  char MY_TIME[50]; 
  time( &t ); 
  
  tmp = localtime( &t ); 
      
    // using strftime to display time 
  strftime(MY_TIME, sizeof(MY_TIME), "%d-%m-%Y_%X", tmp);
  string time_g(MY_TIME); // transforma em string o char com a data
  return time_g;
}

logController::logController(string name_) {
  name = name_;
  filename = GetTime() + ".txt";
  file.open(filename,ios::out);
}

logController::~logController(){
  file.close();
}

void logController::write_log(string line) {
  mAcess.lock();
  line = GetTime() + " " + line;
  //cout << line << endl;
  file << line << endl;
  mAcess.unlock();
}

void logController::read_log(){
  file.close(); //fecha o arquivo aberto como out
  file.open(filename,ios::in);
  //cout << filename << endl;
  cout << "Content of monitor "+ name << endl;
  string line;
  while(getline(file,line)){
    cout << line << endl;
  }
}


int main() {

  //int x = fork();

  logController s1("1");
  //sleep(2); //so it wont create files with same name;
  

  thread t1(&logController::write_log, &s1, "ABRACADABRA");// usa 5 threads pra escrever em cada
  thread t2(&logController::write_log, &s1, "SINSALABIM");
  thread t3(&logController::write_log, &s1, "OLA");
  thread t4(&logController::write_log, &s1, "LPII é muito legal, mas é difícil");
  thread t5(&logController::write_log, &s1, "Quarentena por mais 15 dias");
  sleep(1); // para não terem o mesmo nome de arquivo
  logController s2("2");
  thread t6(&logController::write_log, &s2, "ABRACADABRA 2");
  thread t7(&logController::write_log, &s2, "SINSALABIM 2");
  thread t8(&logController::write_log, &s2, "Ola 2");
  thread t9(&logController::write_log, &s2, "LPII é muito bom mas em C++ é mais difícil 2");
  thread t10(&logController::write_log, &s2, "Lembrete: programar mais em C++, se não esquece 2");

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  t7.join();
  t8.join();
  t9.join();
  t10.join();
  //cout <<"passou" << endl;
  s1.read_log(); // exibe o conteúdo utilizando os monitores
  s2.read_log();

  
  return 0;



}