#ifndef __COMMAND_INTERFACE_HPP__
#define __COMMAND_INTERFACE_HPP__

#include <string>
#include <iostream>
#include <map>
#include <list>
#include <sstream>
#include <fstream>
#include <cstdlib>

using namespace std;

template <class T>
string tostr( T val ){
  stringstream ss;
  ss << val;
  return ss.str();
}

list<string> makeargs( list<string> );

template <class First, class... Rest>
list<string> makeargs( list<string> args , First first , Rest... rest ){
  args.push_back( tostr(first) );
  return makeargs( args , rest... );
}

template <class First, class... Rest>
list<string> makeargs( First first , Rest... rest ){
  list<string> args;
  args.push_back( tostr(first) );
  return makeargs( args , rest... );
}

template <class T>
class Command {
private:
  string cmdname;
protected:
  map<string, Command<T> *> *cmdlist;
public:
  Command( string name ){
    cmdname = name;
  }
  string getCommandName( void ){
    return cmdname;
  }
  virtual string commandFunc( T * , list<string> ) = 0;
  void setCommandList( map<string, Command<T> *> *cl ){
    cmdlist = cl;
  }
  string getarg( int n , list<string> args ){
    list<string>::iterator it = args.begin();
    int i = 0;
    string ret;
    while( it != args.end() ){
      if( i == n ){
	ret = *it;
	break;
      }
      ++it;
      i++;
    }
    if( it == args.end() ){
      ret = "";
    }
    return ret;
  }
};

class CommandReciever{
protected:
  char line_end;
public:
  void setLineEnd( char );
  virtual string getLine( void ) = 0;
  virtual void returnErr( string ) = 0;
};

class CommandSender{
public:
  virtual void returnValue( string ) = 0;
};

class MemorySender : public CommandSender{
private:
  string *mem; 
public:
  MemorySender( string * );
  void returnValue( string );
};

class FileReciever : public CommandReciever{
private:
  ifstream ifs;
  string   fname;
  int      lc;
  bool     iseof;
public:
  FileReciever( string );
  string getLine( void );
  void   returnErr( string );
  bool   isEOF( void );
};

//Default commands
template <class T>
class Quit : public Command<T>{
public:
  Quit() : Command<T>( "quit" ){
  }
  string commandFunc( T *ptr , list<string> args ){
    exit(0);
  }
};

template <class T>
class Params : public Command<T>{
private:
  map<string,string> params;
public:
  Params() : Command<T>( "param" ){
  }
  string commandFunc( T *ptr , list<string> args ){ //pname val
    list<string>::iterator it = args.begin();
    string pname , val;
    string ret;
    if( args.size() > 0 ){
      pname = *it;
    }
    if( args.size() > 1 ){
      ++it;
      val = *it;
      params[pname] = val;
      ret = pname;
    }else if( args.size() == 1 ){
      ret = params[pname];
    }else{
      ret = "";
      for( auto mit = params.begin() ; mit != params.end() ; ++mit ){
	ret += mit->first;
	ret += ":";
	ret += mit->second;
	ret += "\n";
      }
    }
    return ret;
  }
};

template <class T>
class CommandManager{
private:
  CommandReciever         *cr;
  CommandSender           *cs;
  map<string,Command<T> *> cmdlist;
  char                     dlm , esc , line_start , line_end , comment , varident;
  char                     func_start , func_end;
  T                       *ptr;
  //default commands
  Quit<T>   q;
  Params<T> sp;
  
  void addDefaultCommands( void ){
    addCommand( &q );
    addCommand( &sp );
    addCommand( &ef );
  }
public:
  CommandManager(){
    addDefaultCommands();
  }
  CommandManager( CommandReciever *r , CommandSender *s , T *p ){
    cr = r;
    cs = s;
    dlm = ' ';
    line_start = 0;
    line_end   = '\n';
    esc = '\\';
    cr->setLineEnd( line_end );
    ptr = p;
    comment = '#';
    varident = '$';
    func_start = '(';
    func_end   = ')';
    addDefaultCommands();
  }
  CommandManager( CommandReciever *r , CommandSender *s , T *p , 
				char d , char e , char ls , char le ){
    cr = r;
    cs = s;
    dlm = d;
    line_start = ls;
    line_end   = le;
    esc = e;
    cr->setLineEnd( line_end );
    ptr = p;
    comment = '#';
    varident = '$';
    func_start = '(';
    func_end   = ')';
    addDefaultCommands();
  }
  void addCommand( Command<T> *c ){
    cmdlist[c->getCommandName()] = c;
    c->setCommandList( &cmdlist );
  }
  CommandReciever *getCurrentReciever( void ){
    return cr;
  }
  void resetReciever( CommandReciever *r ){
    cr = r;
  }
  CommandSender *getCurrentSender( void ){
    return cs;
  }
  void resetSender( CommandReciever *s ){
    cs = s;
  }
  bool isCommentLine( string val ){
    bool ret;
    if( val.c_str()[0] == comment ){
      ret = true;
    }else{
      ret = false;
    }
    return ret;
  }
  bool isVariable( string val ){
    bool ret;
    if( val.c_str()[0] == varident ){
      ret = true;
    }else{
      ret = false;
    }
    return ret;
  }
  string findVariable( string val ){
    string ret , buf;
    if( val.c_str()[0] == varident ){
      stringstream ss(val);
      getline(ss,buf,varident);
      getline(ss,buf,varident);
      ret = cmdlist["param"]->commandFunc(ptr,makeargs(buf));
    }else{
      ret = val;
    }
    return ret;
  }
  void exeCommand( void ){
    string buf , cmdname;
    list<string> args;
    string line , ret;
    
    line = cr->getLine();
    if( line.length() > 0 ){
      stringstream ss(line);
      while( getline(ss,buf,dlm) ){
	args.push_back( buf );
      }
      cmdname = args.front();
      if( !isCommentLine( cmdname ) ){
	args.pop_front();
	exeCommand( cmdname , args );
      }
    }
  }
  void exeCommand( string cmdname , list<string> args ){
    exeCommand( cmdname , args , cs );
  }
  void exeCommand( string cmdname , list<string> args , CommandSender *cms ){
    string ret;
    list<string>::iterator it = args.begin();
    if( isVariable( cmdname ) ){
      cms->returnValue(findVariable(cmdname));
    }else if( cmdlist[cmdname] ){
      while( it != args.end() ){
	if( isVariable( *it ) ){
	  *it = findVariable( *it );
	}
	++it;
      }
      ret = cmdlist[cmdname]->commandFunc( ptr , args );
      cms->returnValue( ret );
    }else{
      cr->returnErr( cmdname + ": Command not found" );
    }
  }
  Command<T> *getCommandObject( string cmdname ){
    return cmdlist[cmdname];
  }
};

class StdinReciever : public CommandReciever{
private:
  string prompt;
  int    cnt;
public:
  StdinReciever( string );
  string getLine( void );
  void returnErr( string );
};

class StdoutSender : public CommandSender{
public:
  void returnValue( string );
};

#endif
