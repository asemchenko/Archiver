// archiver.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
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
enum mode_type
{
	compressing = 1,
	decompressing = 0,
	error = 2
};
mode_type choice_mode(char* input_str, string &archive_name, string &file_name)
{
	string input = string(input_str);
	string temp = "";
	mode_type compressing_mode;
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
				compressing_mode = compressing;
				break;
			}
			else if (temp == "--decompress")
			{
				compressing_mode = decompressing;
				break;
			}
			else
			{
				printf("Error in input string with parameters =(\n");
				return error;
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
vector<unsigned short int> compress_uint2(ifstream &input_file, map<string, unsigned long int> &dictionary, unsigned long int &max_val_in_dict)
{
	vector<unsigned short int> result;
	char current_symb;
	string word = "";
	input_file.read(&current_symb, 1); //reading first byte from file
	word += current_symb;
	input_file.read(&current_symb, 1); //reading second byte from file
	while ((max_val_in_dict < unshort_int_max_val) && (!input_file.eof()))
	{
		if (dictionary.count(word + current_symb)) // if word in dict
		{
			word += current_symb;
		}
		else
		{
			dictionary.insert(pair<string, unsigned long int>((word + current_symb), ++max_val_in_dict)); // adding word to dict
			result.push_back(dictionary.at(word));
			word = current_symb;
		}
		input_file.read(&current_symb, 1);
	}
	if (input_file.eof() && (max_val_in_dict < unshort_int_max_val)) // if file ended, but we use 16 bites code yet
	{
		result.push_back(dictionary.at(word));
	}
	else // if we should use 32 bites to code last word
	{
		unsigned short int high_shift, low_shift;
		high_shift = (unsigned short int)(dictionary.at(word) >> 16);
		low_shift = (unsigned short int)(dictionary.at(word));
		result.push_back(low_shift);
		result.push_back(high_shift);
	}
	return result;
}
vector<unsigned long int> compress_uint4(ifstream &input_file, map<string, unsigned long int> &dictionary, unsigned long int &max_val_in_dict)
{
	vector<unsigned long int> result;
	char current_symb;
	string word = "";
	input_file.read(&current_symb, 1); //reading first byte from file
	word += current_symb;
	input_file.read(&current_symb, 1); //reading second byte from file
	while ((max_val_in_dict < unshort_int_max_val) && (!input_file.eof()))
	{
		if (dictionary.count(word + current_symb)) // if word in dict
		{
			word += current_symb;
		}
		else
		{
			dictionary.insert(pair<string, unsigned long int>((word + current_symb), ++max_val_in_dict)); // adding word to dict
			result.push_back(dictionary.at(word));
			word = current_symb;
		}
		input_file.read(&current_symb, 1);
	}
	result.push_back(dictionary.at(word));
	return result;
}
unsigned long int compress_file(ofstream &output_file, ifstream &input_file, unsigned long int size_input_data)
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
	unsigned long int max_code_in_dict = 255;
		//compressing using 16 bites codes
	vector<unsigned short int> result_uint2 = compress_uint2(input_file, dict, max_code_in_dict);
	bool used_uint4 = false;
	vector<unsigned long int> result_uint4;
	if ( (max_code_in_dict >= unshort_int_max_val) && (!input_file.eof()) ) // if file is not ended, but we should use 32 bites codes now
	{
		result_uint4 = compress_uint4(input_file, dict, max_code_in_dict);
		used_uint4 = true;
	}
	unsigned long long int size_compressed_data = 2 * result_uint2.size() + (used_uint4 == true ? result_uint4.size() : 0);

	// writing coded data to archive
	if (size_compressed_data <= size_input_data)
	{
		for (unsigned long int i = 0; i < result_uint2.size(); i++)
		{
			output_file.write((char*)&(result_uint2[i]), sizeof(result_uint2[i]));
		}
		if (used_uint4)
		{
			for (unsigned long i = 0; i < result_uint4.size(); i++)
			{
				output_file.write((char*)&(result_uint4[i]), sizeof(result_uint4[i]));
			}
		}
	}
	else // writing non-coded data to archive
	{
		input_file.clear();
		input_file.seekg(0);
		char temp_byte;
		input_file.read(&temp_byte, 1);
		while (!input_file.eof())
		{
			output_file.write(&temp_byte, 1);
			input_file.read(&temp_byte, 1);
		}
		output_file.seekp(ios::beg);
		bool is_compressed = false;
		output_file.write((char*)&is_compressed, sizeof(is_compressed)); // setting flag that we haven't compressing data
	}
	output_file.close();
	return size_compressed_data <= size_input_data ? size_compressed_data:size_input_data;
}
void write_string_to_file(string &input_str, ofstream &file)
{
	unsigned int len = input_str.length() + 1;
	file.write((char*)&len, sizeof(len));
	for (int i = 0; i < len; i++)
	{
		file.write(&(input_str.c_str()[i]), 1);
	}
}
unsigned long int get_file_size(string filename)
{
		FILE *input_file = fopen(filename.c_str(), "rb");
		fseek(input_file, 0, SEEK_END);
		unsigned long int size_input_file = ftell(input_file);
		fclose(input_file);
		return size_input_file;
}
int main()
{
	char inp[] = "--compress result.lzva input_file.txt";
	string archive_name = "";
	string filename = "";
	mode_type mode = choice_mode(inp, archive_name, filename);
	if (mode == error)
	{
		return 1;
	}
	// getting size of input file
	unsigned long int size_input_file = get_file_size(filename);
	ifstream input_file(filename.c_str(), ios::binary | ios::in);
	if (!input_file.is_open())
	{
		cout<<"Error! File "<<filename<<" not found"<<endl;
	}
	else
	{
		if (mode == compressing)
		{
			ofstream archive_file(archive_name.c_str(), ios::binary | ios::out);
			bool is_compressed = true;
			archive_file.write((char*)&is_compressed, sizeof(is_compressed));
			write_string_to_file(filename, archive_file);
			printf("Starting compressing ...\n");
			unsigned long int size_data = compress_file(archive_file, input_file,size_input_file);
			printf("Data have been compressed\n");
			cout << "Size compressed part: " << size_data <<" bytes"<< endl;
		}
	}
	input_file.close();
	system("pause");
    return 0;
}

