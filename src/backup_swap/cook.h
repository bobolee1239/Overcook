/* cook.h */
#pragma once
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <unistd.h>
#include <pthread.h>
using namespace std;

class cook {
	public:
	struct Step{
	        int index;
        	vector<string> material;
	        string tool;
	        string action;
	        unsigned int times;
	        string product;
	        void setMaterial(string m){
	                istringstream temp(m);
	                string s;
	                while(getline(temp, s, '+')){
	                        material.push_back(s);
			}
		}
	};
	
	public:
		/**
		* Read tools from file, return the amount of tools
		**/
		int setTool(char* filename){
			fstream f;
			string line;
			vector< vector<string> > data;
			f.open(filename, ios::in);
			if(!f){
				cerr << "Can't open file" << endl;
				return -1;	
			}
			while(getline(f, line)){
				string temp;
				vector<string> raw;
				istringstream temp_line(line);
				while(getline(temp_line, temp, ','))
					raw.push_back(temp);
				if(raw.size() != 2){
					cerr << filename << " format error" << endl;
					return -1;
				}
				data.push_back(raw);
			}
			int i;
			for(i=0;i<data.size();i++)
				tools.insert(pair<string, int>(data[i][0], atoi(data[i][1].c_str())));
			return data.size();
		}
		/**
		* Read orders from file, return the amount of orders
		**/
		int setOrder(char* filename){
			fstream f;
			string line;
			f.open(filename, ios::in);
			if(!f){
				cerr << "Can't open file" << endl;
				return -1;	
			}
			while(getline(f, line)){
				if(line[line.length() - 1] == '\r') 
					line = line.substr(0, line.length()-1);
				orders.push_back(line);
			}
			return orders.size();
		}
		/**
		* Read steps from file, return the amount of steps
		**/
		int setSteps(char* filename){
			fstream f;
			string line;
			vector< vector<string> > data;
			f.open(filename, ios::in);
			if(!f){
				cerr << "Can't open file" << endl;
				return -1;	
			}
			while(getline(f, line)){
			//read from file 
				string temp;
				vector<string> raw;

				// remove '\r' character 
				if(line[line.length() - 1] == '\r')
					line = line.substr(0, line.length()-1);
				
				istringstream temp_line(line);
				while(getline(temp_line, temp, ','))
					raw.push_back(temp);
				if(raw.size() != 6){
					cerr << filename << " format error" << endl;
					return -1;
				}
				data.push_back(raw);
			}
			//convert to struct
			s_size = data.size();
			steps = new Step[s_size];
			int i;
			for(i=0;i<s_size;i++){
				steps[i].index = atoi(data[i][0].c_str());
				steps[i].setMaterial(data[i][1]);
				steps[i].tool = data[i][2];
				steps[i].action = data[i][3];
				steps[i].times = atoi(data[i][4].c_str());
				steps[i].product = data[i][5];
				storage.insert(pair<string, int>(steps[i].product, 0));
			}
			return s_size;
		}
		/**
		* Get one order from the order list and remove it from the list
		**/
		string getOrder(){
			string order;
			cin >> order;

			if(order == "timeout"){
				cout << "end" << endl;
				return string();
			}

			return order;
		}
		/**
		* Find the Step that will produce specific 'product'
		**/
		const Step* find(string product){
			int i;
			for(i=0;i<s_size;i++){
				if(steps[i].product == product)
					return &steps[i];
			}
			cerr << "Can't find step for " << product << endl;
			return NULL;
		}
		/**
		* Making the product in this function. 
		* Check if there are enough storage to make, then remove those from the storage
		* if not, return the lacking material
		* then check if the tool is avaliable. 
		*At last, cook it and wait for it. 
		*Add the product to the storage and return its name
		**/ 
		vector<string> check_storage(const Step *s){
			vector<string> material;

			//check tools first
			if(s->tool != "none" && tools[s->tool] < 1){
//				cout << "NO TOOL: " << s->tool << endl;
//				sleep(1);
				material.push_back("notools");
				return material;
			}
			//check material storage
			map<string, int> tmp(storage);
			for(vector<string>::const_iterator itr=(s->material).begin(); itr!=(s->material).end(); ++itr){
				if (*itr == "none" || tmp[*itr] > 0){
					--tmp[*itr];
					continue;
				} else {
					//return missing material if it is not enough
					material.push_back(*itr);
				}
			}

			return material;

		
		}
		
		void consume_and_take(const Step *s){
			//consume material
			for(vector<string>::const_iterator itr=(s->material).begin(); itr!=(s->material).end(); ++itr){
				if(*itr != "none"){
					--storage[*itr];
					cout << "use " << *itr << endl;
				}
			}
			
			//take tool
			if(s->tool != "none"){
				--tools[s->tool];
				cout << "take " << s->tool << endl;
			}
		
		}
		void put_storage_and_return(const Step *s){
			//return product
			map<string, int>::iterator product_pos = storage.find(s->product);
			if(product_pos != storage.end()){
				++storage[s->product];
			} else {
				storage.insert(pair<string, int>(s->product, 1));
			}
			cout << "get " << s->product << endl;

			if(s->tool != "none"){
				++tools[s->tool];
				cout << "return " << s->tool << endl; 
			}
			
		}

		string make(const Step *s){
			//cook and wait
			for(unsigned int i=s->times; i>0; i--){
				cout << s->product << '-' << s->action << ", " << s->times - i << endl;
				sleep(1);
			}

			return s->product;
		}
		
		void showStorage(){
			cout << "========== STORAGE ==========\n";
			for(map<string, int>::iterator itr=storage.begin(); itr!=storage.end(); ++itr){
				cout << " * " << itr->first << ": " << itr->second << endl;
			}
			cout << endl;
		}

		~cook(){
			delete [] steps;
		}
		cook(){
		}

	private:
		Step *steps;
		map<string, int> storage;
		map<string, int> tools;
		int s_size;
		vector<string> orders;
};
