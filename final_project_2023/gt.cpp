#define FILE_EXTENSION ".txt"
#pragma GCC optimize("O2")
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <bitset>
using namespace std;
const int N = 10000;

struct trie {
  struct node {
    int cnt, sum;
    node* next[26];
    node(): cnt(0), sum(0) {fill_n(next, 26, nullptr);}
  } *root;
  void insert(const string& str) {
    node* cur = root;
    for (const auto& i : str) {
      auto tmp = toupper(i) - 'A';
      if (!cur->next[tmp]) cur->next[tmp] = new node;
      cur = cur->next[tmp];
      cur->sum++;
    }
    cur->cnt++;
  }
  int find_exact(const string& str) {
    node* cur = root;
    for (const auto& i : str) {
      auto tmp = toupper(i) - 'A';
      if (!cur->next[tmp]) return 0;
      cur = cur->next[tmp];
    }
    return cur->cnt;
  }
  int find_prefix(const string& str) {
    node* cur = root;
    for (const auto& i : str) {
      auto tmp = toupper(i) - 'A';
      if (!cur->next[tmp]) return 0;
      cur = cur->next[tmp];
    }
    return cur->sum;
  }
  bool dfs(node* cur, const string& str, int idx) {
    if (idx == str.size()) return cur->cnt || str[idx - 1] == '*';
    if (isalpha(str[idx])) {
      auto tmp = toupper(str[idx]) - 'A';
      if (!cur->next[tmp]) return 0;
      return dfs(cur->next[tmp], str, idx + 1);
    }
    else { // str[idx] == '*'
      if (dfs(cur, str, idx + 1)) return 1;
      for (int i = 0; i < 26; i++)
        if (cur->next[i] && dfs(cur->next[i], str, idx))
          return 1;
      return 0;
    }
  }
  int find_wildcard(const string& str) {
    return dfs(root, str, 0);
  }
  void del(const node* cur) {
    if (!cur) return;
    for (int i = 0; i < 26; i++) del(cur->next[i]);
    delete cur;
  }
  trie(): root(new node) {}
  // ~trie() {del(root);}
} tries[N], suffix_tries[N];
string title[N];

void trim(string& str) {
  string tmp;
  for (auto i : str)
    if (isalpha(i)) tmp.push_back(toupper(i));
  str = tmp;
}

int main(int argc, char *argv[]) {
  ios::sync_with_stdio(false), cin.tie(nullptr);
  string data_dir = argv[1] + string("/");
  string query = string(argv[2]);
  string output = string(argv[3]);

  fstream fin;
  string tmp;
  int n = 0;

  for (int i = 0; fin.open(data_dir + to_string(i) + ".txt", ios::in), fin; i++) {
    n++;
    getline(fin, tmp), title[i] = tmp;

    stringstream ss(tmp);
    while (ss >> tmp)
      trim(tmp), tries[i].insert(tmp),
           reverse(tmp.begin(), tmp.end()), suffix_tries[i].insert(tmp);

    while (fin >> tmp)
      trim(tmp), tries[i].insert(tmp),
           reverse(tmp.begin(), tmp.end()), suffix_tries[i].insert(tmp);
    fin.close();
  }

  fstream Q, fout;
  Q.open(query, ios::in);
  fout.open(output, ios::out);

  bitset<N> ans;

  while (getline(Q, tmp)) {
    stringstream ss(tmp);
    bitset<N> ans;
    bool flag = 0;
    string A, B;
    while (!ss.eof()) {
      bitset<N> result;

      if (!flag) flag = 1, ss >> B;
      else ss >> A >> B;

      if (isalpha(B[0])) { // prefix
        trim(B);
        for (int i = 0; i < n; i++)
          if (tries[i].find_prefix(B)) result[i] = 1;
      }
      else if (B[0] == '\"') { // exact
        trim(B);
        for (int i = 0; i < n; i++)
          if (tries[i].find_exact(B)) result[i] = 1;
      }
      else if (B[0] == '*') { // suffix
        trim(B), reverse(B.begin(), B.end());
        for (int i = 0; i < n; i++)
          if (suffix_tries[i].find_prefix(B)) result[i] = 1;
      }
      else if (B[0] == '<') { // wildcard
        B.pop_back(), B = B.substr(1);
        for (auto& i : B) i = toupper(i);
        for (int i = 0; i < n; i++)
          if (tries[i].find_wildcard(B)) result[i] = 1;
      }
      // else cerr << "Error\n", exit(0);
      if (A.empty()) ans |= result;
      else if (A[0] == '+') ans &= result;
      else if (A[0] == '/') ans |= result;
      else if (A[0] == '-') ans &= ~result;
      // else cerr << "Error\n", exit(0);
    }

    bool found = 0;
    for (int i = 0; i < n; i++)
      if (ans[i]) fout << title[i] << '\n', found = 1;
    if (!found) fout << "Not Found!\n";
  }

  Q.close();
}
