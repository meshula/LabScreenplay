
// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#include "OptionParser.h"
#include "Screenplay.h"

#include <string>
#include <iostream>
#include <fstream>
#include <set>

using namespace std;

std::string path;

void stringcallback(const std::string& str)
{
    path = str;
}


int main(int argc, char** argv) try
{
	std::cout << "LabScreenplay 20171202.1850" << "\n";

	char * text = nullptr;
    char * end = nullptr;

    OptionParser op("screenplay");
    op.StringCallback(stringcallback, "file to parse");

	if (op.Parse(argc, argv))
	{
        if (path.length() == 0) {
            op.Usage();
            exit(1);
        }

        FILE* f = fopen(path.c_str(), "rb");
		if (!f) {
			std::cout << path << " not found" << std::endl;
			exit(1);
		}

		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		fseek(f, 0, SEEK_SET);
		text = new char[len + 1];
		fread(text, 1, len, f);
		text[len] = '\0';
		fclose(f);
        end = text + len;
    }

    if (!text) {
        std::cerr << "could not read " << path << endl;
		exit(1);
    }

	string scriptText(text, end);
	delete[] text;
	lab::Script script = lab:: Script::parseFountain(scriptText);

	std::ofstream out("C:\\tmp\\test.fountain");
	for (auto& node : script.title.nodes)
		out << node.as_string() << "\n";
	out << "\n\n";
	for (auto& seq : script.sequences)
	{
		out << seq.as_string() << "\n\n";
		for (auto& node : seq.nodes)
			out << node.as_string() << "\n";
	}


	lab::ScriptMeta meta(script);

    std::cout << "\nSummary:\n";
    std::cout << "----------------------------------------------------\n";

	std::cout << "Location count: " << script.sets.size() << "\n";
	std::cout << "Character count: " << script.characters.size() << "\n";
	std::cout << "Sequence count:" << script.sequences.size() << "\n";

    std::cout << "\nLocations:\n";
    std::cout << "----------------------------------------------------\n";

    for (auto& l : script.sets)
        std::cout << l << "\n";

    std::cout << "\nSequences:\n";
    std::cout << "----------------------------------------------------\n";

	for (auto& sc : meta.sequence_characters)
	{
		int idx = script.sequence_index[sc.first];
		string location = script.sequences[idx].location;
		std::cout << "Sequence: " << sc.first << " - " << location << "\n";
		for (auto& c : sc.second)
		{
			std::cout << "   " << c << "\n";
		}
	}
	std::cout << "\n";
    
    std::cout << "\nCharacters:\n";
    std::cout << "----------------------------------------------------\n";
    
    for (auto& cd : meta.character_dialog)
	{
		std::cout << "Character: " << cd.first << ", line count: " << cd.second.size() << "\n";
	}
	std::cout << std::endl;

    return 0;
}
catch (...)
{
	std::cerr << "Problem encountered" << std::endl;
	return 1;
}

