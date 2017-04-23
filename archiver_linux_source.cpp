#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#define error_code 2
#define unshort_int_max_val 65538
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
	if (compressing_mode)
	{
		while (i < input.length())
		{
			file_name += input[i];
			i++;
		}
	}
	return compressing_mode;
}
void compress_file(ofstream &output_file, ifstream &input_file)
{
	map<string, unsigned long int> dict;
	// initialization dict
	string temp = "";
	for (unsigned char i = 0; i < 255; i++)
	{
		temp += i;
		dict.insert(pair<string, unsigned long int>(temp, (unsigned long int)i));
		temp = "";
	}
	temp += (unsigned char)255;
	dict.insert(pair<string, unsigned long int>(temp, (unsigned long int)255));
	temp = "";
	// compressing
	vector<unsigned short int> result;
	unsigned long int max_code_in_dict = 255;
	char current_symb;
	string word = "";
	input_file.read(&current_symb, 1); //reading first byte from file
	word += current_symb;
	input_file.read(&current_symb, 1); //reading second byte from file
	while ((max_code_in_dict < unshort_int_max_val) && (!input_file.eof()))
	{
		if (dict.count(word+current_symb)) // if word in dict
		{
			word += current_symb;
		}
		else
		{
			dict.insert(pair<string, unsigned long int>((word + current_symb), ++max_code_in_dict)); // adding word to dict
			result.push_back(dict.at(word));
			word = current_symb;
		}
		input_file.read(&current_symb, 1);
	}
	result.push_back(dict.at(word));

	for (int i = 0; i < result.size(); i++)
	{
		output_file.write((char*)&(result[i]), sizeof(result[i]));
	}
	output_file.close();
	cout<<"Data has been compressed"<<endl;
}
int main()
{
	char inp[] = "--compress result.lzva input_file.txt";
	string archive_name = "";
	string filename = "";
	int result = choice_mode(inp, archive_name, filename);
	if (result == error_code)
	{
		return 1;
	}
	ifstream input_file(filename.c_str(), ios::binary | ios::in);
	if (!input_file.is_open())
	{
		cout<<"Error! File "<<filename<<" not found"<<endl;
	}
	else
	{
		ofstream archive_file(archive_name.c_str(), ios::binary | ios::out);
		compress_file(archive_file, input_file);
	}
	input_file.close();
//	system("pause");
    return 0;
}

