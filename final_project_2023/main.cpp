#define FILE_EXTENSION ".txt"
#include <bits/stdc++.h>
#include "omp.h"

#pragma GCC target("sse,sse2,sse3,ssse3,sse4,avx,avx2,bmi,bmi2,lzcnt,popcnt")
#pragma GCC optimize("O3,unroll-loops,Ofast")

using namespace std;

#define endl '\n'

const int inf = 1e9 + 7;

struct Trie {
public:
    struct Node {
		array<Node*, 26> go{};
        int mx_depth = -inf;
        bool is_end = 0;
	};
	Trie() {
		root = new_node();
		sz = 1;
	}

	int size() const { return sz; }

	Node* next(Node* p, char c) {
		int v = c - 'a';
		if(p->go[v] == 0) {
			p->go[v] = new_node();
			sz++;
		}
		return p->go[v];
	}

	void insert(const string& s) {
		Node* p = root;
        int depth = 0;
        p->mx_depth = max(p->mx_depth, (int)s.size());
		for(char c : s) {
			p = next(p, c);
            depth++;
            p->mx_depth = max(p->mx_depth, (int)s.size() - depth);
		}
        p->is_end = 1;
	}

	void rev_insert(string s) {
        reverse(s.begin(), s.end());
        insert(s);
	}

	void clear() {
		root = new_node();
		sz = 1;
	}

    inline bool query_exact(const string& s) const {
        Node* p = root;
        for(int v : s) {
            v -= 'a';
            if(p->go[v]) p = p->go[v];
            else return 0;
        }
        return p->is_end;
    }

	inline bool query_prefix(const string& a) const {
        Node* p = root;
		for(int v : a) {
            v -= 'a';
			if(p->go[v]) p = p->go[v];
            else return 0;
		}
		return 1;
	}

    inline bool query_wild(const string& a, int idx, Node* cur, int cnt) const {
        if(idx == a.size()) return cur->is_end;
        if(cnt > cur->mx_depth) return 0;
        if(a[idx] == '*') {
            if(idx == a.size() - 1) return 1;
            if(cur->go[a[idx + 1] - 'a']) 
                if(query_wild(a, idx + 2, cur->go[a[idx + 1] - 'a'], cnt - 1))
                    return 1;
            for(int i = 0; i < 26; ++i) {
                if(cur->go[i]) {
                    if(query_wild(a, idx, cur->go[i], cnt)) return 1;
                }
            }
            return 0;
        }
        else {
            if(cur->go[a[idx] - 'a']) return query_wild(a, idx + 1, cur->go[a[idx] - 'a'], cnt - 1);
            else return 0;
        }
	}
    inline bool query_wild(const string &s) {
        int cnt = 0;
        for(auto &i : s) if(isalpha(i)) cnt++;
        return query_wild(s, 0, root, cnt);
    }

private:
	Node* root = nullptr;
	int sz = 0;

	Node* new_node() {
		return new Node();
	}
};

string int_to_string(int num) { //TODO
    return to_string(num);
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

vector<vector<pair<char, int>>> qry;
vector<string> prefix, suffix, exact, wild;
vector<vector<int>> ans;
vector<vector<int>> prefix_ans, suffix_ans, exact_ans, wild_ans;

int data_nums;
string title[100005];
vector<string> data_set[100005];
Trie prefix_trie[100005] = {}, suffix_trie[100005] = {};

void solve() {
    prefix_ans.resize(prefix.size());
    exact_ans.resize(exact.size());
    suffix_ans.resize(suffix.size());
    wild_ans.resize(wild.size());
    int n = data_nums, q = qry.size();

    //solve prefix and exact
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        for(auto j : data_set[i]) {
            prefix_trie[i].insert(j);
        }
    }
    #pragma omp parallel for
    for(int i = 0; i < prefix.size(); ++i) {
        for(int j = 0; j < n; ++j) {
            if(prefix_trie[j].query_prefix(prefix[i])) prefix_ans[i].push_back(j);
        }
    }
    #pragma omp parallel for
    for(int i = 0; i < exact.size(); ++i) {
        for(int j = 0; j < n; ++j) {
            if(prefix_trie[j].query_exact(exact[i]))
                exact_ans[i].push_back(j);
        }

    }
    #pragma omp parallel for
    for(int i = 0; i< wild.size(); ++i) {
        for(int j = 0; j < n; ++j) {
            if(prefix_trie[j].query_wild(wild[i]))
                wild_ans[i].push_back(j);
        }
    }
    //solve suffix
    #pragma omp parallel for
    for(int i = 0; i < n; ++i) {
        for(auto j : data_set[i]) {
            suffix_trie[i].rev_insert(j);
        }
    }
    #pragma omp parallel for
    for(int i = 0; i < suffix.size(); ++i) {
        reverse(suffix[i].begin(), suffix[i].end());
        for(int j = 0; j < n; ++j) {
            if(suffix_trie[j].query_prefix(suffix[i]))
                suffix_ans[i].push_back(j);
        }
    }

}

vector<int> And(const vector<int> &a, const vector<int> &b) {
    vector<int> res;
    for(int i = 0, j = 0; i < a.size() and j < b.size(); ) {
        if(a[i] < b[j]) i++;
        else if(a[i] > b[j]) j++;
        else {
            res.push_back(a[i]);
            i++, j++;
        }
    }
    return res;
}

vector<int> Or(const vector<int> &a, const vector<int> &b) {
    vector<int> res = a;
    for(auto i : b) {
        res.push_back(i);
    }
    sort(res.begin(), res.end());
    res.erase(unique(res.begin(), res.end()), res.end());
    return res;
}

vector<int> Sub(const vector<int> &a, const vector<int> &b) {
    vector<int> res;
    int i = 0;
    for(int j = 0; i < a.size() and j < b.size(); ) {
        if(a[i] < b[j]) res.push_back(a[i++]);
        else if(a[i] > b[j]) j++;
        else i++, j++;
    }
    while(i < a.size()) res.push_back(a[i++]);
    return res;
}

void Operator(char op, vector<int> &a, const vector<int> b) {
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
        for(int j = 0; j < qry[i].size(); ++j) {
            char a = qry[i][j].first;
            int b = qry[i][j].second;
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
            if(i[0] == '"' and i.back() == '"') { //exact
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
    
    for(int id = 0; ; ++id, ++data_nums) {
        string path = data_dir + to_string(id) + FILE_EXTENSION;
        fi.open(path, ios::in);
        if(!fi.is_open()) break;
        string title_name;
        getline(fi, title_name);
        title[id] = title_name;

        tmp_string = split(title_name, " ");
        vector<string> content = word_parse(tmp_string);
        for(auto &i : content)
            data_set[id].push_back(i);
        while(getline(fi, tmp)) {
            tmp_string = split(tmp, " ");
            vector<string> content = word_parse(tmp_string);
            for(auto &i : content)
                data_set[id].push_back(i);
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
