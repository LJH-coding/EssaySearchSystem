#include <bits/stdc++.h>
#include "omp.h"

#pragma GCC target("sse,sse2,sse3,ssse3,sse4,avx,avx2,abm,mmx,bmi,bmi2,lzcnt,popcnt,fma,tune=native")
#pragma GCC optimize("O3,unroll-loops,Ofast,Os,no-stack-protector,fast-math")
#pragma comment(linker, "/stack:200000000")

using namespace std;

#define FILE_EXTENSION ".txt"
#define FILE_NUMS 30000
#define NUM_THREADS 6
#define endl '\n'
#define all(v) begin(v), end(v)

const int inf = 1e9 + 7;

struct Node {
    Node *go[26] = {};
    int mx_depth = -inf;
    set<int> path, tail;
};

Node* new_node() {
    return new Node();
}

inline Node* next(Node *p, char c) {
    int v = c - 'a';
    if(p->go[v] == NULL) {
        p->go[v] = new_node();
    }
    return p->go[v];
}

inline void insert(Node *p, const string& s, const int &id) {
    int depth = 0;
    p->mx_depth = max(p->mx_depth, (int)s.size());
    for(char c : s) {
        p = next(p, c);
        depth++;
        p->path.insert(id);
        p->mx_depth = max(p->mx_depth, (int)s.size() - depth);
    }
    p->tail.insert(id);
}

inline void rev_insert(Node *p, string s, const int &id) {
    reverse(s.begin(), s.end());
    insert(p, s, id);
}

inline set<int> query_exact(Node *p, const string& s) {
    if(s.size() > p->mx_depth) return {};
    for(int v : s) {
        v -= 'a';
        if(p->go[v]) p = p->go[v];
        else return {};
    }
    return p->tail;
}

inline set<int> query_prefix(Node *p, const string& a) {
    if(a.size() > p->mx_depth) return {};
    for(int v : a) {
        v -= 'a';
        if(p->go[v]) p = p->go[v];
        else return {};
    }
    return p->path;
}

inline set<int> query_wild(const string& a, const int &idx, Node* cur, const int &cnt) {
    if(idx == a.size()) return cur->tail;
    if(cnt > cur->mx_depth) return {};
    if(a[idx] == '*') {
        if(idx == a.size() - 1) return cur->path;
        set<int> res;
        if(cur->go[a[idx + 1] - 'a']) res = query_wild(a, idx + 2, cur->go[a[idx + 1] - 'a'], cnt - 1);
        for(int i = 0; i < 26; ++i) {
            if(cur->go[i]) {
                auto tmp = query_wild(a, idx, cur->go[i], cnt);
                for(auto &i : tmp) res.insert(i);
            }
        }
        return res;
    }
    else {
        if(cur->go[a[idx] - 'a']) return query_wild(a, idx + 1, cur->go[a[idx] - 'a'], cnt - 1);
        else return {};
    }
}
inline set<int> query_wild(Node *p, const string &s) {
    int cnt = 0;
    for(auto &i : s) if(isalpha(i)) cnt++;
    return query_wild(s, 0, p, cnt);
}

inline string query_parse(const string &qry) {
    string res;
    for(auto &i : qry) {
        if(isalpha(i)) {
            if(i >= 'A' && i <= 'Z') res.push_back(i - 'A' + 'a');
            else res.push_back(i);
        }
        else if(i == '*' and qry[0] == '<' and (res.empty() or res.back() != '*')) res.push_back(i);
    }
    return res;
}

inline vector<string> word_parse(const vector<string> &tmp_string) {
	vector<string> parse_string;
	for(auto& word : tmp_string){
		string new_str;
    	for(auto &ch : word){
			if(isalpha(ch)) {
                if(ch >= 'A' && ch <= 'Z') new_str.push_back(ch - 'A' + 'a');
                else new_str.push_back(ch);
            }
		}
        if(new_str.size() > 0)
            parse_string.emplace_back(new_str);
	}
	return parse_string;
}

inline vector<string> split(const string& str, const string& pattern) {
    vector<string> result;
    string::size_type begin, end;

    end = str.find(pattern);
    begin = 0;

    while (end != string::npos) {
        if (end - begin != 0) {
            result.push_back(str.substr(begin, end-begin)); 
        }    
        begin = end + pattern.size();
        end = str.find(pattern, begin);
    }

    if (begin != str.length()) {
        result.push_back(str.substr(begin));
    }
    return result;
}

inline void And(set<int> &a, const set<int> &b) {
    set<int> res;
    for(auto &i : a) {
        if(b.find(i) != b.end()) res.insert(i);
    }
    a = res;
}

inline void Or(set<int> &a, const set<int> &b) {
    for(auto &i : b) a.insert(i);
}

inline void Sub(set<int> &a, const set<int> &b) {
    set<int> res;
    for(auto &i : a) {
        if(b.find(i) == b.end()) res.insert(i);
    }
    a = res;
}

int data_nums = 0;
string title[FILE_NUMS];
vector<string> data_set[FILE_NUMS];
Node *prefix_trie[NUM_THREADS] = {}, *suffix_trie[NUM_THREADS] = {};
vector<string> qry;
vector<set<int>> ans;

inline bool readfile(const string &path, const int &id) {
	fstream fi;
    fi.open(path, ios::in);
    if(!fi.is_open()) return 0;
    string title_name, tmp;
    getline(fi, title_name);
    title[id] = title_name;
    vector<string> tmp_string = split(title_name, " ");
    vector<string> content = word_parse(tmp_string);
    for(auto &i : content) {
        data_set[id].push_back(i);
    }
    while(getline(fi, tmp)) {
        tmp_string = split(tmp, " ");
        vector<string> content = word_parse(tmp_string);
        for(auto &i : content) { 
            data_set[id].push_back(i);
        }
    }
    fi.close();
    return 1;
}

inline void readquery(const string &path) {
    fstream fi;
    fi.open(path, ios::in);
    string tmp;
    while(getline(fi, tmp)) qry.push_back(tmp);
    ans.resize(qry.size());
    fi.close();
}

inline void build_trie() {
    #pragma omp parallel for num_threads(NUM_THREADS)
    for(int i = 0; i < NUM_THREADS; ++i) {
        prefix_trie[i] = new Node();
        suffix_trie[i] = new Node();
    }
    #pragma omp parallel for num_threads(NUM_THREADS)
    for(int i = 0; i < data_nums; ++i) {
        int thread_id = omp_get_thread_num();
        for(const auto &s : data_set[i]) {
            insert(prefix_trie[thread_id], s, i);
            rev_insert(suffix_trie[thread_id], s, i);
        }
    }
}

inline set<int> query(const string &s) {
    string tmp = query_parse(s);
    set<int> res[NUM_THREADS];
    if(s[0] == '"') {
        #pragma omp parallel for num_threads(NUM_THREADS)
        for(int i = 0; i < NUM_THREADS; ++i) {
            res[i] = query_exact(prefix_trie[i], tmp);
        }
    }
    else if(s[0] == '*') {
        reverse(all(tmp));
        #pragma omp parallel for num_threads(NUM_THREADS)
        for(int i = 0; i < NUM_THREADS; ++i) {
            res[i] = query_prefix(suffix_trie[i], tmp);
        }
    }
    else if(s[0] == '<') {
        #pragma omp parallel for num_threads(NUM_THREADS)
        for(int i = 0; i < NUM_THREADS; ++i) {
            res[i] = query_wild(prefix_trie[i], tmp);
        }
    }
    else {
        #pragma omp parallel for num_threads(NUM_THREADS)
        for(int i = 0; i < NUM_THREADS; ++i) {
            res[i] = query_prefix(prefix_trie[i], tmp);
        }
    }
    for(int i = 1; i < NUM_THREADS; ++i) for(auto &j : res[i]) res[0].insert(j);
    return res[0];
}

inline void solve() {
    #pragma omp parallel for num_threads(NUM_THREADS)
    for(int i = 0; i < qry.size(); ++i) {
        vector<string> tmp_string = split(qry[i], " ");
        ans[i] = query(tmp_string[0]);
        for(int j = 1; j < tmp_string.size(); j += 2) {
            if(tmp_string[j][0] == '+') {
                And(ans[i], query(tmp_string[j + 1]));
            }
            else if(tmp_string[j][0] == '-') {
                Sub(ans[i], query(tmp_string[j + 1]));
            }
            else {
                Or(ans[i], query(tmp_string[j + 1]));
            }
        }
    }
}

int main(int argc, char *argv[]) {
//    cout << "NUM_THREADS : " << NUM_THREADS << endl;
    ios::sync_with_stdio(0), cin.tie(0), cout.tie(0);
    omp_set_num_threads(NUM_THREADS);

    string data_dir = argv[1] + string("/");
	string query = string(argv[2]);
	string output = string(argv[3]);

    //readfile
    for(int id = 0; ; ++id, ++data_nums) {
        string path = data_dir + to_string(id) + FILE_EXTENSION;
        if(!readfile(path, id)) break;
    }

    //build trie
    build_trie();

    //readquery
    readquery(query);

    solve();

    fstream fo;
    fo.open(output, ios::out);
    for(int i = 0; i < qry.size(); ++i) {
        if(ans[i].size() == 0) {
            fo << "Not Found!" << endl;
        }
        for(const auto &j : ans[i]) {
            fo << title[j] << endl;
        }
    }
    fo.close();

}
