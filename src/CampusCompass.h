#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

struct Student {
    string name;
    string id;
    int residence_location;
    unordered_set<string> classes;
};

struct Class {
    int location_id;
    int start;
    int end;
};


class CampusCompass {
public:
    unordered_map<int, string> id_to_hall;
    unordered_map<string, unordered_set<string>> class_roster;
    unordered_map<int, vector<tuple<int, int, bool>>> adj_list;

    // map student & class names to objects w/ more data
    unordered_map<string, Student> students;
    unordered_map<string, Class> class_info;

    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);

    bool insertStudent(string name, string id, int residence, unordered_set<string> classes);
    bool removeStudent(string id);
    bool dropClass(string id, string class_code);
    bool replaceClass(string id, string class_code1, string class_code2);
    int removeClass(string class_code);
    void toggleEdgesClosure(vector<pair<int, int>> edges);
    int checkEdgeStatus(pair<int, int> edge);
    bool isConnected(int start, int target);
    tuple<Student, unordered_map<int, int>, unordered_map<int, int>> dijkstra(string id);
    void printStudentEdges(Student stu, unordered_map<int, int> distance, unordered_map<int, int> prev);
    int printStudentZone(int id);

    bool ParseCommand(const string &command);
};
