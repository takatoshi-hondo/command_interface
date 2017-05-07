#include "commandInterface.hpp"
#include <sstream>

list<string> makeargs( list<string> args ){
  return args;
}

void CommandReciever::setLineEnd( char le ){
  line_end = le;
}

StdinReciever::StdinReciever( string p ){
  prompt = p;
  cnt    = 0;
}

string StdinReciever::getLine( void ){
  string line;
  cnt++;
  cout << "[" << cnt << "] " << prompt;
  getline(cin,line,line_end);
  return line;
}

void StdinReciever::returnErr( string ret ){
  cerr << ret << endl;
}

void StdoutSender::returnValue( string ret ){
  cout << ret << endl;
}

MemorySender::MemorySender( string *m ){
  mem = m;
}

void MemorySender::returnValue( string ret ){
  *mem = ret;
}

FileReciever::FileReciever( string name ){
  fname = name;
  ifs.open( fname.c_str() );
  lc = 0;
  iseof = false;
}

string FileReciever::getLine( void ){
  string line;
  lc++;
  if( !getline(ifs,line,line_end) ){
    iseof = true;
    line = "";
  }
  cerr << "[File reciever line " << lc << "] " << line << endl;
  return line;
}

void FileReciever::returnErr( string ret ){
  cerr << "While executing file " << fname << ", line " << lc << endl;
  cerr << ret << endl;
}

bool FileReciever::isEOF( void ){
  return iseof;
}

void FileReciever::close( void ){
  ifs.close();
}
