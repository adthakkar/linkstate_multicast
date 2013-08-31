/*
* Author: Aditya Thakkar
* Course: Advance Computer Networks
* Data: May 11, 2013
*/

#include <iostream>
#include <stdlib.h> 
#include <stdio.h>
#include <map>
#include <list>
#include "node.h"

/**********************************************************
* Lessons Learned
* - It is important to close file
    for each file open session.
* - Use string stream to convert int to
    string
* - Do NOT use itoa to convert int to string
    as it is not good way in c++
***********************************************************/

/**************************************
* Global Variables Declaration
***************************************/
userInputs            sInputs;
statRecord            sRecord;
fileDB                sFNames;

std::map<int,int>     mNeighbors;
std::map<int,int>     mLSA;
std::map<int,int>     mLSALifeTime;
std::list<joinPacket> lSourceList[MAX_NODES];

int                   iTopology[MAX_NODES][MAX_NODES];
bool                  bLDBChanged = false;

using namespace std;

int main (int argc, char* argv[])
{
  std::fstream        outFile;
  std::fstream        inFile;
  std::fstream        recFile; 
  
  //TODO: Error Checking NOT working
  
  //Initial Error Checking.
  /*if(argc>4 || (atoi(argv[1]) <0) || (atoi(argv[1]) > 9))
  {
    printError(INVALID_COMMAND, "Arguments > 4 or ID out of Range ID=");
    return -1;
  }
  if(argc>2)
  {
    string tmp(argv[2]);
    std::transform(tmp.begin(), tmp.end(), tmp.begin(),(int (*)(int))tolower);

    if((tmp == "receiver") && (((atoi(argv[3]) <0) || (atoi(argv[3]) > 9))))
    {
      printError(INVALID_COMMAND, "Arguments != 4 or ID out of range");
      return -1;
    }
    else if((tmp == "sender") && ((atoi(argv[3])) == (atoi(argv[1]))))
    {
      printError(INVALID_COMMAND, "Arguments != 4, ID(s) out of range or SenderID = ID");
      return -1;      
    }
  }*/
  
  //save user inputs
  if(NONE != saveUserInfo(argc, argv))
  {
    return -1;
  }
  
  //initialize the system i.e. create files
  initializeSystem(&outFile, &inFile, &recFile);
  
  for(int i=0; i<MAX_FINISH_TIME; i++)
  {
    sendHello(&outFile);                    //send hello message  
    sendLSA(&outFile);                      //send linkstate message
    sendJoin(&outFile);                     //send Join msg. This takes care of refresh Join 
    sendData(&outFile);                     //send data if it is the sender    
        
    removeExpiredLSA();                     //remove stale linkstate records 
    removeExpiredJoin();                    //remove stale join records  
    
    parseFile(&inFile, INPUT_FILE);         //read input file for latest updates.
    if(sInputs.bReceiver)
    {
      parseFile(&recFile, RECEIVE_FILE);    //handle receive file 
    }
    
    sleep(1);
    sRecord.iTimeStamp++;
    
    if(!(sRecord.iTimeStamp % 15))
    {
      cout<<"Node "<<sInputs.iID<<" Elapsed Time="<<sRecord.iTimeStamp<<endl;
    }
  }
  
  cout<<"Success ID="<<sInputs.iID<<endl;
  
  return 0;
}

void printError(errType errType, std::string str)
{
  switch(errType)
  {
    case NONE:
      break;
    case INVALID_COMMAND:
      cout<<"Invalid Command - "<<str<<endl;
      cout<<"Following Commands accepted:"<<endl;
      cout<<" node <ID>                         --ID is in range from 0-9"<<endl;
      cout<<" node <ID> sender <multicast data> --ID is in range from 0-9"<<endl;
      cout<<" node <ID> receiver <sender ID>    --ID is in range from 0-9; ID != <sender ID>"<<endl;
      break;
    case INVALID_PARSE_TYPE:
      cout<<"DEUG: Invalid Parse Type "<<str<<endl;
      break;
    case FILE_NOT_FOUND:
      cout<<"DEBUG: File Not Found - "<<str<<endl;
      break;
    default:
      break; 
  }
  
  return;
}

errType saveUserInfo(int argc, char* argv[])
{
  errType ret = NONE;
  string cmd = "";
  stringstream ss;
  
  sInputs.iID = atoi(argv[1]);
  ss<<(sInputs.iID);
    
  sFNames.sInFile="input_"+ ss.str();
  sFNames.sOutFile="output_"+ ss.str();
  
  switch(argc)
  {
    case 2:
      break;
    case 4:
      cmd = argv[2];
      std::transform(cmd.begin(), cmd.end(), cmd.begin(),(int (*)(int))tolower);
      if(cmd == "sender")
      {
        sInputs.bSender = true;
        sInputs.sData = argv[3];
      }
      else if(cmd == "receiver")
      {
        sInputs.bReceiver = true;
        sInputs.iSenderID = atoi(argv[3]);
        
        sFNames.sRecFile=ss.str()+"_received_from_";
        ss.str(std::string()); //clear contents
        ss<<sInputs.iSenderID;
        sFNames.sRecFile +=ss.str();
      }
      break;
    default:
      printError(INVALID_COMMAND, "");
      break;
  }
  
  //cout<<"DEBUG: ID="<<sInputs.iID<<" bSender="<<sInputs.bSender<<" bReceiver="<<sInputs.bReceiver<<
  //      " iSenderID="<<sInputs.iSenderID<<" sData="<<sInputs.sData<<endl;
  
  return ret;
}

void initializeSystem(std::fstream* oFile, std::fstream* iFile, std::fstream* rFile)
{
  //create files
  oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
  iFile->open(sFNames.sInFile.c_str(), ios::in);
  
  if(sInputs.bReceiver)
  {
    rFile->open(sFNames.sRecFile.c_str(), ios::in);
    rFile->close();
  }
  
  oFile->close();
  iFile->close(); 
  //cout<<"DEBUG: "<<sFNames.sInFile<<" "<<sFNames.sOutFile<<" "<<sFNames.sRecFile<<endl;
}

void sendHello(std::fstream* oFile)
{
  stringstream ss;
  ss<<sInputs.iID;
  
  if(sRecord.iTimeStamp % 5)
  {
    //We send hello every 5 second interval!
    return;
  }
  
  if(!oFile->is_open())
  {
    oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
  }
  (*oFile)<<HELLO_MSG+" "+ss.str()<<endl;
  oFile->close();
}

void sendLSA(std::fstream* oFile)
{
  string                      lsaMsg = LINK_STATE_MSG + " ";
  stringstream                ss;
  map<int,int>::iterator      it;
  map<int,int>::iterator      it2;
  
  ss<<sInputs.iID<<" "<<sRecord.iTimeStamp;  
  
  if(sRecord.iTimeStamp % 10 || mNeighbors.empty())
  {
    //We send LSA every 10 second interval!
    //Also don't send LSA if no neighbors
    return;
  }
  
  for(int i = 0; i<MAX_NODES; i++)
  {
    iTopology[i][sInputs.iID]=0;
  }
    
  for(it=mNeighbors.begin(); it != mNeighbors.end(); ++it)
  {
    ss<<" "<<it->first;
    iTopology[it->first][sInputs.iID]=1;
  }
  
  lsaMsg += ss.str();
  
  if(!oFile->is_open())
  {
    oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
  }
  (*oFile)<<lsaMsg<<endl;
  oFile->close();
  
  //cout<<"DEBUG lsaMSG: "<<lsaMsg<<endl;
  return;
}

void parseFile(std::fstream* file, eParseTypes pType)
{
  int         nToken=0;
  int         lineCount=0; 
  char        buf[MAX_CHARS_PER_LINE];
         
  
  switch(pType)
  {
    case INPUT_FILE:
      if(!file->is_open())
      {
        file->open(sFNames.sInFile.c_str(), ios::in);
      }
      if(!file->good())
      {
        break;
      }
      while(!!file->getline(buf, MAX_CHARS_PER_LINE))
      {
        const char* token[MAX_TOKENS_PER_LINE] = {};
        nToken=0;
        
        //file->getline(buf, MAX_CHARS_PER_LINE);
        lineCount++;
        if(lineCount <= sRecord.iInLineCount)
        {
          continue;
        } 
        token[0] = std::strtok(buf, DELIMITER); // first token
        
        if(token[0])
        {
          for (nToken = 1; nToken < MAX_TOKENS_PER_LINE; nToken++)
          {
            token[nToken] = strtok(0, DELIMITER); // subsequent tokens
            if (!token[nToken]) break; // no more tokens
          }
          //cout<<"DEBUG: numToken="<<nToken<<endl;
          parseLine(token, nToken);
        }
      }
      file->close();
      //cout<<lineCount<<endl;
      sRecord.iInLineCount = lineCount;
      break;
    case RECEIVE_FILE:
      break;
    default:
      break;
  }
  return;
}

void parseLine(const char* line[], int size)
{
  stringstream                ss;
  std::map<int,int>::iterator it;
  int                         id = atoi(line[1]);
  fstream                     oStream;
  
  //for LSA
  int                         ts;
  int                         oldLSA[MAX_NODES];
 
  //for join and data
  std::vector<int>                vSp;
  std::list<joinPacket>::iterator jit;
  std::vector<int>::iterator      vit;
  joinPacket                      sJoinNode;
  int                             srcId;
  string                          data;

  ss << line[0];
  if(ss.str() == HELLO_MSG)
  {
    it=mNeighbors.find(id);
    if(it == mNeighbors.end())
    {
      mNeighbors.insert(std::pair<int,int>(id, sRecord.iTimeStamp));
      //cout<<"DEBUG_parseLine it!=: "<<id<<" "<<sRecord.iTimeStamp<<endl;
    }
    else if(sRecord.iTimeStamp > it->second)
    {
      mNeighbors.erase(it);
      mNeighbors.insert(std::pair<int,int>(id, sRecord.iTimeStamp));
      //cout<<"DEBUG_parseLine it==: "<<id<<" "<<sRecord.iTimeStamp<<endl;
    }
  }
  else if(ss.str() == LINK_STATE_MSG)
  {
    ts = atoi(line[2]);
    it = mLSA.find(id);

    for(int i=1; i<=size; i++)
    {
      ss<<" "<<line[i];
    }
    
    //if this is not the latest LSA, stop.
    if((it != mLSA.end() && (it->second)>=ts) || (id == sInputs.iID))
    {
      return;
    }
    else if(it != mLSA.end() && (it->second)<ts)
    {
      mLSA.erase(it);
      
      it = mLSALifeTime.find(id);
      if(it != mLSALifeTime.end())
      {
        mLSALifeTime.erase(it);
      }
      else
      {
        cout<<"DEBUG: mLSALifeTime("<<id<<") NOT found"<<endl;
      }
    }

    //track the timings
    mLSA.insert(std::pair<int,int>(id, ts));
    mLSALifeTime.insert(std::pair<int,int>(id, sRecord.iTimeStamp));
    //cout<<id<<" "<<sRecord.iTimeStamp<<endl;
    
    //forward the message to other neighbors regardless of timestamp
    oStream.open(sFNames.sOutFile.c_str(), ios::out | ios::app);
    oStream<<ss.str()<<endl;
    oStream.close();
    
    //clear the old LSA info
    for(int i=0; i<MAX_NODES; i++)
    {
      oldLSA[i] = iTopology[i][id];
      iTopology[i][id] = 0;
    }
 
    //update the topology with new LSA info
    for(int i=3; i<size; i++)
    {
      //cout<<atoi(line[i])<<endl;
      iTopology[atoi(line[i])][id] = 1;
    }
    
    //check if LSA DB has changed
    for(int i=0; i<MAX_NODES; i++)
    {
      if(oldLSA[i] != iTopology[i][id])
      {
        bLDBChanged = true;
        break;
      }
    }
    
  }
  else if(ss.str() == JOIN_MSG)
  {
    //There are 3 cases to handle
    srcId = atoi(line[2]);
           
    if((size == 4) && (atoi(line[3]) == sInputs.iID))
    {      
      //Case 1: Sender receives join
      if((sInputs.bSender) && (srcId == sInputs.iID))
      {
        if(!lSourceList[srcId].empty())
        {
          for(jit = lSourceList[srcId].begin(); jit != lSourceList[srcId].end(); ++jit)
          {
            if(jit->iChildID == id)
            {
              jit = lSourceList[srcId].erase(jit);
            }
          }
        }
        
        sJoinNode.iRecTS = sRecord.iTimeStamp;
        sJoinNode.iJoinTS = 0;
        sJoinNode.iChildID = id;
        sJoinNode.iParentID = -1;
        lSourceList[srcId].push_back(sJoinNode);
        //cout<<"DEBUG: This is case 1 cid="<<sJoinNode.iChildID<<" pid="<<sJoinNode.iParentID<<endl;
      }
      else if(id != sInputs.iID)
      {
        //Case 2: This is the parent
        //add to child list, compute dijkstra to find parent, send join
        id = atoi(line[1]);
        if(!(lSourceList[srcId].empty()))
        { 
          for(jit = lSourceList[srcId].begin(); jit != lSourceList[srcId].end(); ++jit)
          {
            if(jit->iChildID == id)
            {
              jit = lSourceList[srcId].erase(jit);
            }
          }
        }
                       
        sJoinNode.iRecTS = sRecord.iTimeStamp;
        sJoinNode.iJoinTS = 0;
        sJoinNode.iChildID = id;
        sJoinNode.iParentID = 600;              //put an arbitrary value to initialize          
        lSourceList[srcId].push_back(sJoinNode);
                   
        sendJoin(&oStream);
        
        
        /*if(srcId == 1 && sInputs.iID==3)
        {
          cout<<line[0]<<" "<<id<<" "<<srcId<<" "<<atoi(line[3])<<endl;
          for(jit = lSourceList[srcId].begin(); jit != lSourceList[srcId].end(); ++jit)
          {
            cout<<"DEBUG - CASE 2: childID="<<jit->iChildID<<" pId="<<jit->iParentID<<" root="<<srcId
            <<endl<<" recTS="<<jit->iRecTS<<" joinTS="<<jit->iJoinTS<<endl;
          }
          cout<<endl<<endl;
        }*/
               
        //cout<<"DEBUG - CASE 2: childID="<<sJoinNode.iChildID<<" thisID="<<sInputs.iID<<" pId="<<sJoinNode.iParentID<<" root="<<srcId
        //    <<endl<<" recTS="<<sJoinNode.iRecTS<<" joinTS="<<sJoinNode.iJoinTS<<endl;
      }
    }
    else if((size > 4) && (atoi(line[4]) == sInputs.iID))
    {
      //Case 3: This is intermmediate node. Forward the join
      ss<<" "<<line[1]<<" "<<line[2]<<" "<<line[3];
      if(size>5)
      {
        for(int i=5; i<size; i++)
        {
          ss<<" "<<line[i];
        }
      }
      
      //forward the join message to neighbors
      oStream.open(sFNames.sOutFile.c_str(), ios::out | ios::app);
      oStream<<ss.str()<<endl;
      oStream.close();
    }
  }
  else if(ss.str() == DATA_MSG)
  {
    srcId = atoi(line[2]);
    
    if(size > 4)
    {
      ss.str(std::string());
      ss<<line[3];
      for(int i=4; i<size; i++)
      {
        ss<<" "<<line[i];
      }
      data = ss.str();
    }
    
    ss.str(std::string());
    ss<<line[0];
    
    if(!lSourceList[srcId].empty())
    {
      for(jit = lSourceList[srcId].begin(); jit != lSourceList[srcId].end(); ++jit)
      {
        if(jit->iParentID == id && jit->iChildID != sInputs.iID)
        {
          //forward it to the child
          ss<<" "<<sInputs.iID<<" "<<srcId<<" "<<data;
          oStream.open(sFNames.sOutFile.c_str(), ios::out | ios::app);
          oStream<<ss.str()<<endl;
          oStream.close();
          break;
        }
      }
    }
    if(sInputs.bReceiver && sInputs.iSenderID == srcId)
    {
      ss.str(std::string());
      ss<<data;
      oStream.open(sFNames.sRecFile.c_str(), ios::out | ios::app);
      oStream<<ss.str()<<endl;
      oStream.close();
    }
  }
  return;
}

void removeExpiredLSA()
{
  std::map<int,int>::iterator it;
  std::map<int,int>::iterator i;
  
  if(!mLSALifeTime.empty())
  {
    for(i = mLSALifeTime.begin(); i != mLSALifeTime.end(); ++i)
    {
      if(sRecord.iTimeStamp - i->second >= 30)
      {
        //clear the topology information for the stale record
        for(int j=0; j<MAX_NODES; j++)
        {
          iTopology[j][i->first] = 0;
        }
        
        //this is a stale recod. Remove it
        bLDBChanged = true;
        mLSALifeTime.erase(i);
        it = mLSA.find(i->first);
        if(it != mLSA.end())
        {
          mLSA.erase(it);
        }
        else
        {
          cout<<"DEBUG: mLSA("<<i->first<<") NOT found"<<endl;
        }
        
        //remove it from the neighbor list
        it = mNeighbors.find(i->first);
        if(it != mNeighbors.end())
        {
          mNeighbors.erase(it);
        }
        
      }
    }  
  }
  
  return;
}

std::vector<int> dijkstraSPT(int src, int dest)
{
  int                   dist[MAX_NODES];
  int                   parent[MAX_NODES];
  bool                  sptSet[MAX_NODES];       //visited nodes
  int                   u;
  std::vector<int>      sp;

  for(int i=0; i<MAX_NODES; i++)
  {
    dist[i] = 500000;
    sptSet[i] = false;
    parent[i] = -1;
  }

  dist[src] = 0;

  for(int count=0; count<MAX_NODES; count++)
  {
    u = minDistance(dist, sptSet);
    sptSet[u] = true;
    
    if(u == dest)
    {
      break;
    }

    for(int v=0; v<MAX_NODES; v++)
    {
      if((!sptSet[v]) && (iTopology[u][v] !=0) &&
         (dist[u] + iTopology[u][v] < dist[v]))
      {
        dist[v] = dist[u] + iTopology[u][v];
        parent[v] = u;
      }
    }
  }
  
  u = dest;
  while(u != src && u != -1 && parent[u] != -1)
  {
    sp.insert(sp.begin(), parent[u]);
    u = parent[u];
  }
  
  /*cout<<"shortest path "<<sInputs.iID<<" from src="<<src<<" to dest="<<dest<<" is:";
  for(std::vector<int>::iterator it=sp.begin(); it != sp.end(); it++)
  {
    cout<<*it<<" ";
  }
  cout<<endl;
  */  
  return sp;
}

int minDistance(int dist[], bool sptSet[])
{
   // Initialize min value
   int min = 500000;
   int min_index=0;

   for (int v = 0; v <MAX_NODES; v++)
   {
     if (sptSet[v] == false && dist[v] < min)
     {
       min = dist[v];
       min_index = v;
     }
   }
   return min_index;
}

void sendJoin(std::fstream* oFile)
{
  std::vector<int>                  spList;
  stringstream                      ss;
  std::vector<int>::iterator        it;
  std::list<joinPacket>::iterator   lit;
  
  joinPacket                        sPkt;
  int                               iParent;
  int                               latestTime = 0;
  int                               latestParent;
  bool                              bReceiverPresent = false;
  
  for(int i = 0; i<MAX_NODES; i++)
  {
    latestTime = 0;
          
    if(!(lSourceList[i].empty()))
    {
      lit = lSourceList[i].begin();
      latestParent = lit->iParentID;
      
      for(lit = lSourceList[i].begin(); lit != lSourceList[i].end(); ++lit)
      {
        if(lit->iJoinTS > latestTime)
        {
          latestTime = lit->iJoinTS;
        }
        if(sInputs.bReceiver && lit->iChildID == sInputs.iID)
        {
          bReceiverPresent = true;
        }
      }
               
      //cout<<"srcID="<<i<<" thisID="<<sInputs.iID<<" curTS="<<sRecord.iTimeStamp<<" latestTS="<<latestTime<<
      //" latestParent="<<latestParent<<endl;
           
      if((sRecord.iTimeStamp - latestTime) >= 10 || bLDBChanged)
      {
        if(latestParent != -1)
        {
          latestTime = sRecord.iTimeStamp;
          
          spList = computeJoinPath(i, sInputs.iID, &iParent);
          bLDBChanged = false;
          if(!spList.empty())
          {
            ss<<JOIN_MSG<<" "<<sInputs.iID<<" "<<i<<" "<<iParent;
            latestParent = iParent;
            for(it = spList.begin(); it != spList.end(); ++it)
            {
              if(*it != iParent && *it != sInputs.iID)
                ss<<" "<<*it;
            }
            
            //forward the join message to neighbors
            if(!oFile->is_open())
            {
              oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
            }
            *oFile<<ss.str()<<endl;
            oFile->close();
            
            for(lit = lSourceList[i].begin(); lit != lSourceList[i].end(); ++lit)
            {
              lit = lSourceList[i].erase(lit);
            }
            latestTime = sRecord.iTimeStamp;
            sPkt.iJoinTS = sRecord.iTimeStamp;
            sPkt.iRecTS = lit->iRecTS;
            sPkt.iChildID = lit->iChildID;
            sPkt.iParentID = iParent;
            lSourceList[i].push_back(sPkt); 
          }
          else
          {
            cout<<"for src="<<i<<" dest="<<sInputs.iID<<endl;
            for(int i=0; i<MAX_NODES; i++)
            {
              for(int j=0; j<MAX_NODES; j++)
              {
                //cout<<iTopology[i][j]<<" ";
              }
              cout<<endl;
            }
          }
        }
      }
      
      if(!bReceiverPresent)
      {
        lit = lSourceList[i].begin();
        sPkt.iJoinTS = latestTime;
        sPkt.iRecTS = 0;
        sPkt.iChildID = sInputs.iID;
        sPkt.iParentID = latestParent;
        lSourceList[i].push_back(sPkt);  
      }
      
      //if there are multiple childs, then make sure all have same joinTS
      for(lit = lSourceList[i].begin(); lit != lSourceList[i].end(); ++lit)
      {
        if(lit->iJoinTS != latestTime && lit->iParentID != -1) //check on the second clause
        {
          sPkt.iJoinTS = latestTime;
          sPkt.iRecTS = lit->iRecTS;
          sPkt.iChildID = lit->iChildID;
          sPkt.iParentID = latestParent;
          lit = lSourceList[i].erase(lit);
          lSourceList[i].push_back(sPkt);
        }  
      }
    }
    else if(sInputs.bReceiver && sInputs.iSenderID == i && !bReceiverPresent)
    {
      //compute parent
      //cout<<"src="<<sInputs.iSenderID<<" dest="<<sInputs.iID<<endl;
      spList = computeJoinPath(sInputs.iSenderID, sInputs.iID, &iParent);
      bLDBChanged = false;
      if(!spList.empty())
      {
        ss<<JOIN_MSG<<" "<<sInputs.iID<<" "<<sInputs.iSenderID<<" "<<iParent;
        for(it = spList.begin(); it != spList.end(); ++it)
        {
          if(*it != iParent && *it != sInputs.iID)
            ss<<" "<<*it;
        }
        
        //forward the join message to neighbors
        if(!oFile->is_open())
        {
          oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
        }
        *oFile<<ss.str()<<endl;
        oFile->close();
        
        //add the node in lSourceList
        sPkt.iJoinTS = sRecord.iTimeStamp;
        sPkt.iRecTS = 0;
        sPkt.iChildID = sInputs.iID;
        sPkt.iParentID = iParent;
        lSourceList[sInputs.iSenderID].push_back(sPkt);
      }
    }
  } 
  
  return;
}


std::vector<int> computeJoinPath(int src, int dest, int* parent)
{
  std::vector<int>              ret;
  std::vector<int>::iterator    it;
  
  //cout<<"ID="<<sInputs.iID<<" src="<<src<<" dest="<<dest<<endl;
  *parent = -1;  
  ret = dijkstraSPT(src, dest);
  if(!ret.empty())
  {
    it = ret.end()-1;
    *parent = *it;
    ret.clear();
    ret.insert(ret.begin(), *parent);
    
    //cout<<"*ID="<<sInputs.iID<<" src="<<src<<" dest="<<dest<<" parent="<<*parent<<endl;
    
    if(!iTopology[dest][*parent])
    {
      ret = dijkstraSPT(dest, *parent);
      //std::reverse(ret.begin(), ret.end());
    }
    if(ret.empty())
    {
      *parent = -1;
    }   
  }  
  return ret;
}

void removeExpiredJoin()
{
  std::list<joinPacket>::iterator     it;
  
  for(int i=0; i<MAX_NODES; i++)
  {
    if(!lSourceList[i].empty())
    {
      for(it=lSourceList[i].begin(); it!=lSourceList[i].end(); ++it)
      {
        if((it->iChildID != sInputs.iID) && (sRecord.iTimeStamp - it->iRecTS >= 20))
        {
          it = lSourceList[i].erase(it);
        }
      }
    }
  }
  return;
}

void sendData(std::fstream* oFile)
{
  stringstream    ss;
  ss<<sInputs.iID<<" "<<sInputs.iID<<" "<<sInputs.sData;
  
  if(sRecord.iTimeStamp % 10)
  {
    //we send data every 10 seconds
    return;
  }
  
  if(sInputs.bSender && !lSourceList[sInputs.iID].empty())
  {
    if(!oFile->is_open())
    {
      oFile->open(sFNames.sOutFile.c_str(), ios::out | ios::app);
    }
    (*oFile)<<DATA_MSG+" "+ss.str()<<endl;
    oFile->close();
  }
  
  return;  
}