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

string host = "https://kakomimasu.website/api";
void setHost(string s) {
    host = s;
}

static size_t callbackWrite(char *ptr, size_t size, size_t nmemb, string *stream) {
    int dataLength = size * nmemb;
    stream->append(ptr, dataLength);
    return dataLength;
}

string curlGet(string req, string token = "") {
    // cout << "GET " << req << endl;
    const int sz = 102400;
    char buf[sz];
    char cmdline[sz];
    snprintf(cmdline, sz, "curl -s -H 'Authorization: %s' %s%s", token.c_str(), host.c_str(), req.c_str());
#ifdef _MSC_VER
    FILE *fp = _popen(cmdline, "r");
#else
    FILE *fp = popen(cmdline, "r");
#endif
    fgets(buf, sz, fp);
    string res(buf);
    // cout << res << endl;
    return res;
}

string curlPost(string req, string post_data, string bearer = "") {
    // cout << "POST " << req << endl;
    const int sz = 102400;
    char buf[sz];
    char cmdline[sz];
#ifdef _WIN64
    for (int i = 0; i < post_data.size(); ++i) {
        if (post_data[i] == '"') {
            post_data.insert(i, "\\");
            i++;
        }
    }
    snprintf(cmdline, sz, "curl -s -X POST -H \"Authorization:Bearer %s\" -H \"Content-Type: application/json\" -d %s \"%s%s\"", bearer.c_str(), post_data.c_str(), host.c_str(), req.c_str());
#else
    snprintf(cmdline, sz, "curl -s -X POST -H \"Authorization:Bearer %s\" -H \"Content-Type: application/json\" -d '%s' \"%s%s\"", bearer.c_str(), post_data.c_str(), host.c_str(), req.c_str());
#endif

#ifdef _MSC_VER
    FILE *fp = _popen(cmdline, "r");
#else
    FILE *fp = popen(cmdline, "r");
#endif

    fgets(buf, sz, fp);
    string res(buf);
    // cout << res << endl;
    return res;
}

string userShow(string identifier) {
    string res = curlGet("/users/show/" + identifier);
    cout << res << endl;
    return res;
}

random_device seed_gen;
mt19937 engine(seed_gen());
int rnd(int n) {
    return engine() % n;
}

KakomimasuClient::KakomimasuClient(string bearer) {
    m_bearer = bearer;
}

bool KakomimasuClient::getGameInfo() {
    string res = curlGet("/match/" + m_game_id);

    picojson::value val;
    picojson::parse(val, res);
    picojson::object obj = val.get<picojson::object>();

    m_gameInfo = obj;
    if (!m_gameInfo["board"].is<picojson::null>()) {
        m_board = m_gameInfo["board"].get<picojson::object>();
    }

    return m_gameInfo["gaming"].get<bool>();
}

void KakomimasuClient::waitMatching() {
    picojson::object send_obj;
    string res = curlPost("/match", picojson::value(send_obj).serialize(), m_bearer);

    picojson::value val;
    picojson::parse(val, res);
    picojson::object obj = val.get<picojson::object>();

    m_game_id = obj["gameId"].get<string>();
    m_player_no = obj["index"].get<double>();

    while (!getGameInfo()) {
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout << "maching!" << endl;
}

int KakomimasuClient::getWidth() {
    return (int)m_board["width"].get<double>();
};

int KakomimasuClient::getHeight() {
    return (int)m_board["height"].get<double>();
};

int KakomimasuClient::getAgentCount() {
    return (int)m_board["nAgent"].get<double>();
};

int KakomimasuClient::getPlayerNumber() {
    return m_player_no;
};

int KakomimasuClient::getNextTurnUnixTime() {
    return (int)m_gameInfo["nextTurnUnixTime"].get<double>();
};

int KakomimasuClient::getTurn() {
    return (int)m_gameInfo["turn"].get<double>();
};

int KakomimasuClient::getTotalTurn() {
    return (int)m_gameInfo["totalTurn"].get<double>();
};

vector<vector<int>> KakomimasuClient::getPoints() {
    int height = getHeight();
    int width = getWidth();
    vector<vector<int>> res(height, vector<int>(width));
    picojson::array ary = m_board["points"].get<picojson::array>();
    int i = 0, j = 0;
    for (auto &val : ary) {
        res[i][j] = val.get<double>();

        j++;
        if (j == width) j = 0, i++;
    }

    return res;
}

vector<vector<Tile>> KakomimasuClient::getFiled() {
    int height = getHeight();
    int width = getWidth();
    vector<vector<Tile>> res(height, vector<Tile>(width));
    picojson::array ary = m_board["points"].get<picojson::array>();
    int i = 0, j = 0;
    for (auto &val : ary) {
        res[i][j].point = val.get<double>();

        j++;
        if (j == width) j = 0, i++;
    }

    i = j = 0;
    ary = m_gameInfo["tiled"].get<picojson::array>();
    for (auto &val : ary) {
        int type, pid;
        sscanf(val.serialize().c_str(), "[%d,%d]", &type, &pid);
        res[i][j].type = type;
        res[i][j].pid = pid;

        j++;
        if (j == width) j = 0, i++;
    }

    return res;
}

vector<Agent> KakomimasuClient::getAgent() {
    vector<Agent> res;
    picojson::array player = m_gameInfo["players"].get<picojson::array>();
    picojson::object obj = player[m_player_no].get<picojson::object>();
    picojson::array ary = obj["agents"].get<picojson::array>();
    for (int i = 0; i < ary.size(); i++) {
        picojson::object pos = ary[i].get<picojson::object>();
        res.push_back({(int)pos["x"].get<double>(), (int)pos["y"].get<double>()});
    }
    return res;
};

vector<Agent> KakomimasuClient::getEnemyAgent() {
    vector<Agent> res;
    picojson::array player = m_gameInfo["players"].get<picojson::array>();
    picojson::object obj = player[1 - m_player_no].get<picojson::object>();
    picojson::array ary = obj["agents"].get<picojson::array>();
    for (int i = 0; i < ary.size(); i++) {
        picojson::object pos = ary[i].get<picojson::object>();
        res.push_back({(int)pos["x"].get<double>(), (int)pos["y"].get<double>()});
    }
    return res;
};

void KakomimasuClient::waitNextTurn() {
    int next_time = getNextTurnUnixTime();
    while (time(NULL) < next_time) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void KakomimasuClient::setAction(vector<Action> action) {
    picojson::array arr;
    for (const auto &[agentId, type, x, y] : action) {
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

    cout << curlPost("/match/" + m_game_id + "/action", post_data, m_bearer) << endl;
};