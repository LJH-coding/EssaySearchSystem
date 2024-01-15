#define FILE_EXTENSION ".txt"
#include <functional>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <array>
#include <set>
#include <bits/stdc++.h>

#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")

using namespace std;
struct Node {
    set<int> path, tail;
	array<int, 26> go{};
}node_pool[8000005];

int new_node() {
    static int cnt = 0;    
    cnt++;
    return cnt;
}

struct Trie {
public:
	Trie() {
		root = new_node();
		sz = 1;
	}

	int size() const { return sz; }

	int next(int p, char c) {
		int v = c - 'a';
		if(node_pool[p].go[v] == 0) {
			node_pool[p].go[v] = new_node();
			sz++;
		}
		return node_pool[p].go[v];
	}

	void insert(const string& s, const int &idx) {
		int p = root;
		for(char c : s) {
			p = next(p, c);
            node_pool[p].path.insert(idx);
		}
        node_pool[p].tail.insert(idx);
	}

	void rev_insert(string s, const int &idx) {
        reverse(s.begin(), s.end());
        insert(s, idx);
	}

	void clear() {
		root = new_node();
		sz = 1;
	}

    inline set<int> query_exact(const string& s) const {
        int p = root;
        for(int v : s) {
            v -= 'a';
            if(node_pool[p].go[v]) p = node_pool[p].go[v];
            else return {};
        }
        return node_pool[p].tail;
    }

	inline set<int> query_prefix(const string& a) const {
        int p = root;
		for(int v : a) {
            v -= 'a';
			if(node_pool[p].go[v]) p = node_pool[p].go[v];
            else return {};
		}
		return node_pool[p].path;
	}

    inline set<int> query_wild(const string& a, int idx, int node_idx) const {
        const Node &cur = node_pool[node_idx];
        if(idx == a.size()) return cur.tail;
        if(a[idx] == '*') {
            if(idx == a.size() - 1) return cur.path;
            set<int> res;
            if(cur.go[a[idx + 1] - 'a']) res = query_wild(a, idx + 2, cur.go[a[idx + 1] - 'a']);
            for(int i = 0; i < 26; ++i) {
                if(cur.go[i]) {
                    auto tmp = query_wild(a, idx, cur.go[i]);
                    for(auto j : tmp) res.insert(j);
                }
            }
            return res;
        }
        else {
            if(cur.go[a[idx] - 'a']) return query_wild(a, idx + 1, cur.go[a[idx] - 'a']);
            else return {};
        }
	}
    inline set<int> query_wild(const string &s) {
        return query_wild(s, 0, root);
    }

private:
    int root = 0;
	int sz = 0;
};

template<class T>
struct rolling_hash{
    int p, q;
    vector<int> hash, p_table;
    inline void build(T v, int x, int y){
        hash.resize(v.size() + 5);
        p_table.resize(v.size() + 5);
        p = x, q = y;
		p_table[0] = 1,hash[0] = (int)v[0];
		for(int j = 1;j<v.size();++j){
			p_table[j] = (1ll*p_table[j-1]*p)%q;
			hash[j] = (1ll*hash[j-1]*p+(int)v[j])%q;
		}
    }
	inline int query(int l,int r){
		int ans;
        if(l==0){
			ans = hash[r];
			return ans;
		}
		int x = (hash[r]-(1ll*hash[l-1]*p_table[r-l+1]))%q;
		x = (x+q)%q;
		ans = x;
        return ans;
    }
};
int wildcard(const string &a, const string &b){
    function<int(string, string, int, int)> f = [&](string a, string b, int p, int q) {
        rolling_hash<string>hs[2];
        hs[0].build(a, p, q);
        hs[1].build(b, p, q);
        int n = a.size(),m = b.size();
        for(int l = 0,r = -2,lhs = 0,rhs = 0,j = 0;l<n;l = r+2,lhs = rhs,rhs = 0){
            for(int i = r+2;i<n;++i){
                if(a[i]=='*'){
                    rhs = 1;
                    r = i-1;
                    break;
                }
            }
            if(!rhs)r = n-1;
            if(l>r)continue;
            int len = r-l;
            if(!lhs){
                if(j+len>=m || hs[1].query(j,j+len)!=hs[0].query(l,r)){
                    return 0;
                }
                j = j+len+1;
            }
            else{
                bool flag = 0;
                for(;j+len<m;++j){
                    if(hs[1].query(j,j+len)==hs[0].query(l,r)){
                        if(!rhs and j+len!=m-1)continue;
                        flag = 1;
                        j = j+len+1;
                        break;
                    }
                }
                if(!flag){
                    return 0;
                }
            }
        }
        return 1;
    };
    return f(a, b, 827167801, 999999937);
//    return f(a, b, 827167801, 999999937) and f(a, b, 998244353, 999999929) and f(a, b, 146672737, 922722049);
}

vector<string> word_parse(vector<string> tmp_string){
	vector<string> parse_string;
	for(auto& word : tmp_string){
		string new_str;
    	for(auto &ch : word){
			if(isalpha(ch)) {
                if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
				new_str.push_back(ch);
            }
		}
        if(new_str.size() > 0)
            parse_string.emplace_back(new_str);
	}
	return parse_string;
}

vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if("" == str) return res;

	char * strs = new char[str.length() + 1] ; 
	strcpy(strs, str.c_str());

	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char *p = strtok(strs, d);
	while(p) {
		string s = p; 
		res.push_back(s); 
		p = strtok(NULL, d);
	}

	return res;
}

string title[200000];
vector<vector<string>> data_set;

Trie prefix_trie, suffix_trie;

set<int> query_wild(const string &qry) {
    set<int> res;
    for(int i = 0; i < data_set.size(); ++i) {
        for(auto j : data_set[i]) {
            if(wildcard(qry, j)) {
                res.insert(i);
                break;
            }
        }
    }
    return res;
}

set<int> And(const set<int> &a, const set<int> &b) {
    set<int> res;
    for(auto i : a) {
        if(b.find(i) != b.end()) res.insert(i);
    }
    return res;
}

set<int> Or(const set<int> &a, const set<int> &b) {
    set<int> res = a;
    for(auto i : b) res.insert(i);
    return res;
}

set<int> Sub(const set<int> &a, const set<int> &b) {
    set<int> res = a;
    for(auto i : a) {
        if(b.find(i) != b.end()) res.erase(res.find(i));
    }
    return res;
}

void Operator(char op, set<int> &a, const set<int> &b) {
    if(op == '$') {
        a = b;
    }
    else if(op == '+') {
        a = And(a, b);
    }
    else if(op == '-') {
        a = Sub(a, b);
    }
    else {
        a = Or(a, b);
    }
}

int main(int argc, char *argv[]) {

    string data_dir = argv[1] + string("/");
	string query = string(argv[2]);
	string output = string(argv[3]);

	string tmp;
	fstream fi;
    fstream fo;
	vector<string> tmp_string;

    for(int id = 0; ; ++id) {
        string path = data_dir + to_string(id) + FILE_EXTENSION;
        fi.open(path, ios::in);
        if(!fi.is_open()) break;
        string title_name;
        getline(fi, title_name);
        title[id] = title_name;
        data_set.push_back({});

        tmp_string = split(title_name, " ");
        vector<string> content = word_parse(tmp_string);
        for(auto &i : content) {
            data_set.back().push_back(i);
            prefix_trie.insert(i, id);
            suffix_trie.rev_insert(i, id);
        }
        while(getline(fi, tmp)) {
            tmp_string = split(tmp, " ");
            vector<string> content = word_parse(tmp_string);
            for(auto &i : content) {
                data_set.back().push_back(i);
                prefix_trie.insert(i, id);
                suffix_trie.rev_insert(i, id);
            }
        }

        fi.close();
    }

    //query
    fo.open(output, ios::out);
    fi.open(query, ios::in);
    while(getline(fi, tmp)) {
        tmp_string = split(tmp, " ");
        set<int> ans;
        char op = '$';
        for(auto &qry : tmp_string) {
            string s = "";
            for(auto i : qry) {
                if(isalpha(i)) {
                    if(i >= 'A' and i <= 'Z') i = i - 'A' + 'a';
                    s.push_back(i);
                }
                else if(qry[0] == '<' and i == '*') s.push_back(i);
            }
            if(qry[0] == '"') {
                Operator(op, ans, prefix_trie.query_exact(s));
            }
            else if(qry[0] == '*') {
                reverse(s.begin(), s.end());
                Operator(op, ans, suffix_trie.query_prefix(s));
            }
            else if(qry[0] == '<') {
                Operator(op, ans, prefix_trie.query_wild(s));
//                Operator(op, ans, prefix_trie.query_wild(s));
            }
            else if(qry.size() == s.size()) {
                Operator(op, ans, prefix_trie.query_prefix(s));
            }
            else {
                op = qry[0];
            }
        }
        if(ans.size() == 0) {
            fo << "Not Found!" << endl;
        }
        for(auto i : ans) {
            fo << title[i] << endl;
        }
    }
    fi.close();
    fo.close();
}
