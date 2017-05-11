#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <stdint.h>
using namespace std;
enum mode_type
{
	compressing = 1,
	decompressing = 0,
	error = 2
};
uint8_t write_to_vect(uint32_t number, int count_bites, vector<uint8_t> &vect)
{
	// эта функция переразбивает число number на биты так чтобы можно было записать его в вектор vect вместе с остатками битов,
	// которые хранятся  буффере от прошлого числа. Ту часть number которую не удалось вместить мы оставляем в буффере до следующего раза
	static uint8_t buffer = 0;
	static int free_space_buffer = 8;
	uint8_t high_shift = number >> (count_bites - free_space_buffer); // getting high free_space_buffer bites from number
	count_bites -= free_space_buffer;
	vect.push_back(buffer | high_shift); // writing buffer bits + high_shift bits to vect
										 // пишем в вектор оставшиеся БАЙТЫ числа number
	while (count_bites > 7)
	{
		count_bites -= 8;
		vect.push_back((uint8_t)(number >> (count_bites)));
	}
	// пишем в буффер (в его старшие разряды) оставшиеся БИТЫ числа number
	buffer = number << (8 - count_bites);
	free_space_buffer = 8 - count_bites;
	return free_space_buffer;
}
unsigned long int compress_file(ofstream &output_file, ifstream &input_file, unsigned long int size_input_data)
{
	map<string, uint32_t> dict;
	// initialization dict
	string temp = "";
	for (unsigned int i = 0; i < 256; i++)
	{
		temp += (char)i;
		dict.insert(pair<string, uint32_t>(temp, (uint32_t)i));
		temp = "";
	}
	// compressing
	uint32_t max_code_in_dict = 255;
	vector<uint8_t> result;
	uint8_t current_symb;
	string word = "";
	input_file.read((char*)&current_symb, 1); //reading first byte from file
	word += (char)current_symb;
	input_file.read((char*)&current_symb, 1); //reading second byte from file
	uint32_t code_lenght = 9;
	uint32_t count_readed_bytes = 2;
	uint32_t j;
	while (!input_file.eof())
	{
		j = 0;
		uint32_t max = 1 << (code_lenght - 1); // 2^(code_lenght - 1). ^ - pow, not XOR
		std::cout << "Left: " << size_input_data - count_readed_bytes << " bytes" << '\n';
		// coding words using code_leght bites code
		// таким немного сложным способом я отслеживаю когда нужно увеличить длину кода. Если мы начинаем кодировать n-битными кодами, то через 2^(n-1) добавлений в словарь 
		// прийдеться увеличить длину кода
		while ((j < max) && (!input_file.eof()))
		{
			if (dict.count(word + (char)current_symb)) // if word in dict
			{
				word += (char)current_symb;
			}
			else
			{
				dict.insert(pair<string, uint32_t>((word + (char)current_symb), ++max_code_in_dict));
				j++;
				write_to_vect(dict.at(word), code_lenght, result);
				word = (char)current_symb;
			}
			input_file.read((char*)&current_symb, 1);
			count_readed_bytes++;
		}
		// starting use (code_lenght + 1) bites code
		code_lenght++;
	}
	// if file ended, but we should use (code_lenght-2) bites code yet. We must'nt increment code_lenght, but we make it
	// обработка ситуации когда файл закончился до того во время работы цикла while ((j < max) && (!input_file.eof())). При этом выходя из цикла мы увеличили code_lenght на 1
	// хотя делать этого не следовало. Так что просто отнимаем еденицу от code_lenght. Еще одну еденицу мы должны отнять и так.
	if (j < (1 << (code_lenght - 2))) // j < 2^(code_lenght - 2). ^ - pow, not XOR
	{
		code_lenght--;
	}
	uint8_t free_bits_in_buffer = write_to_vect(dict.at(word), code_lenght, result);
	if (free_bits_in_buffer < 8) // if buffer is not empty
	{
		write_to_vect(0, free_bits_in_buffer, result); // writing buffer to result
	}
	unsigned long long int size_compressed_data = result.size();
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
	std::cout << "\n\n" << '\n';
	return size_compressed_data <= size_input_data ? size_compressed_data : size_input_data;
}
void write_string_to_file(string &input_str, ofstream &file)
{
	uint8_t len = input_str.length() + 1;
	file.write((char*)&len, sizeof(len));
	for (int i = 0; i < len; i++)
	{
		file.write(&(input_str.c_str()[i]), 1);
	}
}
unsigned long int get_file_size(FILE* &input_file)
{
	fseek(input_file, 0, SEEK_END);
	unsigned long int size_input_file = ftell(input_file);
	fclose(input_file);
	return size_input_file;
}
int main(int argvc, char* argv[])
{
	if (argvc  > 1)
	{
		string mode_str = string(argv[1]);
		string archive_name = "";
		string filename = "";
		mode_type mode;
		if ((mode_str == "--compress") || (mode_str == "-c"))
		{
			if (argvc == 4)
			{
				archive_name = string(argv[2]);
				filename = string(argv[3]);
				mode = compressing;
			}
			else
			{
				printf("Error! Error in count operands for mode --compress\n");
				mode = error;
			}
		}
		else if ((mode_str == "--decompress") || (mode_str == "-d"))
		{
			if (argvc == 3)
			{
				archive_name = string(argv[2]);
				mode = decompressing;
			}
			else
			{
				printf("Error in count operands for mode --decompress\n");
				mode = error;
			}
		}
		else
		{
			printf("Error! There is no mode: %s\n", mode_str.c_str());
			mode = error;
		}
		if (mode == error)
		{
			return 1;
		}
		// getting size of input file
		FILE *file = fopen(filename.c_str(), "rb");
		if (!file)
		{
			printf("Cannot open file %s\n", filename.c_str());
			return 1;
		}
		unsigned long int size_input_file = get_file_size(file);
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
				cout << "Size input file: " << size_input_file << endl;
				cout << "Size archive:    " << size_data + 3 + filename.length() << " bytes" << endl;
				cout << "Compression koeficient: " << (((float)size_input_file) / (size_data + 3 + filename.length())) << endl;
				
			}
		}
		input_file.close();
	}
	else
	{
		printf("Error, no arguments\n");
	}
	return 0;
}
