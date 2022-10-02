#include "kakomimasu.h"

const int DIR[8][2] = {
    {-1, -1},
    {0, -1},
    {1, -1},
    {-1, 0},
    {1, 0},
    {-1, 1},
    {0, 1},
    {1, 1},
};

string host = "https://api.kakomimasu.com";
void setHost(string s)
{
    host = s;
}

string curlGet(string req, string token = "")
{
    cout << "GET " << req << endl;
    const int sz = 102400;
    char *buf = new char[sz];
    char *cmdline= new char[sz];
    snprintf(cmdline, sz, "curl -s -H 'Authorization: %s' %s%s", token.c_str(), host.c_str(), req.c_str());
#ifdef _MSC_VER
    FILE *fp = _popen(cmdline, "r");
#else
    FILE *fp = popen(cmdline, "r");
#endif
    fgets(buf, sz, fp);

#ifdef _MSC_VER
	_pclose(fp);
#else
	pclose(fp);
#endif

    string res(buf);
    // cout << res << endl;

    delete[] buf;
    delete[] cmdline;
    return res;
}

string curlPost(string req, string post_data, string auth = "")
{
    cout << "POST " << req << endl;
    // cout << post_data << endl;
    const int sz = 102400;
    char *buf= new char[sz];
    char *cmdline = new char[sz];
#if defined(_WIN64) || defined(_WIN32)
    for (int i = 0; i < post_data.size(); ++i)
    {
        if (post_data[i] == '"')
        {
            post_data.insert(i, "\\");
            i++;
        }
    }
    snprintf(cmdline, sz, "curl -s -X POST -H \"Authorization:%s\" -H \"Content-Type: application/json\" -d %s \"%s%s\"", auth.c_str(), post_data.c_str(), host.c_str(), req.c_str());
#else
    snprintf(cmdline, sz, "curl -s -X POST -H \"Authorization:%s\" -H \"Content-Type: application/json\" -d '%s' \"%s%s\"", auth.c_str(), post_data.c_str(), host.c_str(), req.c_str());
#endif

#ifdef _MSC_VER
    FILE *fp = _popen(cmdline, "r");
#else
    FILE *fp = popen(cmdline, "r");
#endif

    fgets(buf, sz, fp);

#ifdef _MSC_VER
    _pclose(fp);
#else
    pclose(fp);
#endif
    string res(buf);
    // cout << res << endl;
    delete[] buf;
    delete[] cmdline;
    return res;
}

random_device seed_gen;
mt19937 engine(seed_gen());
int rnd(int n)
{
    return engine() % n;
}

KakomimasuClient::KakomimasuClient()
{
    m_guest_name = "cpp-guest";
}

void KakomimasuClient::setBearerToken(string bearer)
{
    m_bearer = bearer;
}

void KakomimasuClient::setAi(string ai_name, string ai_board)
{
    m_ai_name = ai_name;
    m_ai_board = ai_board;
}

void KakomimasuClient::setGuestName(string guest_name)
{
    m_guest_name = guest_name;
}

bool KakomimasuClient::getGameInfo()
{
    string res = curlGet("/v1/match/" + m_game_id);

    picojson::value val;
    picojson::parse(val, res);
    picojson::object obj = val.get<picojson::object>();

    m_gameInfo = obj;
    if (!m_gameInfo["board"].is<picojson::null>())
    {
        m_board = m_gameInfo["board"].get<picojson::object>();
    }

    return m_gameInfo["gaming"].get<bool>();
}

void KakomimasuClient::waitMatching()
{
    string auth = "";

    picojson::object send_obj;

    if (m_bearer.empty())
    {
        picojson::object guest_obj;
        guest_obj["name"] = picojson::value(m_guest_name);
        send_obj["guest"] = picojson::value(guest_obj);
    }
    else
    {
        auth = "Bearer " + m_bearer;
    }

    if (!m_ai_name.empty())
    {
        picojson::object ai_obj;
        ai_obj["aiName"] = picojson::value(m_ai_name);
        if (!m_ai_board.empty())
        {
            ai_obj["boardName"] = picojson::value(m_ai_board);
        }

        send_obj["useAi"] = picojson::value(true);
        send_obj["aiOption"] = picojson::value(ai_obj);
    }

    string res = curlPost("/v1/match", picojson::value(send_obj).serialize(), auth);

    picojson::value val;
    picojson::parse(val, res);
    picojson::object obj = val.get<picojson::object>();

    m_game_id = obj["gameId"].get<string>();
    m_player_no = (int)obj["index"].get<double>();
    m_pic = obj["pic"].get<string>();

    cout << "gameId: " << m_game_id << endl;
    cout << "playerNo: " << m_player_no << endl;

    while (!getGameInfo())
    {
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout << "maching!" << endl;
}

int KakomimasuClient::getWidth()
{
    return (int)m_board["width"].get<double>();
};

int KakomimasuClient::getHeight()
{
    return (int)m_board["height"].get<double>();
};

int KakomimasuClient::getAgentCount()
{
    return (int)m_board["nAgent"].get<double>();
};

int KakomimasuClient::getPlayerNumber()
{
    return m_player_no;
};

int KakomimasuClient::getNextTurnUnixTime()
{
    int start_at_unix_time = (int)m_gameInfo["startedAtUnixTime"].get<double>();
    int operation_sec = (int)m_gameInfo["operationSec"].get<double>();
    int transition_sec = (int)m_gameInfo["transitionSec"].get<double>();
    int turn = (int)m_gameInfo["turn"].get<double>();
    int next_turn_unix_time = start_at_unix_time + (operation_sec + transition_sec) * turn;
    cout << start_at_unix_time << " " << operation_sec << " " << transition_sec << " " << turn << " " << next_turn_unix_time << endl;
    return next_turn_unix_time;
};

int KakomimasuClient::getTurn()
{
    return (int)m_gameInfo["turn"].get<double>();
};

int KakomimasuClient::getTotalTurn()
{
    return (int)m_gameInfo["totalTurn"].get<double>();
};

vector<vector<int>> KakomimasuClient::getPoints()
{
    int height = getHeight();
    int width = getWidth();
    vector<vector<int>> res(height, vector<int>(width));
    picojson::array ary = m_board["points"].get<picojson::array>();
    int i = 0, j = 0;
    for (auto &val : ary)
    {
        res[i][j] = (int)val.get<double>();

        j++;
        if (j == width)
            j = 0, i++;
    }

    return res;
}

vector<vector<Tile>> KakomimasuClient::getFiled()
{
    int height = getHeight();
    int width = getWidth();
    vector<vector<Tile>> res(height, vector<Tile>(width));
    picojson::array ary = m_board["points"].get<picojson::array>();
    int i = 0, j = 0;
    for (auto &val : ary)
    {
        res[i][j].point = (int)val.get<double>();

        j++;
        if (j == width)
            j = 0, i++;
    }

    i = j = 0;
    ary = m_gameInfo["tiled"].get<picojson::array>();
    for (auto &val : ary)
    {
        int type, pid;
        sscanf(val.serialize().c_str(), "[%d,%d]", &type, &pid);
        res[i][j].type = type;
        res[i][j].pid = pid;

        j++;
        if (j == width)
            j = 0, i++;
    }

    return res;
}

vector<Agent> KakomimasuClient::getAgent()
{
    vector<Agent> res;
    picojson::array player = m_gameInfo["players"].get<picojson::array>();
    picojson::object obj = player[m_player_no].get<picojson::object>();
    picojson::array ary = obj["agents"].get<picojson::array>();
    for (int i = 0; i < ary.size(); i++)
    {
        picojson::object pos = ary[i].get<picojson::object>();
        res.push_back({(int)pos["x"].get<double>(), (int)pos["y"].get<double>()});
    }
    return res;
};

vector<Agent> KakomimasuClient::getEnemyAgent()
{
    vector<Agent> res;
    picojson::array player = m_gameInfo["players"].get<picojson::array>();
    picojson::object obj = player[1 - m_player_no].get<picojson::object>();
    picojson::array ary = obj["agents"].get<picojson::array>();
    for (int i = 0; i < ary.size(); i++)
    {
        picojson::object pos = ary[i].get<picojson::object>();
        res.push_back({(int)pos["x"].get<double>(), (int)pos["y"].get<double>()});
    }
    return res;
};

void KakomimasuClient::waitNextTurn()
{
    int next_time = getNextTurnUnixTime();
    while (time(NULL) < next_time)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void KakomimasuClient::setAction(vector<Action> action)
{
    picojson::array arr;
    for (const auto &[agentId, type, x, y] : action)
    {
        picojson::object obj;
        obj.emplace(make_pair("agentId", picojson::value((double)agentId)));
        obj.emplace(make_pair("type", type));
        obj.emplace(make_pair("x", picojson::value((double)x)));
        obj.emplace(make_pair("y", picojson::value((double)y)));

        arr.push_back(picojson::value(obj));
    }

    picojson::object send_obj;
    send_obj.emplace(make_pair("actions", arr));
    send_obj.emplace(make_pair("index", (double)m_player_no));

    picojson::value val(send_obj);
    string post_data = val.serialize();

    cout << curlPost("/v1/match/" + m_game_id + "/action", post_data, m_pic) << endl;
};