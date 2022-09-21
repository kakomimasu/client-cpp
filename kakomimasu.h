#define _CRT_SECURE_NO_WARNINGS
#include "picojson.h"
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
using namespace std;

struct Action
{
  int agentId;
  string type;
  int x;
  int y;
};
struct Tile
{
  int type;
  int pid;
  int point;
};
struct Agent
{
  int x;
  int y;
};

extern const int DIR[8][2];
void setHost(string s);
string curlGet(string req, string token);
string curlPost(string req, string post_data, string token);
int rnd(int n);

class KakomimasuClient
{
public:
  KakomimasuClient();
  void setBearerToken(string bearer);
  void setAi(string ai_name, string ai_board);
  void setGuestName(string guest_name);
  bool getGameInfo();
  void waitMatching();
  int getWidth();
  int getHeight();
  int getAgentCount();
  int getPlayerNumber();
  int getNextTurnUnixTime();
  int getTurn();
  int getTotalTurn();
  vector<vector<int>> getPoints();
  vector<vector<Tile>> getFiled();
  vector<Agent> getAgent();
  vector<Agent> getEnemyAgent();
  void waitNextTurn();
  void setAction(vector<Action> action);

private:
  string m_game_id;
  string m_guest_name;
  string m_bearer;
  string m_pic;
  string m_ai_name;
  string m_ai_board;
  int m_player_no;
  picojson::object m_gameInfo;
  picojson::object m_board;
};