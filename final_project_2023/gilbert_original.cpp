#define FILE_EXTENSION ".txt"
#define MAX_FILE_NUM 50000
#define BUFFER_SIZE (16 * 1024)
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<iostream>
#include<set>
#include<algorithm>
#include<cassert>
#include<fcntl.h>
//#pragma loop-opt(on)
//#pragma GCC target("sse,sse2,sse3,ssse3,sse4,avx,avx2,fma,tune=native")
//#pragma GCC optimize("unroll-loops")
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
string titleName[MAX_FILE_NUM];

// Trie 
class Trie {
public:
    static int totalCnt;

    class Node {
    public:
        vector<int> nxt;
        set<int> wordPrefix;
        set<int> wordEnd;
        set<int> reversePrefix;
        int max_depth;
    } tr[2000000];

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
    set<int> wildcards_dfs(int now, const string &s, int cur_index, int left);
};

Trie trie;

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

char buf[BUFFER_SIZE];
inline vector<string> split(const string& str, const char *d) {
	vector<string> res;
	if("" == str) return res;
	strcpy(buf, str.c_str());

	char *p = strtok(buf, d);
	while (p) {
		string s = p; 
		res.push_back(s); 
		p = strtok(NULL, d);
	}
	return res;
}

// My Func
inline bool readFile(string &dir, int data_id) {
    if (data_id >= MAX_FILE_NUM) return false;
    string file_path = dir + to_string(data_id) + ".txt";

	string file, title_name, buffer;
	fstream fi;
	vector<string> tmp_string;

	fi.open(file_path, ios::in);
    if (!fi) 
        return false;

	getline(fi, title_name);
    titleName[data_id] = title_name;
    tmp_string = split(title_name, " ");

	vector<string> title = word_parse(tmp_string);
    for (auto &s : title) {
        trie.insert(s, data_id);
        reverse(s.begin(), s.end());
        trie.reverse_insert(s, data_id);
    }

	while(getline(fi, buffer)){
		tmp_string = split(buffer, " ");
		vector<string> content = word_parse(tmp_string);
        for (string &s : content) {
            trie.insert(s, data_id);
            reverse(s.begin(), s.end());
            trie.reverse_insert(s, data_id);
        }
	}

	fi.close();
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

set<int> query_string(const string &s) {
    string pure_string = format_qstring(s);
    if (s[0] == '"') { // exact
        return trie.search_exact(pure_string);
    } else if (s[0] == '*') { // suffix
        reverse(pure_string.begin(), pure_string.end());
        return trie.search_suffix(pure_string);
    } else if (s[0] == '<') { // wildcard
        return trie.search_wildcards(pure_string);
    } 
    // prefix
    return trie.search_prefix(pure_string);
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

int main(int argc, char *argv[])
{
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);

    string data_dir = argv[1] + string("/");
	string query_path = string(argv[2]);
	string output_path = string(argv[3]);

	// Read File & Parser Example
    //#pragma omp for
    int file_num = 0;
    for (int i = 0; ; i++) {
        file_num = i + 1;
        if (!readFile(data_dir, i))
            break;
    }

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
int Trie::totalCnt = -1;

inline int Trie::NewNode() {
    totalCnt++;
    tr[totalCnt].nxt.resize(26, 0); 
    tr[totalCnt].max_depth = -1000000;
    return totalCnt;
}

inline Trie::Trie() {
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
        chmax(tr[cur].max_depth, n - i + 1);
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
    for (int i = 0; i < s.size(); i++) if (s[i] != '*') cnt++;
    return wildcards_dfs(0, s, 0, cnt);
}

inline set<int> Trie::wildcards_dfs(int now, const string &s, int cur_index, int left) {
    char c = s[cur_index]; 
    if (cur_index == s.size()) {
        return tr[now].wordEnd;
    }
    if (tr[now].max_depth < left) return set<int>();
    if (c == '*') {
        if (cur_index == s.size() - 1) return tr[now].wordPrefix; // * as last char
        set<int> res;
        int cid = get_char_id(s[cur_index + 1]);
        if (tr[now].nxt[cid]) // * as empty char
            res = wildcards_dfs(tr[now].nxt[cid], s, cur_index + 2, left - 1);
        for (int i = 0; i < 26; i++) {
            if (tr[now].nxt[i]) {
                set<int> tmp = wildcards_dfs(tr[now].nxt[i], s, cur_index, left);
                res.insert(tmp.begin(), tmp.end());
            }
        }
        return res;
    } else {
        int cid = get_char_id(c);
        if (!tr[now].nxt[cid]) {
            return set<int>();
        }
        return wildcards_dfs(tr[now].nxt[cid], s, cur_index + 1, left - 1);
    }
}

