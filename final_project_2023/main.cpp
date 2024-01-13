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

using namespace std;

template<int ALPHABET = 26, char MIN_CHAR = 'a'>
struct Trie {
public:
	struct Node {
		array<Node*, ALPHABET> go{};
        set<int> path, tail;
	};

	Trie() {
		root = new_node();
		sz = 1;
	}

	int size() const { return sz; }

	Node* next(Node* p, char c) {
		int v = c - MIN_CHAR;
		if(p->go[v] == nullptr) {
			p->go[v] = new_node();
			sz++;
		}
		return p->go[v];
	}

	void insert(const string& s, const int &idx) {
		Node* p = root;
		for(char c : s) {
			p = next(p, c);
            p->path.insert(idx);
		}
        p->tail.insert(idx);
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
        Node *p = root;
        for(int v : s) {
            v -= MIN_CHAR;
            if(p->go[v]) p = p->go[v];
            else return {};
        }
        return p->tail;
    }

	inline set<int> query_prefix(const string& a) const {
        Node *p = root;
		for(int v : a) {
            v -= MIN_CHAR;
			if(p->go[v]) p = p->go[v];
            else return {};
		}
		return p->path;
	}

    inline set<int> query_wild(const string& a, int idx, Node *cur) const {
        if(idx == a.size()) return cur->tail;
        if(a[idx] == '*') {
            if(idx == a.size() - 1) return cur->path;
            set<int> res;
            if(cur->go[a[idx + 1] - MIN_CHAR]) res = query_wild(a, idx + 2, cur->go[a[idx + 1] - MIN_CHAR]);
            for(int i = 0; i < ALPHABET; ++i) {
                if(cur->go[i]) {
                    auto tmp = query_wild(a, idx, cur->go[i]);
                    for(auto j : tmp) res.insert(j);
                }
            }
            return res;
        }
        else {
            if(cur->go[a[idx] - MIN_CHAR]) return query_wild(a, idx + 1, cur->go[a[idx] - MIN_CHAR]);
            else return {};
        }
	}
    inline set<int> query_wild(const string &s) {
        return query_wild(s, 0, root);
    }

private:
	Node* root = nullptr;
	int sz = 0;

	Node* new_node() {
		return new Node();
	}
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
    return f(a, b, 827167801, 999999937) and f(a, b, 998244353, 999999929) and f(a, b, 146672737, 922722049);
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

vector<string> title;
vector<vector<pair<char, int>>> qry;
vector<string> prefix, suffix, exact, wild;
vector<vector<string>> data_set;
vector<set<int>> ans;
vector<set<int>> prefix_ans, suffix_ans, exact_ans, wild_ans;

Trie prefix_trie, suffix_trie;

void solve() {
    prefix_ans.resize(prefix.size());
    exact_ans.resize(exact.size());
    suffix_ans.resize(suffix.size());
    wild_ans.resize(wild.size());
    int n = data_set.size(), q = qry.size();

    //solve prefix and exact
    #pragma omp parallel for
    for(int i = 0; i < prefix.size(); ++i) {
        auto ans = prefix_trie.query_prefix(prefix[i]);
        for(auto j : ans) prefix_ans[i].insert(j);
    }
    #pragma omp parallel for
    for(int i = 0; i < exact.size(); ++i) {
        auto ans = prefix_trie.query_exact(exact[i]);
        for(auto j : ans) exact_ans[i].insert(j);
    }
    //solve suffix
    #pragma omp parallel for
    for(int i = 0; i < suffix.size(); ++i) {
        reverse(suffix[i].begin(), suffix[i].end());
        auto ans = suffix_trie.query_prefix(suffix[i]);
        for(auto j : ans) suffix_ans[i].insert(j);
    }
    //solve wild
    #pragma omp parallel for
    for(int i = 0; i < wild.size(); ++i) {
        /*
        for(int j = 0; j < n; ++j) {
            for(auto k : data_set[j]) {
                if(wildcard(wild[i], k)) {
                    wild_ans[i].insert(j);
                    break;
                }
            }
        }
        */
        auto ans = prefix_trie.query_wild(wild[i]);
        for(auto j : ans) wild_ans[i].insert(j);
    }
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

void Operator(char op, set<int> &a, const set<int> b) {
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

void save_ans() {
    ans.resize(qry.size());
    #pragma omp parallel for
    for(int i = 0; i < qry.size(); ++i) {
        char op = '$';
        for(auto [a, b] : qry[i]) {
            if(a == 'p') {
                Operator(op, ans[i], prefix_ans[b]);
            }
            else if(a == 's') {
                Operator(op, ans[i], suffix_ans[b]);
            }
            else if(a == 'w') {
                Operator(op, ans[i], wild_ans[b]);
            }
            else if(a == 'e') {
                Operator(op, ans[i], exact_ans[b]);
            }
            else {
                op = a;
            }
        }
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

    //query
    fi.open(query, ios::in);
    while(getline(fi, tmp)) {
        tmp_string = split(tmp, " ");
        vector<pair<char, int>> v;
        for(auto &i : tmp_string) {
            if(i.size() == 1 and isalpha(i[0]) == 0) {
                v.push_back({i[0], -1}); // operator
                continue;
            }
            for(auto &j : i) {
                if(isalpha(j) and j >= 'A' and j <= 'Z') {
                    j = j - 'A' + 'a';
                }
            }
            if(i[0] == '\"' and i.back() == '\"') { //exact
                i.pop_back(); 
                i.erase(i.begin());
                v.push_back({'e', exact.size()});
                exact.push_back(i);
            }
            else if(i[0] == '*' and i.back() == '*') { //suffix
                i.pop_back(); 
                i.erase(i.begin());
                v.push_back({'s', suffix.size()});
                suffix.push_back(i);
            }
            else if(i[0] == '<' and i.back() == '>') {
                i.pop_back(); 
                i.erase(i.begin());
                v.push_back({'w', wild.size()});
                wild.push_back(i);
            }
            else {//prefix
                v.push_back({'p', prefix.size()});
                prefix.push_back(i);
            }
        }
        qry.push_back(v);
    }
    fi.close();

    while(1) {
        int id = title.size();
        string path = data_dir + to_string(id) + FILE_EXTENSION;
        fi.open(path, ios::in);
        if(!fi.is_open()) break;
        string title_name;
        getline(fi, title_name);
        title.push_back(title_name);
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

    solve();
    save_ans();

    fo.open(output.c_str(), ios::out);
    for(int i = 0; i < qry.size(); ++i) {
        if(ans[i].size() == 0) {
            fo << "Not Found!" << endl;
        }
        for(auto j : ans[i]) {
            fo << title[j] << endl;
        }
    }
    fo.close();
}
