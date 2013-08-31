/*
* Author: Aditya Thakkar
* Course: Advance Computer Networks
*/

#include <algorithm> 
#include <string>
#include <cctype> 
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <vector>

/**************************************
* CONST VARIABLES
***************************************/
const std::string HELLO_MSG          = "hello";
const std::string JOIN_MSG           = "join";
const std::string LINK_STATE_MSG     = "linkstate";
const std::string DATA_MSG           = "data";

const int MAX_CHARS_PER_LINE         = 512;
const int MAX_TOKENS_PER_LINE        = 20;
const int MAX_FINISH_TIME            = 150;
const int MAX_NODES                  = 10;

const char* const DELIMITER          = " "; 

/**************************************
* Structure Declaration
***************************************/
struct userInputs
{
  int           iID;			  // ID of the node
  int           iSenderID;		  // ID of the sender from whom this node wants to receive multicast data
  bool          bSender;		  // This is sender node if True
  bool          bReceiver;		  // This is receiver node if True
  std::string   sData;			  // Multicast Data
};

struct statRecord
{
  int           iTimeStamp;       //time stamp recorder
  int           iInLineCount;     //line count for input file  
  int           iRecLineCount;    //line count for received multicast data         
};

struct fileDB
{
  std::string   sInFile;
  std::string   sOutFile;
  std::string   sRecFile;
};

struct joinPacket
{
  int iRecTS;
  int iJoinTS;
  int iChildID;
  int iParentID;
  
  joinPacket()
  {
    iRecTS = 0;
    iJoinTS = 0;
    iChildID = -1;
    iParentID = -1;
  }  
};

/**************************************
* Enum Declaration
***************************************/
enum errType
{
  NONE,
  INVALID_COMMAND,
  INVALID_PARSE_TYPE,
  FILE_NOT_FOUND,
  INVALID_FILE_CONTENT
};

enum eParseTypes
{
  INPUT_FILE,
  RECEIVE_FILE,
};

/**************************************
* Function Declaration node.cc
***************************************/
void printError(errType errType, std::string str);
errType saveUserInfo(int argc, char* argv[]);
void initializeSystem(std::fstream* oFile, std::fstream* iFile, std::fstream* rFile);
void sendHello(std::fstream* oFile);
void parseFile(std::fstream* file, eParseTypes pType);
void parseLine(const char* line[], int size);
void sendLSA(std::fstream* oFile);
void removeExpiredLSA();
std::vector<int> dijkstraSPT(int src, int dest);
int minDistance(int dist[], bool sptSet[]);
void sendJoin(std::fstream* oFile);
std::vector<int> computeJoinPath(int src, int dest, int* parent);
void removeExpiredJoin();
void sendData(std::fstream* oFile);

/**************************************
* Function Declaration controller.cc
***************************************/
errType popTopology(std::fstream* file);
void readFile(int id);
void writeFile(std::string buf, char* tok, int id);
