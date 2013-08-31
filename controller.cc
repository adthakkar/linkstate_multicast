/*
* Author: Aditya Thakkar
* Course: Advance Computer Networks
*/

#include <iostream>
#include "node.h"

using namespace std;

/**************************************
* Global Variables Declaration
***************************************/

int       iNetTopology[MAX_NODES][MAX_NODES];
int       iLineNumber[MAX_NODES];
int       iParentNodes[MAX_NODES];

const string sTopFile = "topology";
const string sInFile  = "output_";
const string sOutFile = "input_";

int main ()
{
  std::fstream        file;
  
  if(NONE != popTopology(&file))
  {
    return -1;
  }
  
  while(1)
  {
    for(int i=0; i<MAX_NODES; i++)
    {
      if(!iParentNodes[i])
      {
        continue;
      }
      readFile(i);
    }
    sleep(1);
  }
}

//This function populates topology from a file
errType popTopology(std::fstream* file)
{
  errType ret = NONE;
  int nToken = 0;
  
  if(!file->is_open())
  {
    file->open(sTopFile.c_str(), ios::in);
  } 
  if(!file->good())
  {
    ret=FILE_NOT_FOUND;
    cout<<"DEBUG_controller: File Not Found - "<<sTopFile<<endl;
  }
  else
  {
    while(!file->eof())
    {
      char buf[MAX_CHARS_PER_LINE];
      const char* token[MAX_TOKENS_PER_LINE] = {};
      nToken=0;
      
      file->getline(buf, MAX_CHARS_PER_LINE);

      token[0] = std::strtok(buf, DELIMITER); // first token
      
      if(token[0])
      {
        for (nToken = 1; nToken < MAX_TOKENS_PER_LINE; nToken++)
        {
          token[nToken] = strtok(0, DELIMITER); // subsequent tokens
          if (!token[nToken]) break; // no more tokens
        }
        if(nToken!=2)
        {
          ret=INVALID_FILE_CONTENT;
          cout<<"DEBUG_controller: Invalid File Content; Expected Content: node node"<<endl;
          break;
        }  
        else
        {
          iNetTopology[atoi(token[0])][atoi(token[1])] = 1;
          iParentNodes[atoi(token[0])] = 1;
        }
      }
    }
    file->close();
  }
  
  return ret;
}

//THis function reads output file of a node 
void readFile(int id)
{
  fstream         iStream;
  stringstream    ss;
  string          iFile;
  string          str;
  int             lineCount = 0;
  
  char buf[MAX_CHARS_PER_LINE];
  
  ss<<id;
  iFile=sInFile+ss.str();
  
  iStream.open(iFile.c_str(), ios::in); 
  if(iStream.good())
  {
    while(!!iStream.getline(buf, MAX_CHARS_PER_LINE))
    {
      char* token;
      
      str = buf;
      lineCount++;
      if(lineCount <= iLineNumber[id])
      {
        continue;
      }
      token = std::strtok(buf, DELIMITER); //first token
      writeFile(str, token, id);
    }
    iLineNumber[id] = lineCount;
  }
  iStream.close();
  return;
}

//This function writes messages to the input file of the neighboring node
void writeFile(string buf, char* tok, int id)
{
  fstream        oStream;
  fstream        iStream;
  string         oFile;        
  stringstream   ss;
  string         token(tok);
  string         iFile; 
  
  ss<<id;
  oFile = sOutFile+ss.str();
   
  for(int i=0; i<MAX_NODES; i++)
  {
    if(iNetTopology[id][i])
    {
      ss.str(std::string());
      ss<<i;
      oFile = sOutFile+ss.str();
      iFile = sInFile+ss.str();
      
      iStream.open(iFile.c_str(), ios::in); 
      if(!(iStream.is_open()))
      {
        continue;
      }
      
      iStream.close();           
      oStream.open(oFile.c_str(), ios::out | ios::app);
      if(oStream.good())
      {
        if(token==HELLO_MSG || token==LINK_STATE_MSG || token==JOIN_MSG || token==DATA_MSG)
        {
          oStream<<buf<<endl; 
          oStream.close(); 
        }
      }
    }
  }
  return;
}



