// archiver.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
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
				printf("Error in input string =(");
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
int main()
{
	string archive_name = "";
	string filename = "";
	char inp[] = "--decompress file1 file2";
	bool result = choice_mode(inp, archive_name, filename);
	system("pause");
    return 0;
}

