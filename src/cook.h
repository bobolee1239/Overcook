/* 	FILE  		: cook.h 
 *	AUTHOR		: Brian, Lee
 *	Description	: Homework 3 of Embedded Operating System
 *
 */
#pragma once
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <unistd.h>
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
		string make(const Step& s){
			//cook and wait
			for(unsigned int i=s.times; i>0; i--){
				cout << s.product << '-' << s.action << ',' << (s.times - i) << endl;
				sleep(1);	
			}

			//return product
			return s.product;
		}

		vector<string> check_n_take(const Step& s){
			vector<string> material;
			
			// check tools 
			if(s.tool != "none" && tools[s.tool] < 1){
				material.push_back("notools");
				return material;
			}

			// check storage
			map<string, int> tmp(storage);
			for(vector<string>::const_iterator itr=s.material.begin(); itr!=s.material.end(); ++itr){
				if(*itr == "none")		continue;
				else if(tmp[*itr] > 0)	--tmp[*itr];
				else					material.push_back(*itr);
			}
			
			if(!material.empty())	return material;
			
			// consume and take tools
			if(s.tool != "none"){
				--tools[s.tool];
				cout << "take " << s.tool << endl;
			}

			for(vector<string>::const_iterator itr=s.material.begin(); itr!=s.material.end(); ++itr){
				if(*itr != "none"){
					--storage[*itr];
					cout << "use " << *itr << endl;
				}
			}

			return material;
		}

		void put_storage_and_return_tools(const Step& s){
			// return product 
			map<string, int>::iterator product_pos = storage.find(s.product);

			if(product_pos != storage.end()){
				++storage[s.product];
			} else{
				storage.insert(pair<string, int>(s.product, 1));
			}
			cout << "get " << s.product << endl;

			// return tools
			if(s.tool != "none"){
				++tools[s.tool];
				cout << "return " << s.tool << endl;
			}
		}

		/**
		*	show all materials in storage : for debugging sake
		**/
		void showStorage(){
			cerr << "\n=============== STORAGE ===============" << endl;
			
			cerr << "[FOOD]" << endl;
			for(map<string, int>::iterator itr=storage.begin(); itr!=storage.end(); ++itr){
				cout << "* " << itr->first << " : " << itr->second << endl;
			}

			cerr << "[TOOL]" << endl;
			for(map<string, int>::iterator itr=tools.begin(); itr!=tools.end(); ++itr){
				cerr << "* " << itr->first << " : " << itr->second << endl;
			}
			cerr << endl;
		}

		/**
		*	push order to order list 
		**/
		void push_order(string& order){
			orders.push_back(order);
		}

		void push_orders(vector<string>& materials){
			for(vector<string>::iterator itr=materials.begin(); itr!=materials.end(); ++itr){
				orders.push_back(*itr);
			}
		}
		
		/**
		*	pop & return order from order list 
		**/
		string pop_order(){
			// check if orders empty or not
			if(orders.empty())	return string();

			string order = orders.back();
			orders.pop_back();

			return order;
		}

	private:
		Step *steps;
		map<string, int> storage;
		map<string, int> tools;
		int s_size;
		vector<string> orders;
		

};
