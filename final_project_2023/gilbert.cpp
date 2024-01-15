#define FILE_EXTENSION ".txt"
#define MAX_FILE_NUM 30000
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<iostream>
#include<set>
#include<algorithm>
#include<cassert>
#include<fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <omp.h>
#pragma loop-opt(on)
//#pragma GCC target("sse,sse2,sse3,ssse3,sse4,avx,avx2,fma,tune=native")
#pragma GCC optimize("unroll-loops")
#pragma GCC optimize("O3")
#pragma GCC optimize("Ofast")

using namespace std;

bool chmax(int& a, const int& b) { return b>a ? a=b, 1 : 0; }

// debug
//#define loli
void abc() {cout << endl;}
template <typename T, typename ...U> void abc(T a, U ...b) {
    cout << a << ' ', abc(b...);
}
#ifdef loli
#define test(args...) abc("[" + string(#args) + "]", args)
#else
#define test(args...) void(0)
#endif

// Global Var
int num_procs = 4;
string titleName[MAX_FILE_NUM + 5];
vector<vector<string>> paral;

// Trie 
class Trie {
public:
    int totalCnt;

    class Node {
    public:
        vector<int> nxt;
        set<int> wordPrefix;
        set<int> wordEnd;
        set<int> reversePrefix;
    } tr[400000];

    Trie();
    inline int NewNode();
    inline int get_char_id(char c);
    inline void insert(const string &s, int data_id);
    inline void reverse_insert(const string &s, int data_id);
    inline set<int> search_prefix(const string &s);
    inline set<int> search_suffix(const string &s);
    inline set<int> search_exact(const string &s);
    inline set<int> search_wildcards(const string &s);

private:
    int root;
    set<int> wildcards_dfs(int now, const string &s, int cur_index);
};

vector<Trie> trie;
// Utility Func
// string parser : output vector of strings (words) after parsing
inline vector<string> word_parse(const vector<string> &tmp_string){
	vector<string> parse_string;
	for(auto& word : tmp_string){
		string new_str;
    	for(auto &ch : word){
			if(isalpha(ch))
				new_str.push_back(ch);
		}
		parse_string.emplace_back(new_str);
	}
	return parse_string;
}

inline vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if("" == str) return res;

	char * strs = new char[str.length() + 1] ; 
	strcpy(strs, str.c_str());

	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char *p = strtok(strs, d);
	while (p) {
		string s = p; 
		res.push_back(s); 
		p = strtok(NULL, d);
	}

	return res;
}

inline string parse_1word(const string &s) {
    string res;
    for (char c : s) if (isalpha(c)) res.push_back(c);
    return res;

}

// My Func
#define BUFFER_SIZE (8 * 1024)
inline bool readFile(string &dir, int data_id) {
    if (data_id >= MAX_FILE_NUM) return false;
    string file_path = dir + to_string(data_id) + ".txt";

    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) return false;

//    posix_fadvise(fd, 0, 0, 1);

    char buf[BUFFER_SIZE + 1];
    int first = 1;
    vector<string> tmp_string;
    while (size_t read_num = read(fd, buf, BUFFER_SIZE)) {
        if (!read_num) break;
        if (first) {
            for (int i = 0; ; i++) {
                if (buf[i] == '\n') 
                    break;
                titleName[data_id].push_back(buf[i]);
            }
        }
        first = 0;

        buf[read_num] = '\0';
        char *p = strtok(buf, " \n");
        while (p) {
            string s = p; 
            s = parse_1word(s);
            tmp_string.emplace_back(s);
            p = strtok(NULL, " \n");
        }
    }
    paral.emplace_back(tmp_string);

    close(fd);
    return true;
}

string format_qstring(const string &s) {
    string res;
    for (int i = 0; i < s.size(); i++) {
        if ((i == 0 || i == s.size() - 1) && !isalpha(s[i])) continue;
        if (isalpha(s[i])) res.push_back(tolower(s[i]));
        if (s[i] == '*' && s[i-1] == '*') continue;
        else if (s[i] == '*') res.push_back('*');
    }
    return res;
}

// set operation
void my_add(set<int> &a, set<int> &b) {
    a.insert(b.begin(), b.end());
}

void my_sub(set<int> &a, set<int> &b) {
    for (int i : b) {
        if (a.find(i) != a.end())
            a.erase(i);
    }
}

void my_inter(set<int> &a, set<int> &b) {
    set<int> tmp;
    if (a.size() <= b.size()) {
        for (int i : a) if (b.find(i) != b.end())
            tmp.insert(i);
    } else {
        for (int i : b) if (a.find(i) != a.end())
            tmp.insert(i);
    }
    swap(tmp, a);
}

set<int> query_string(const string &s) {
    string pure_string = format_qstring(s);
    set<int> res[num_procs];
    if (s[0] == '"') { // exact
        #pragma omp parallel for default(none) num_threads(num_procs) shared(res, pure_string, num_procs, trie)
        for (int i = 0; i < num_procs; i++) {
            res[i] = trie[i].search_exact(pure_string);
        }
    } else if (s[0] == '*') { // suffix
        reverse(pure_string.begin(), pure_string.end());
        #pragma omp parallel for default(none) num_threads(num_procs) shared(res, pure_string, num_procs, trie)
        for (int i = 0; i < num_procs; i++) {
            res[i] = trie[i].search_suffix(pure_string);
        }
    } else if (s[0] == '<') { // wildcard
        #pragma omp parallel for default(none) num_threads(num_procs) shared(res, pure_string, num_procs, trie)
        for (int i = 0; i < num_procs; i++) {
            res[i] = trie[i].search_wildcards(pure_string);
        }
    } else { // prefix
        #pragma omp parallel for default(none) num_threads(num_procs) shared(res, pure_string, num_procs, trie)
        for (int i = 0; i < num_procs; i++) {
            res[i] = trie[i].search_prefix(pure_string);
        }
    }
    for (int i = 1; i < num_procs; i++) my_add(res[0], res[i]);
    return res[0];
}

int main(int argc, char *argv[])
{
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

    string data_dir = argv[1] + string("/");
	string query_path = string(argv[2]);
	string output_path = string(argv[3]);


    int num_file = 0;
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        num_file = i;
        if (!readFile(data_dir, i))
            break;
    }

	// Read File & Parser Example
    int limit = 16;
    if (num_file <= 2000) limit = 4;
    num_procs = min(limit, omp_get_num_procs() - 2);
    test(num_procs);
    trie.resize(num_procs);
    #pragma omp parallel default(none) shared(data_dir, trie, num_file, paral) num_threads(num_procs) 
    {
        #pragma omp for
        for (int i = 0; i < num_file; i++) {
            int tid = omp_get_thread_num();
            for (auto &s : paral[i]) {
                trie[tid].insert(s, i);
                reverse(s.begin(), s.end());
                trie[tid].reverse_insert(s, i);
            }
        }
    }

    for (int i = 0; i < num_procs; i++) 
        test(trie[i].totalCnt);

    // Answer Query
    fstream queryFile;
	queryFile.open(query_path, ios::in);

    ofstream outFile;
    outFile.open(output_path);

    string tmp_string;
    while (getline(queryFile, tmp_string)) {
        vector<string> tmp_parse_string = split(tmp_string, " ");
        set<int> res = query_string(tmp_parse_string[0]);

        for (int i = 1; i < tmp_parse_string.size(); i += 2) {
            char op = tmp_parse_string[i][0];
            string qs = tmp_parse_string[i+1];

            set<int> cur;
            cur = query_string(qs);

            if (op == '+') {
                my_inter(res, cur);
            } else if (op == '-') {
                my_sub(res, cur);
            } else if (op == '/') {
                my_add(res, cur);
            } else {  // something go wrong
                cout << "unkown opperation " << op << endl;
            }
        }
        for (int i : res) 
            outFile << titleName[i] << '\n';
        if (res.empty())
            outFile << "Not Found!\n";
    }

    return 0;
}

//////////////////////////////////////////////////////////

// Trie implement
inline int Trie::NewNode() {
    totalCnt++;
    tr[totalCnt].nxt.resize(26);
    return totalCnt;
}

inline Trie::Trie() {
    totalCnt = -1;
    root = NewNode();
}

inline int Trie::get_char_id(char c) {
    if ('a' <= c && c <= 'z') return (c - 'a');
    if ('A' <= c && c <= 'Z') return (c - 'A');
    return -1;
}

inline void Trie::insert(const string &s, int data_id) {
    int cur = root, n = s.size();
    for (int i = 0; i < n; i++) {
        int cid = get_char_id(s[i]); 
        if (!tr[cur].nxt[cid]) {
            tr[cur].nxt[cid] = NewNode();
        }
        cur = tr[cur].nxt[cid];
        tr[cur].wordPrefix.insert(data_id);
    }
    tr[cur].wordEnd.insert(data_id);
}

inline void Trie::reverse_insert(const string &s, int data_id) {
    int cur = root;
    for (char c : s) {
        int cid = get_char_id(c); 
        if (!tr[cur].nxt[cid]) {
            tr[cur].nxt[cid] = NewNode();
        }
        cur = tr[cur].nxt[cid];
        tr[cur].reversePrefix.insert(data_id);
    }
}

inline set<int> Trie::search_exact(const string &s) {
    int cur = root;
    for (char c : s) {
        int cid = get_char_id(c); 
        if (!tr[cur].nxt[cid]) {
            return set<int>();
        }
        cur = tr[cur].nxt[cid];
    }
    return tr[cur].wordEnd;
}

inline set<int> Trie::search_prefix(const string &s) {
    int cur = root;
    for (char c : s) {
        int cid = get_char_id(c); 
        if (!tr[cur].nxt[cid]) {
            return set<int>();
        }
        cur = tr[cur].nxt[cid];
    }
    return tr[cur].wordPrefix;
}

inline set<int> Trie::search_suffix(const string &s) {
    int cur = root;
    for (char c : s) {
        int cid = get_char_id(c); 
        if (!tr[cur].nxt[cid]) {
            return set<int>();
        }
        cur = tr[cur].nxt[cid];
    }
    return tr[cur].reversePrefix;
}

inline set<int> Trie::search_wildcards(const string &s) {
    int cnt = root;
    return wildcards_dfs(0, s, 0);
}

inline set<int> Trie::wildcards_dfs(int now, const string &s, int cur_index) {
    char c = s[cur_index]; 
    if (cur_index == s.size()) {
        return tr[now].wordEnd;
    }
    if (c == '*') {
        if (cur_index == s.size() - 1) return tr[now].wordPrefix; // * as last char
        set<int> res;
        int cid = get_char_id(s[cur_index + 1]);
        if (tr[now].nxt[cid]) // * as empty char
            res = wildcards_dfs(tr[now].nxt[cid], s, cur_index + 2);
        for (int i = 0; i < 26; i++) {
            if (tr[now].nxt[i]) {
                set<int> tmp = wildcards_dfs(tr[now].nxt[i], s, cur_index);
                res.insert(tmp.begin(), tmp.end());
            }
        }
        return res;
    } else {
        int cid = get_char_id(c);
        if (!tr[now].nxt[cid]) {
            return set<int>();
        }
        return wildcards_dfs(tr[now].nxt[cid], s, cur_index + 1);
    }
}

