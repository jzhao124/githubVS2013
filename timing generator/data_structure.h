#include <vector>
#include <map>
#include "CSVParser.h"
#include <direct.h>
#include <algorithm>
using namespace std;
struct TOD
{
	int timing_id;
	int timing_from_time;
	int timing_to_time;
	int starting_phase;
	int offset; // ??what is the meaning of offset??
};
class timing
{
public:
	map<int,vector<TOD>> int_timing_id;
	map<int, map<int,int>> int_next_phase; // ??why do we define it like this??
	map<int, map<int,vector<int>>> mapping_signal_links;
	map<int, map<int, map<int,int>>> plan_dependent_gree_duration;
	map<int, map<int, int>> sig_link_TD_status; 
public:
	void read_input_files();
	void generate_timing_input();
};

void timing::read_input_files()
{
	const char* a_cwd = _getcwd(NULL, 0);
	std::string file_path(a_cwd);
	std::string file_path1(a_cwd);
	std::string file_path2(a_cwd);
	file_path1 = file_path1 + "\\input_TOD.csv";
	file_path2 = file_path2 + "\\input_timing.csv";
	CCSVParser parser,parser1;
	if (parser.OpenCSVFile(file_path1, false)) //?? why false here, should be true??
	{
		std::map<int, int> node_id_map;
		parser.m_bDataHubSingleCSVFile = true;
		while (parser.ReadRecord())  // if this line contains [] mark, then we will also read field headers.
		{
			int int_id, plan_id, from_time, to_time,starting_phase,off_set;
			vector<TOD> temp;
			string int_name;
			if (parser.GetValueByFieldName("int_id", int_id) == false)
				continue;
			if (parser.GetValueByFieldName("timing_plan_no", plan_id) == false)
				continue;
			if (parser.GetValueByFieldName("from_time", from_time) == false)
				continue;
			if (parser.GetValueByFieldName("to_time", to_time) == false)
				continue;
			if (parser.GetValueByFieldName("starting_phase", starting_phase) == false)
				continue;
			if (parser.GetValueByFieldName("off_set", off_set) == false)
				continue;
			if (int_timing_id.find(int_id) == int_timing_id.end())//this is a new intersection
			{
				int_timing_id.insert(pair<int, vector<TOD>>(int_id, temp));
				TOD temp1;
				temp1.timing_id = plan_id;
				temp1.timing_from_time = from_time;
				temp1.timing_to_time = to_time;
				temp1.starting_phase = starting_phase;
				temp1.offset = off_set;
				int_timing_id[int_id].push_back(temp1);
			}
			else
			{
				TOD temp1;
				temp1.timing_id = plan_id;
				temp1.timing_from_time = from_time;
				temp1.timing_to_time = to_time;
				temp1.starting_phase = starting_phase;
				temp1.offset = off_set;
				int_timing_id[int_id].push_back(temp1);
			}
		}
		parser.CloseCSVFile();
	}
	else
	{
		cout << "input_TOD.csv cannot be found. Please double check!\n";
	}
	if (parser1.OpenCSVFile(file_path2, false))
	{
		std::map<int, int> node_id_map;
		parser1.m_bDataHubSingleCSVFile = true;
		while (parser1.ReadRecord())  // if this line contains [] mark, then we will also read field headers.
		{
			int int_id, timing_plan, pt_phase_id, next_phase, green_duration;
			string signal_links;
			if (parser1.GetValueByFieldName("int_id", int_id) == false)
				continue;
			if (parser1.GetValueByFieldName("timing_plan_no", timing_plan) == false)
				continue;
			if (parser1.GetValueByFieldName("pt_phase_id", pt_phase_id) == false)
				continue;
			if (parser1.GetValueByFieldName("next_phase", next_phase) == false)
				continue;
			if (parser1.GetValueByFieldName("green_duration", green_duration) == false)
				continue;
			if (parser1.GetValueByFieldName("signal_links", signal_links) == false)
				continue;
			std::istringstream ss1(signal_links);
			std::string token;
			vector<int> temp_sig_link_list;
			string section_name;
			while (getline(ss1, token, ';'))
			{
				temp_sig_link_list.push_back(atoi(token.c_str()));
			}
			map<int, vector<int>> temp_phase_sig_links_map;
			temp_phase_sig_links_map.insert(pair<int, vector<int>>(pt_phase_id, temp_sig_link_list));
			
			if (mapping_signal_links.find(int_id) == mapping_signal_links.end())//new intersection
			{
				mapping_signal_links.insert(pair<int, map<int, vector<int>>>(int_id, temp_phase_sig_links_map));
			}
			else //existing 
			{
				mapping_signal_links[int_id].insert(pair<int, vector<int>>(pt_phase_id, temp_sig_link_list));
			}
			
			map<int, int> temp_next_phase_map;
			if (int_next_phase.find(int_id) == int_next_phase.end()) //new
			{
				temp_next_phase_map.insert(pair<int, int>(pt_phase_id, next_phase));
				int_next_phase.insert(pair<int, map<int, int>>(int_id, temp_next_phase_map));
			}
			else
			{
				int_next_phase[int_id].insert(pair<int, int>(pt_phase_id, next_phase));
			}

			//map<int, map<int, map<int, int>>> plan_dependent_gree_duration;

			if (plan_dependent_gree_duration.find(int_id) == plan_dependent_gree_duration.end()) //new intersection
			{
				map<int, map<int, int>> temp_plan;
				map <int, int> temp_duration;
				temp_duration.insert(pair<int, int>(pt_phase_id, green_duration));
				temp_plan.insert(pair<int, map<int, int>>(timing_plan, temp_duration));
				plan_dependent_gree_duration.insert(pair<int, map<int, map<int, int>>>(int_id, temp_plan));
			}
			else //existing intersection
			{
				if (plan_dependent_gree_duration[int_id].find(timing_plan) == plan_dependent_gree_duration[int_id].end())//new timing plan
				{
					map<int, map<int, int>> temp_plan;
					map <int, int> temp_duration;
					temp_duration.insert(pair<int, int>(pt_phase_id, green_duration));
					temp_plan.insert(pair<int, map<int, int>>(timing_plan, temp_duration));
					plan_dependent_gree_duration[int_id].insert(pair<int, map<int, int>>(timing_plan, temp_duration));
				}
				else //existing timing plan
				{
					plan_dependent_gree_duration[int_id][timing_plan].insert(pair<int, int>(pt_phase_id, green_duration));

				}
			}
		}
		parser1.CloseCSVFile();
	}
	else
	{
		cout << "input_timing.csv cannot be found. Please double check!\n";
	}
}

void timing::generate_timing_input()
{
	ofstream oFile_link;
	//oFile_link.open("input_sig_link_status.csv");
	//oFile_link << "[Sig_link_status],time,sig_link,status"<<endl;
	map<int, vector<TOD>>::iterator int_timing_id_iter;
	for (int_timing_id_iter = int_timing_id.begin(); int_timing_id_iter != int_timing_id.end(); int_timing_id_iter++)//for each intersection
	{
		if (int_timing_id[int_timing_id_iter->first].size() != mapping_signal_links[int_timing_id_iter->first].size())
		{
			//added by jun zhao for testing
			cout << int_timing_id[int_timing_id_iter->first].size() << endl;
			cout << mapping_signal_links[int_timing_id_iter->first].size() << endl;
			//end testing

			cout << "The number of total timing plans in two inputs files does not match, please check and run this program again!\n";
			system("pause");
			exit(1);
		}
		int int_id, plan_id,from_time, to_time, starting_phase,offset;
		for (int i = 0; i < (int)int_timing_id_iter->second.size(); i++)
		{			
			int_id = int_timing_id_iter->first;
			plan_id = int_timing_id_iter->second[i].timing_id;
			offset = int_timing_id_iter->second[i].offset;
			cout << "Timing Plan: " << plan_id << endl;
			from_time = int_timing_id_iter->second[i].timing_from_time;
			to_time = int_timing_id_iter->second[i].timing_to_time;
			starting_phase = int_timing_id_iter->second[i].starting_phase;
			map<int, int> phase_duration = plan_dependent_gree_duration[int_id][plan_id];
			int local_clock=0;
			int current_phase = starting_phase;
			for (int t = from_time; t < to_time; t++)
			{
				cout << "t= " << t << endl;
				map<int, vector<int>>::iterator iter;
				for (iter = mapping_signal_links[int_id].begin(); iter != mapping_signal_links[int_id].end(); iter++)
				{
					int phase_id = iter->first;
					if (phase_id == current_phase)
					{
						vector<int> sig_link_vector = mapping_signal_links[int_id][phase_id];
						//write green status into csv file
						for (int j = 0; j < (int)sig_link_vector.size(); j++)
						{
							//oFile_link << "," << t << "," << sig_link_vector[j] << "," << 1 << endl;//1 is green
							if (sig_link_TD_status.find(sig_link_vector[j]) == sig_link_TD_status.end()) //this is a new signal link
							{
								map<int, int>temp;
								temp.insert(pair<int, int>(t, 1));
								sig_link_TD_status.insert(pair<int, map<int, int>>(sig_link_vector[j], temp));
							}
							else
							{
								sig_link_TD_status[sig_link_vector[j]].insert(pair<int, int>(max(0,(t+offset)), 1));
							}
						}
					}
					else
					{
						vector<int> sig_link_vector = mapping_signal_links[int_id][phase_id];
						//write red status
						for (int j = 0; j < (int)sig_link_vector.size(); j++)
						{
							//oFile_link << "," << t << "," << sig_link_vector[j] << "," << 2 << endl;//2 is red
							if (sig_link_TD_status.find(sig_link_vector[j]) == sig_link_TD_status.end()) //this is a new signal link
							{
								map<int, int>temp;
								temp.insert(pair<int, int>(t, 1));
								sig_link_TD_status.insert(pair<int, map<int, int>>(sig_link_vector[j], temp));
							}
							else
							{
								sig_link_TD_status[sig_link_vector[j]].insert(pair<int, int>(max(0,(t + offset)), 2));
							}
						}
					}
				}
				local_clock++;
				if (local_clock >= plan_dependent_gree_duration[int_id][plan_id][current_phase])//max-out
				{
					local_clock = 0;
					current_phase = int_next_phase[int_id][current_phase];
				}
			}
		}		
	}
	//oFile_link.close();
	oFile_link.open("input_timing_status.csv");
	oFile_link << "movement_id," << "from_time_in_second," << "to_time_in_second," << "td_status,"<<endl;
	for (map<int, map<int, int>>::iterator iter = sig_link_TD_status.begin(); iter != sig_link_TD_status.end(); ++iter)
	{
		//oFile_link << iter->first << "," << "0," << "86400,";
		int old_status = 0;
		int from_time = 0;
		int to_time = 0;

		for (map<int, int>::iterator iter1 = iter->second.begin(); iter1 != iter->second.end(); ++iter1)
		{
			if (iter1->first == 1)
			{
				oFile_link << iter->first << "," << iter1->first << ",";
			}
			if (old_status == 2 && iter1->second == 1) //this phase switch on
			{
				oFile_link << iter->first << "," << iter1->first<< ",";
			}
			if (old_status == 1 && iter1->second == 2) //this phase switch off
			{
				oFile_link << iter1->first<<",1" << endl;
			}
			if(iter1!=iter->second.begin())
				old_status = iter1->second;
		}
	}
	oFile_link.close();
}