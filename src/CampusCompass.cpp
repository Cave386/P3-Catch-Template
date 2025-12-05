#include "CampusCompass.h"

using namespace std;


bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath)
{
    // Parse edges file
    ifstream file(edges_filepath);

    if (!file.is_open()) {
        cerr << "Failed to open edges file." << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        int id1, id2, time;
        string name1, name2;

        string token;

        getline(ss, token, ',');
        id1 = stoi(token);

        getline(ss, token, ',');
        id2 = stoi(token);

        getline(ss, token, ',');
        name1 = token;

        getline(ss, token, ',');
        name2 = token;

        getline(ss, token, ',');
        time = stoi(token);

        id_to_hall[id1] = name1;
        id_to_hall[id2] = name2;

        if (adj_list.find(id1) == adj_list.end())
            adj_list[id1] = vector<tuple<int, int, bool> >();

        if (adj_list.find(id2) == adj_list.end())
            adj_list[id2] = vector<tuple<int, int, bool> >();

        adj_list[id1].push_back({id2, time, true});
        adj_list[id2].push_back({id1, time, true});
    }
    file.close();

    // Parse classes file
    ifstream classes_file(classes_filepath);

    if (!classes_file.is_open()) {
        cerr << "Failed to open classes file." << endl;
        return false;
    }

    while (getline(classes_file, line)) {
        stringstream ss(line);
        string class_code;
        int location_id, start_time, end_time;

        string token;

        getline(ss, token, ',');
        class_code = token;

        getline(ss, token, ',');
        location_id = stoi(token);

        getline(ss, token, ',');
        start_time = stoi(token);

        getline(ss, token, ',');
        end_time = stoi(token);

        Class c;
        c.location_id = location_id;
        c.start = start_time;
        c.end = end_time;

        class_info[class_code] = c;
    }
    classes_file.close();

    return true;
}

bool CampusCompass::insertStudent(string name, string id, int residence, unordered_set<string> classes)
{
    Student s;
    s.name = name;
    s.id = id;
    s.residence_location = residence;
    s.classes = classes;

    if (students.find(id) == students.end())
        students[id] = s;
    else
        return false;

    return true;
}

bool CampusCompass::removeStudent(string id)
{
    if (students.find(id) == students.end())
        return false;

    students.erase(id);
    return true;
}

bool CampusCompass::dropClass(string id, string class_code)
{
    if (students.find(id) == students.end())
        return false;

    if ((students[id].classes).find(class_code) == (students[id].classes).end())
        return false;

    (students[id].classes).erase(class_code);
    if ((students[id].classes).size() == 0)
        students.erase(id);

    return true;
}

bool CampusCompass::replaceClass(string id, string class_code1, string class_code2)
{
    if (students.find(id) == students.end())
        return false;

    if ((students[id].classes).find(class_code1) == (students[id].classes).end())
        return false;

    if ((students[id].classes).find(class_code2) != (students[id].classes).end())
        return false;

    (students[id].classes).erase(class_code1);
    (students[id].classes).insert(class_code2);

    return true;
}

int CampusCompass::removeClass(string class_code)
{
    int total = 0;
    vector<string> to_remove;

    for (auto &s : students)
    {
        if ((s.second.classes).find(class_code) != (s.second.classes).end())
        {
            (s.second.classes).erase(class_code);
            total++;

            // Mark student for removal if they have no more classes
            if ((s.second.classes).size() == 0)
                to_remove.push_back(s.first);
        }
    }

    // Remove students with no classes
    for (const auto& id : to_remove)
        students.erase(id);

    if (class_info.find(class_code) != class_info.end())
        class_info.erase(class_code);

    return total;
}

void CampusCompass::toggleEdgesClosure(vector<pair<int, int>> edges)
{
    for (const auto& e : edges)
    {
        for (auto& edgeTuple : adj_list[e.first])
        {
            if (get<0>(edgeTuple) == e.second)
            {
                get<2>(edgeTuple) = !get<2>(edgeTuple);
                break;
            }
        }
        for (auto& edgeTuple : adj_list[e.second])
        {
            if (get<0>(edgeTuple) == e.first)
            {
                get<2>(edgeTuple) = !get<2>(edgeTuple);
                break;
            }
        }
    }
}

int CampusCompass::checkEdgeStatus(pair<int,int> edge)
{
    for (auto& e : adj_list[edge.first])
    {
        if (get<0>(e) == edge.second)
        {
            if (get<2>(e) == true)
                return 1;

            return 2;
        }
    }

    return 0;
}

bool CampusCompass::isConnected(int start, int target)
{
    if (start == target) return true;

    unordered_set<int> visited;
    queue<int> q;

    q.push(start);
    visited.insert(start);

    while (!q.empty())
    {
        int curr = q.front();
        q.pop();

        for (auto& edge : adj_list[curr])
        {
            int neighbor = get<0>(edge);
            bool open = get<2>(edge);

            if (!open)
                continue;

            if (!visited.count(neighbor))
            {
                if (neighbor == target)
                    return true;

                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    return false;
}

tuple<Student, unordered_map<int, int>, unordered_map<int, int>> CampusCompass::dijkstra(string id)
{
    int INF = numeric_limits<int>::max();

    if (!students.count(id)) {
        cout << "Student not found.\n";
        return {};
    }

    Student stu = students[id];
    int start = stu.residence_location;

    unordered_map<int, int> distance;
    unordered_map<int, int> prev;

    for (auto& p : adj_list)
        distance[p.first] = INF;
    distance[start] = 0;

    using pii = pair<int,int>;
    priority_queue<pii, vector<pii>, greater<pii>> pq;
    pq.push({0, start});

    while (!pq.empty())
    {
        auto [dist_u, u] = pq.top();
        pq.pop();

        if (dist_u > distance[u]) continue;

        for (auto& edge : adj_list[u])
        {
            int v   = get<0>(edge);
            int w   = get<1>(edge);
            bool open = get<2>(edge);

            if (!open) continue;

            if (distance[u] + w < distance[v])
            {
                distance[v] = distance[u] + w;
                prev[v] = u;
                pq.push({distance[v], v});
            }
        }
    }

    return { stu, distance, prev };
}

void CampusCompass::printStudentEdges(Student stu, unordered_map<int, int> distance, unordered_map<int, int> prev)
{
    int INF = numeric_limits<int>::max();
    cout << "Name: " << stu.name << "\n";

    vector<string> classList(stu.classes.begin(), stu.classes.end());
    sort(classList.begin(), classList.end());

    for (auto &code : classList)
    {
        int buildingID = -1;
        if (class_info.count(code))
            buildingID = class_info[code].location_id;

        int t = -1;

        if (buildingID != -1 && distance.count(buildingID) && distance[buildingID] != INF)
            t = distance[buildingID];

        cout << code << " | Total Time: " << t << "\n";
    }
}

int CampusCompass::printStudentZone(int id_int)
{
    string id = to_string(id_int);

    if (!students.count(id)) {
        cout << "Student not found.\n";
        return -1;
    }

    auto [stu, distance, prev] = dijkstra(id);

    int INF = numeric_limits<int>::max();
    int start = stu.residence_location;

    unordered_set<int> zoneNodes;

    for (const string &code : stu.classes)
    {
        if (!class_info.count(code)) continue;

        int target = class_info[code].location_id;

        int cur = target;
        while (cur != start && prev.count(cur)) {
            zoneNodes.insert(cur);
            cur = prev[cur];
        }
        zoneNodes.insert(start);
    }

    struct Edge { int u, v, w; };
    vector<Edge> edges;

    for (int u : zoneNodes) {
        for (auto &e : adj_list[u]) {
            int v = get<0>(e);
            int w = get<1>(e);
            bool open = get<2>(e);

            if (!open) continue;
            if (!zoneNodes.count(v)) continue;
            if (u < v)
                edges.push_back({u, v, w});
        }
    }

    struct DSU {
        unordered_map<int,int> p, r;
        void add(int x){ p[x] = x; r[x] = 0; }
        int find(int x){ return p[x]==x ? x : p[x]=find(p[x]); }
        void unite(int a,int b){
            a = find(a); b = find(b);
            if (a != b) {
                if (r[a] < r[b]) swap(a,b);
                p[b] = a;
                if (r[a] == r[b]) r[a]++;
            }
        }
    } dsu;

    for (int v : zoneNodes) dsu.add(v);

    sort(edges.begin(), edges.end(),
         [](auto &a, auto &b){ return a.w < b.w; });

    int mstCost = 0;

    for (auto &e : edges) {
        if (dsu.find(e.u) != dsu.find(e.v)) {
            dsu.unite(e.u, e.v);
            mstCost += e.w;
        }
    }

    cout << "Student Zone Cost For " << stu.name << ": " << mstCost << "\n";
    return mstCost;
}


bool CampusCompass::ParseCommand(const string &command)
{
    if (command.empty()) return false;

    string cmd = command;
    stringstream ss(cmd);
    string mainCmd;
    ss >> mainCmd;

    if (mainCmd == "insert")
    {
        size_t firstQuote = cmd.find('"');
        size_t secondQuote = cmd.find('"', firstQuote + 1);
        if (firstQuote == string::npos || secondQuote == string::npos) {
            cout << "unsuccessful\n";
            return false;
        }

        string name = cmd.substr(firstQuote + 1, secondQuote - firstQuote - 1);

        string after = cmd.substr(secondQuote + 1);
        stringstream s2(after);

        string id;
        int residence, n;
        if (!(s2 >> id >> residence >> n)) {
            cout << "unsuccessful\n";
            return false;
        }

        vector<string> classes;
        for (int i = 0; i < n; i++) {
            string c;
            if (!(s2 >> c)) {
                cout << "unsuccessful\n";
                return false;
            }
            classes.push_back(c);
        }

        unordered_set<string> cls(classes.begin(), classes.end());

        bool ok = insertStudent(name, id, residence, cls);
        cout << (ok ? "successful\n" : "unsuccessful\n");
        return ok;
    }

    if (mainCmd == "remove")
    {
        string id;
        if (!(ss >> id)) {
            cout << "unsuccessful\n";
            return false;
        }

        bool ok = removeStudent(id);
        cout << (ok ? "successful\n" : "unsuccessful\n");
        return ok;
    }

    if (mainCmd == "dropClass")
    {
        string id, code;
        if (!(ss >> id >> code)) {
            cout << "unsuccessful\n";
            return false;
        }

        bool ok = dropClass(id, code);
        cout << (ok ? "successful\n" : "unsuccessful\n");
        return ok;
    }

    if (mainCmd == "replaceClass")
    {
        string id, c1, c2;
        if (!(ss >> id >> c1 >> c2)) {
            cout << "unsuccessful\n";
            return false;
        }

        bool ok = replaceClass(id, c1, c2);
        cout << (ok ? "successful\n" : "unsuccessful\n");
        return ok;
    }

    if (mainCmd == "removeClass")
    {
        string code;
        if (!(ss >> code)) {
            cout << "unsuccessful\n";
            return false;
        }

        int dropped = removeClass(code);
        cout << dropped << "\n";
        return true;
    }

    if (mainCmd == "toggleEdgesClosure")
    {
        int n;
        if (!(ss >> n)) {
            cout << "unsuccessful\n";
            return false;
        }

        vector<pair<int,int>> edges;
        for (int i = 0; i < n; i++) {
            int a, b;
            if (!(ss >> a >> b)) {
                cout << "unsuccessful\n";
                return false;
            }
            edges.push_back({a, b});
        }

        toggleEdgesClosure(edges);
        cout << "successful\n";
        return true;
    }

    if (mainCmd == "checkEdgeStatus")
    {
        int a, b;
        if (!(ss >> a >> b)) {
            cout << "unsuccessful\n";
            return false;
        }

        int result = checkEdgeStatus({a, b});
        if (result == 1) cout << "open\n";
        else if (result == 2) cout << "closed\n";
        else cout << "DNE\n";
        return true;
    }

    if (mainCmd == "isConnected")
    {
        int a, b;
        if (!(ss >> a >> b)) {
            cout << "unsuccessful\n";
            return false;
        }

        bool ok = isConnected(a, b);
        cout << (ok ? "successful\n" : "unsuccessful\n");
        return true;
    }

    if (mainCmd == "printShortestEdges")
    {
        string id;
        if (!(ss >> id)) {
            cout << "unsuccessful\n";
            return false;
        }

        auto tup = dijkstra(id);
        Student stu;
        unordered_map<int,int> dist, prev;
        tie(stu, dist, prev) = tup;

        printStudentEdges(stu, dist, prev);
        return true;
    }

    if (mainCmd == "printStudentZone")
    {
        int idnum;
        if (!(ss >> idnum)) {
            cout << "unsuccessful\n";
            return false;
        }

        printStudentZone(idnum);
        return true;
    }

    cout << "unsuccessful\n";
    return false;
}
