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
#include <stdint.h>
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
vector<uint16_t> compress_uint2(ifstream &input_file, map<string, uint32_t> &dictionary, unsigned long int &max_val_in_dict)
{
	vector<uint16_t> result;
	uint8_t current_symb;
	string word = "";
	input_file.read((char*)&current_symb, 1); //reading first byte from file
	word += (char)current_symb;
	input_file.read((char*)&current_symb, 1); //reading second byte from file
	while ((max_val_in_dict < unshort_int_max_val) && (!input_file.eof()))
	{
		if (dictionary.count(word + (char)current_symb)) // if word in dict
		{
			word += (char)current_symb;
		}
		else
		{
			dictionary.insert(pair<string, uint16_t>((word + (char)current_symb), ++max_val_in_dict)); // adding word to dict
			result.push_back(dictionary.at(word));
			word = (char)current_symb;
		}
		input_file.read((char*)&current_symb, 1);
	}
	if (input_file.eof() && (max_val_in_dict < unshort_int_max_val)) // if file ended, but we use 16 bites code yet
	{
		result.push_back(dictionary.at(word));
	}
	else // if we should use 32 bites to code last word
	{
		uint16_t high_shift, low_shift;
		high_shift = (uint16_t)(dictionary.at(word) >> 16);
		low_shift = (uint16_t)(dictionary.at(word));
		result.push_back(low_shift);
		result.push_back(high_shift);
	}
	return result;
}
vector<uint32_t> compress_uint4(ifstream &input_file, map<string, uint32_t> &dictionary, unsigned long int &max_val_in_dict)
{
	vector<uint32_t> result;
	uint8_t current_symb;
	string word = "";
	input_file.read((char*)&current_symb, 1); //reading first byte from file
	word += (char)current_symb;
	input_file.read((char*)&current_symb, 1); //reading second byte from file
	while ((max_val_in_dict < unshort_int_max_val) && (!input_file.eof()))
	{
		if (dictionary.count(word + (char)current_symb)) // if word in dict
		{
			word += (char)current_symb;
		}
		else
		{
			dictionary.insert(pair<string, unsigned long int>((word + (char)current_symb), ++max_val_in_dict)); // adding word to dict
			result.push_back(dictionary.at(word));
			word = (char)current_symb;
		}
		input_file.read((char*)&current_symb, 1);
	}
	result.push_back(dictionary.at(word));
	return result;
}
void write_to_vector(uint32_t number, int size, vector<uint16_t> &vect)
{
	switch (size)
	{
	case 2:
		vect.push_back((uint16_t)number);
		break;
	case 4:
		vect.push_back((uint16_t)number);
		vect.push_back((uint16_t)(number >> 8));
		break;
	case 8:
		vect.push_back((uint16_t)number);
		vect.push_back((uint16_t)(number >> 8));
		vect.push_back((uint16_t)(number >> 16));
		vect.push_back((uint16_t)(number >> 24));
		break;
	default:
		break;
	}

}
unsigned long int compress_file(ofstream &output_file, ifstream &input_file, unsigned long int size_input_data)
{
	map<string, uint32_t> dict;
	// initialization dict
	string temp = "";
	for (unsigned int i = 0; i < 256; i++)
	{
		temp += (char)i;
		dict.insert(pair<string,uint32_t>(temp, (uint32_t)i));
		temp = "";
	}
	// compressing
	uint32_t max_code_in_dict = 255;
	//compressing using 16 bites codes
	vector<uint16_t> result;
	uint8_t current_symb;
	string word = "";
	input_file.read((char*)&current_symb, 1); //reading first byte from file
	word += (char)current_symb;
	input_file.read((char*)&current_symb, 1); //reading second byte from file
	uint32_t code_lenght = 2;
	uint32_t count_readed_bytes = 2;
	while ((max_code_in_dict < UINT16_MAX + 1) && (!input_file.eof()))
	{
		if (dict.count(word + (char)current_symb)) // if word in dict
		{
			word += (char)current_symb;
		}
		else
		{
			dict.insert(pair<string, uint32_t>((word + (char)current_symb), ++max_code_in_dict));
			//cout << result.size() << endl;// adding word to dict
			write_to_vector(dict.at(word), code_lenght, result);
			//result.push_back(dict.at(word));
			word = (char)current_symb;
		}
		input_file.read((char*)&current_symb, 1);
		count_readed_bytes++;
	}
	code_lenght = 4;
	while ((max_code_in_dict < UINT32_MAX + 1) && (!input_file.eof()))
	{
		if (dict.count(word + (char)current_symb)) // if word in dict
		{
			word += (char)current_symb;
		}
		else
		{
			dict.insert(pair<string, uint32_t>((word + (char)current_symb), ++max_code_in_dict));
			//cout << result.size() << endl;// adding word to dict
			write_to_vector(dict.at(word), code_lenght, result);
			//result.push_back(dict.at(word));
			word = (char)current_symb;
		}
		input_file.read((char*)&current_symb, 1);
	}
	write_to_vector(dict.at(word), code_lenght, result);
	unsigned long long int size_compressed_data = 2 * result.size();

	// writing coded data to archive
	if (size_compressed_data <= size_input_data)
	{
		for (unsigned long int i = 0; i < result.size(); i++)
		{
			output_file.write((char*)&(result[i]), sizeof(result[i]));
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
	return size_compressed_data <= size_input_data ? size_compressed_data : size_input_data;
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
int main(int argvc,char* argv[])
{
	if (argvc  > 0)
	{
		char* inp = "--compress book dm_.pdf";//argv[1];
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
			cout << "Error! File " << filename << " not found" << endl;
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
				unsigned long int size_data = compress_file(archive_file, input_file, size_input_file);
				printf("Data have been compressed\n");
				cout << "Size compressed part: " << size_data << " bytes" << endl;
			}
		}
		input_file.close();
	}
	else
	{
		printf("Error, no arguments\n");
	}
	system("pause");
    return 0;
}

