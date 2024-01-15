#define FILE_EXTENSION ".txt"
#include<fstream>
#include<string>
#include<cstring>
#include<vector>
#include<iostream>
#include <set>
#include <algorithm>
#include <iterator>
#include <ctime>
using namespace std;

// Utility Func

struct TrieNode{
	TrieNode *child[26];
	int word_end;
};

TrieNode* NEWnode(){
	TrieNode* node = new TrieNode;
	for(int i=0;i<26;i++){
		node->child[i] = NULL;
	}
	node->word_end = 0;

	return node;
}

void insert(TrieNode *root, string word){
	TrieNode *node = root;

	for(int i=0;i<word.length();i++){
		int index;
		if(isupper(word[i])) index = word[i] - 'A';
		else index = word[i] - 'a';

		if(!node->child[index]){
			node->child[index] = NEWnode();
		}
		node = node->child[index];
	}
	node->word_end = 1;
}

void insert_rev(TrieNode *root, string word){
	reverse(word.begin(), word.end());
	TrieNode *node = root;

	for(int i=0;i<word.length();i++){
		int index;
		if(isupper(word[i])) index = word[i] - 'A';
		else index = word[i] - 'a';

		if(!node->child[index]){
			node->child[index] = NEWnode();
		}
		node = node->child[index];
	}
	node->word_end = 1;
}

int search_pre(TrieNode *root,string word){
	TrieNode *node = root;

	for(int i=0;i<word.length();i++){
		int index;
		if(isupper(word[i])) index = word[i] - 'A';
		else index = word[i] - 'a';

		if(!node->child[index]){
			return 0;
		}
		node = node->child[index];
	}
	return 1;
}

int search_exa(TrieNode *root,string word){
	TrieNode *node = root;

	for(int i=1;i<word.length()-1;i++){
		int index;
		if(isupper(word[i])) index = word[i] - 'A';
		else index = word[i] - 'a';

		if(!node->child[index]){
			return 0;
		}
		node = node->child[index];
	}
	return node->word_end;
}

int search_suf(TrieNode *root,string word){
	reverse(word.begin(), word.end());
	TrieNode *node = root;

	for(int i=1;i<word.length()-1;i++){
		int index;
		if(isupper(word[i])) index = word[i] - 'A';
		else index = word[i] - 'a';

		if(!node->child[index]){
			return 0;
		}
		node = node->child[index];
	}
	return 1;
}

int search_wil(TrieNode *root,string word,int where){
	TrieNode *node = root;

	if(where == word.size()-1) return node->word_end;
	else if(word[where]!='*'){
		if(islower(word[where]) && node->child[word[where]-'a']){
			return search_wil(node->child[word[where]-'a'], word, where+1);
		}else if(isupper(word[where]) && node->child[word[where]-'A']){
			return search_wil(node->child[word[where]-'A'], word, where+1);
		}else return 0;
	}else{
		if(where == word.size()-2) return 1;
		int word_num;
		if(isupper(word[where+1])) word_num = word[where+1] - 'A';
		else word_num = word[where+1] - 'a';

		int T_F=0, F_T;
		if(node->child[word_num]) T_F = search_wil(node->child[word_num], word, where+2);
		for(int i=0;i<26;i++){
			if(node->child[i]){
				F_T = search_wil(node->child[i], word, where);
				T_F = T_F | F_T;
			}
		}
		return T_F;
	}
}


// string parser : output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string){
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

TrieNode *Pre_tree[10005];
TrieNode *Suf_tree[10005];

int main(int argc, char *argv[])
{

    // INPUT :
	// 1. data directory in data folder
	// 2. number of txt files
	// 3. output route

    string data_dir = argv[1] + string("/");
	string query = string(argv[2]);
	string output = string(argv[3]);

	// Read File & Parser Example


	string file, title_name, tmp;
	fstream fin, fquery;
	ofstream fout;
	vector<string> tmp_string;
	vector<string> output_;
	int data_number;

	// from data_dir get file ....
	// eg : use 0.txt in data directory

	for(data_number=0;;data_number++){
		fin.open(data_dir + to_string(data_number) + FILE_EXTENSION, ios::in);
		if(fin.fail()) break;

		// GET TITLENAME
		getline(fin, title_name);
		output_.push_back(title_name);
		Pre_tree[data_number] = NEWnode();
		Suf_tree[data_number] = NEWnode();

    	// GET TITLENAME WORD ARRAY
    	tmp_string = split(title_name, " ");
		vector<string> title = word_parse(tmp_string);

		
		for(auto &i:title){
			insert(Pre_tree[data_number], i);
			insert_rev(Suf_tree[data_number], i);
		}

		// GET CONTENT LINE BY LINE
		while(getline(fin, tmp)){

        	// GET CONTENT WORD VECTOR
			tmp_string = split(tmp, " ");

			// PARSE CONTENT
			vector<string> content = word_parse(tmp_string);

			for(auto &i : content){
				insert(Pre_tree[data_number], i);
				insert_rev(Suf_tree[data_number], i);
			}
		}
		fin.close();
	}

	fquery.open(query,ios::in);
	fout.open(output,ios::out);
	set<int> The_End; //this is my life
	while(getline(fquery, tmp)){
		tmp_string = split(tmp, " ");

		for(auto i=tmp_string.begin();i !=tmp_string.end();i++){
			if(*i=="+"){
				i++;
				if((*i)[0]=='"'){
					// vector<string> content = word_parse(&i);
					set<int> regis = The_End;
					for(int j=0;j<data_number;j++){
						if(search_exa(Pre_tree[j],*i)) regis.erase(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else if((*i)[0]=='*'){
					set<int> regis = The_End;
					for(int j=0;j<data_number;j++){
						if(search_suf(Suf_tree[j],*i)) regis.erase(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else if((*i)[0]=='<'){
					set<int> regis = The_End;
					for(int j=0;j<data_number;j++){
						if(search_wil(Pre_tree[j],*i,1)) regis.erase(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else{
					set<int> regis = The_End;
					for(int j=0;j<data_number;j++){
						if(search_pre(Pre_tree[j],*i)) regis.erase(j);
					}
					for(auto j:regis) The_End.erase(j);
				}
			} else if(*i=="-"){
				i++;
				if((*i)[0]=='"'){
					// vector<string> content = word_parse(&i);
					set<int> regis;
					for(int j=0;j<data_number;j++){
						if(search_exa(Pre_tree[j],*i)) regis.insert(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else if((*i)[0]=='*'){
					// vector<string> content = word_parse(&i);
					set<int> regis;
					for(int j=0;j<data_number;j++){
						if(search_suf(Suf_tree[j],*i)) regis.insert(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else if((*i)[0]=='<'){
					// vector<string> content = word_parse(&i);
					set<int> regis;
					for(int j=0;j<data_number;j++){
						if(search_wil(Pre_tree[j],*i,1)) regis.insert(j);
					}
					for(auto j:regis) The_End.erase(j);
				} else{
					// vector<string> content = word_parse(&i);
					set<int> regis;
					for(int j=0;j<data_number;j++){
						if(search_pre(Pre_tree[j],*i)) regis.insert(j);
					}
					for(auto j:regis) The_End.erase(j);
				} 
			} else if(*i=="/"){
				i++;
				if((*i)[0]=='"'){
					for(int j=0;j<data_number;j++){
						if(search_exa(Pre_tree[j],*i)) The_End.insert(j);
					}
				} else if((*i)[0]=='*'){
					for(int j=0;j<data_number;j++){
						if(search_suf(Suf_tree[j],*i)) The_End.insert(j);
					}
				} else if((*i)[0]=='<'){
					for(int j=0;j<data_number;j++){
						if(search_wil(Pre_tree[j],*i,1)) The_End.insert(j);
					}
				} else{
					for(int j=0;j<data_number;j++){
						if(search_pre(Pre_tree[j],*i)) The_End.insert(j);
					}
				}
			} else{
				if((*i)[0]=='"'){
					for(int j=0;j<data_number;j++){
						if(search_exa(Pre_tree[j],*i)) The_End.insert(j);
					}
				} else if((*i)[0]=='*'){
					for(int j=0;j<data_number;j++){
						if(search_suf(Suf_tree[j],*i)) The_End.insert(j);
					}
				} else if((*i)[0]=='<'){
					for(int j=0;j<data_number;j++){
						if(search_wil(Pre_tree[j],*i,1)) The_End.insert(j);
					}
				} else{
					for(int j=0;j<data_number;j++){
						if(search_pre(Pre_tree[j],*i)) The_End.insert(j);
					}
				}
			}
		}
		if(The_End.empty()){
			fout << "Not Found!" << '\n';
		} else{
			for(auto &i:The_End){
				fout << output_[i] << '\n';
				// fout << output_[i] << " " << i << endl;
			}
		}
		The_End.clear();
	}
	fquery.close();
	fout.close();

    



	

	// for(auto &word : title){
	// 	cout << word << endl;
	// }

    

    // CLOSE FILE
	
}


// 1. UPPERCASE CHARACTER & LOWERCASE CHARACTER ARE SEEN AS SAME.
// 2. FOR SPECIAL CHARACTER OR DIGITS IN CONTENT OR TITLE -> PLEASE JUST IGNORE, YOU WONT NEED TO CONSIDER IT.
//    EG : "AB?AB" WILL BE SEEN AS "ABAB", "I AM SO SURPRISE!" WILL BE SEEN AS WORD ARRAY AS ["I", "AM", "SO", "SURPRISE"].
// 3. THE OPERATOR IN "QUERY.TXT" IS LEFT ASSOCIATIVE
//    EG : A + B / C == (A + B) / C

//

//////////////////////////////////////////////////////////

