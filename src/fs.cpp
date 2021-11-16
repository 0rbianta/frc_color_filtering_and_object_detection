#include "include/fs.h"

bool is_file_exist(string fpath)
{
    ifstream file(fpath);
    bool result = file.is_open() && file.good();
    file.close();
    return result;
}


string read_file(string fpath)
{
	ifstream file(fpath);
	string fline;
	string ret;


	if (!is_file_exist(fpath))
		return "";
    
    while(getline(file, fline))
    	ret += fline + "\n";


	file.close();


	return ret;
}

