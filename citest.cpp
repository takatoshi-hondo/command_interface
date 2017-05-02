#include "commandInterface.hpp"

class TestCommand1 : public Command<string>{
public:
  TestCommand1() : Command( "tc1" ){
  }
  string commandFunc( string *ptr , list<string> args ){
    (*cmdlist)["param"]->commandFunc( ptr , makeargs(getarg(0,args),1,2,3) );
    return "";
  }
};

class Plus : public Command<string>{
public:
  Plus() : Command( "+" ){
  }
  string commandFunc( string *ptr , list<string> args ){
    list<string>::iterator it = args.begin();
    double ret = 0;
    while( it != args.end() ){
      ret += stod( *it );
      ++it;
    }
    return tostr(ret);
  }
};

int main( void ){
  TestCommand1 tc1;
  Plus         plus;
  StdinReciever rcv( ">> " );
  StdoutSender snd;
  string buf;
  CommandManager<string> cm( &rcv , &snd , &buf );
  cm.addCommand( &tc1 );
  cm.addCommand( &plus );
  while( 1 ){
    cm.exeCommand();
  }
}
