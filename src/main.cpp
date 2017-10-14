
// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#include "OptionParser.h"
#include "Screenplay.h"

#include <LabText/TextScanner.h>
#include <LabText/TextScanner.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <set>

using namespace std;

#ifdef _MSC_VER
#define strnicmp _strnicmp
#endif

using TextScanner::StripLeadingWhitespace;
using TextScanner::StripLeadingWhitespace;

using lab::NodeKind;
using lab::Script;
using lab::ScriptNode;
using lab::Sequence;

std::string path;

void stringcallback(const std::string& str)
{
    path = str;
}

bool isLineContinuation(const string & s)
{
	if (s.length() < 1)
		return false;
	return s[0] == '\t' || (s[0] == ' ');
}

string parseValue(const string & s, const vector<string> & lines, size_t & i)
{
	string result;
	size_t off = s.find(':');
	if (off != string::npos)
		result = s.substr(off);

	size_t line = i+1;
	while (isLineContinuation(lines[line])) {
		result += lines[line];
		++line;
	}
	i = line-1;
	return StripLeadingWhitespace(result);
}

bool lineIsUpperCase(const char * curr, const char * end)
{
	const char * next = tsScanForBeginningOfNextLine(curr, end);
	while (next > curr) {
		if (*curr >= 'a' && *curr <= 'z')
			return false;
		++curr;
	}
	return true;
}

bool charIsEmphasis(char c) {
	return c == '*' || c == '_';
}

bool beginsWith(const std::string& input, const char * match)
{
	return !strnicmp(input.c_str(), match, (strlen(match)));
}


string parseShot(const std::string & input, bool & interior, bool & exterior)
{
	interior = false;
	exterior = false;

	if (input.length() < 2)
		return "";

	if (input[0] == '.')
		return StripLeadingWhitespace(input.substr(1));

	if (beginsWith(input, "INT./EXT.") || beginsWith(input, "EXT./INT.")) {
		interior = true;
		exterior = true;
		return StripLeadingWhitespace(input.substr(9));
	}

	if (beginsWith(input, "INT./EXT") || beginsWith(input, "EXT./INT")) {
		interior = true;
		exterior = true;
		return StripLeadingWhitespace(input.substr(8));
	}

	if (beginsWith(input, "INT/EXT") || beginsWith(input, "EXT/INT")) {
		interior = true;
		exterior = true;
		return StripLeadingWhitespace(input.substr(7));
	}

	if (beginsWith(input, "INT ") || beginsWith(input, "EXT ") || beginsWith(input, "INT.") || beginsWith(input, "EXT.") || beginsWith(input, "I/E "))
	{
		interior = beginsWith(input, "I");
		exterior = beginsWith(input, "E") || beginsWith(input, "I/E ");
		return StripLeadingWhitespace(input.substr(4));
	}
	return "";
}

bool isShot(const std::string& input)
{
	if (input.length() < 2)
		return false;

	if (input[0] == '.' && input[1] != '.')
		return true;

	return beginsWith(input, "INT ") || beginsWith(input, "EXT ")
		|| beginsWith(input, "INT.") || beginsWith(input, "EXT.")
		|| beginsWith(input, "INT/EXT") || beginsWith(input, "EXT/INT")
		|| beginsWith(input, "I/E ");
}

bool isTransition(const std::string & input)
{
	if (!input.length())
		return false;

	if (!lineIsUpperCase(input.c_str(), input.c_str() + input.length()))
		return false;

	string s = StripLeadingWhitespace(input);
	size_t len = s.length() - 1;
	if (len < 4)
		return false;

	const char * transitions[] = {
		"CUT TO BLACK",
		"INTERCUT WITH:",
		"FADE IN:",
		"TITLE OVER:",
		"SPLIT SCREEN:",
		"OPENING CREDITS",
		"END CREDITS"
	};

	for (auto str : transitions)
		if (s.find(str) != string::npos)
			return true;

	string end = s.substr(len - 3, len - 2);
	return beginsWith(end, " TO") || beginsWith(end, " IN");
}

string parseTransition(const std::string & input)
{
	return StripLeadingWhitespace(StripLeadingWhitespace(input));
}

bool isDialog(const std::string & input)
{
	if (input.length() < 2)
		return false;

	if (input[0] == '@')
		return true;

	if (isTransition(input))
		return false;

	return lineIsUpperCase(input.c_str(), input.c_str() + input.length());
}

string parseDialog(const string& s, string& character, const vector<string>& lines, size_t & i)
{
	if (s[0] == '@')
		character = s.substr(1);
	else
		character = s;

	string result;

	size_t line = i + 1;
	while (line < lines.size() && lines[line].length() > 0) {
		result += lines[line] + "\n";
		++line;
	}
	i = line - 1;
	return result;
}

Script parseFountain(const std::string& text)
{
	const bool escapes = false;
	const bool emptyLines = true;
	std::vector<std::string> lines = TextScanner::Split(text, '\n', escapes, emptyLines);

	Script script;

	size_t los = lines.size();
	for (size_t i = 0; i < los; ++i)
	{
		string s = StripLeadingWhitespace(lines[i]);
		if (!s.length())
			continue;

		if (beginsWith(s, "Title:")) {
			// Title: Big Fish
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Title", content));
		}
		else if (beginsWith(s, "Credit:")) {
			// Credit: written by
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Credit", content));
		}
		else if (beginsWith(s, "Author:")) {
			// Credit: written by
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Author", content));
		}
		else if (beginsWith(s, "Source:")) {
			// Author: John August
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Source", content));
		}
		else if (beginsWith(s, "Draft Date:")) {
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Draft Date", content));
		}
		else if (beginsWith(s, "Notes:")) {
			// Source: based on the novel by Daniel Wallace
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Notes", content));
		}
		else if (beginsWith(s, "Contact:")) {
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Contact", content));
		}
		else if (beginsWith(s, "Copyright:")) {
			string content = parseValue(s, lines, i);
			script.title.nodes.emplace_back(ScriptNode(NodeKind::KeyValue, "Copyright", content));
		}
		else if (beginsWith(s, "===")) {
			if (script.sequences.size())
				(*script.sequences.rbegin()).nodes.emplace_back(ScriptNode(NodeKind::Divider, s));
			else
				script.title.nodes.emplace_back(ScriptNode(NodeKind::Divider, s));
		}
		else if (isShot(s)) {
			bool interior, exterior;
			string content = parseShot(s, interior, exterior);
			script.sequences.emplace_back(Sequence(content, interior, exterior));
			script.sets.insert(s);
		}
		else if (isTransition(s)) {
			string content = parseTransition(s);
			if (script.sequences.size())
				(*script.sequences.rbegin()).nodes.emplace_back(ScriptNode(NodeKind::Transition, content));
			else
				script.title.nodes.emplace_back(ScriptNode(NodeKind::Transition, content));
		}
		else if (isDialog(s)) {
			string character;
			string content = parseDialog(s, character, lines, i);
			if (script.sequences.size())
				(*script.sequences.rbegin()).nodes.emplace_back(ScriptNode(NodeKind::Dialog, character, content));
			else
				script.title.nodes.emplace_back(ScriptNode(NodeKind::Dialog, character, content));

			size_t paren = character.find('(');
			if (paren != string::npos)
				character = character.substr(0, paren - 1);

			script.characters.insert(character);
		}
		else {
			// must be action
			string content = s;

			++i;
			while (i < lines.size() && lines[i].length() > 0) {
				content += lines[i] + "\n";
				++i;
			}

			if (script.sequences.size())
				(*script.sequences.rbegin()).nodes.emplace_back(ScriptNode(NodeKind::Action, content));
			else
				script.title.nodes.emplace_back(ScriptNode(NodeKind::Action, content));
		}
	}
	
	return script;
}











int main(int argc, char** argv)
{
	std::cout << "LabScreenplay 20171014.0016" << "\n";

	char * text = nullptr;
    char * end = nullptr;

    OptionParser op("landruc");
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
	Script script = parseFountain(scriptText);

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

    return 0;
}
