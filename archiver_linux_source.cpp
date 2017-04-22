//#include "stdafx.h"
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <iostream>
#define error_code 2
using namespace std;
int choice_mode(char* input_str, string &archive_name, string &file_name)
{
	string input = string(input_str);
	string temp = "";
	bool compressing_mode;
	int i = 0;
	// reading mode (compress/decompress)
	while (i < input.length())
	{
		temp += input[i];
		i++;
		if (input[i] == ' ')
		{
			if (temp == "--compress")
			{
				compressing_mode = true;
				break;
			}
			else if (temp == "--decompress")
			{
				compressing_mode = false;
				break;
			}
			else
			{
				printf("Error in input string with parameters =(\n");
				return error_code;
			}
		}
	}
	i++;
	while (i < input.length())
	{
		if (input[i] != ' ')
		{
			archive_name += input[i];
		}
		else
		{
			break;
		}
		i++;
	}
	i++;
	if (!compressing_mode)
	{
		while (i < input.length())
		{
			file_name += input[i];
			i++;
		}
	}
	return compressing_mode;
}
int main(int argc,char *argv[])
{
	if (argc < 2)
	{
		printf("Error! No parameters!\n");
		return 1;
	}
	char inp[] = "--recompress file1 file2";
	string archive_name = "";
	string filename = "";
	int result = choice_mode(argv[1], archive_name, filename);
	if (result == error_code)
	{
		return 1;
	}
	ifstream input_file(filename.c_str(), ios::binary | ios::in);
	if (!input_file.is_open())
	{
		printf("Error! File %s not found\n",filename.c_str());
	}
	else
	{
		ofstream archive_file(archive_name.c_str(), ios::binary | ios::out);
		
		archive_file.close();
	}
	input_file.close();
	//system("pause");
    return 0;
}

