
// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#include "Screenplay.h"
#include <LabText/TextScanner.h>
#include <LabText/TextScanner.hpp>


namespace lab
{
	using namespace std;
	using namespace TextScanner;

	std::string ScriptNode::as_string() const
	{
		switch (kind)
		{
		case NodeKind::KeyValue: return key + ": " + content;
		case NodeKind::Divider: return content + "\n";
		case NodeKind::Character:
		case NodeKind::Location: return content + "\n";
		case NodeKind::Action: return content + "\n";
		case NodeKind::Dialog: return key + "\n" + content;
		case NodeKind::Direction: return content + "\n";
		case NodeKind::Transition: return content + "\n";
			break;
		}

		return "";
	}

	Sequence::Sequence(const std::string & name_, bool interior, bool exterior)
		: interior(interior), exterior(exterior)
	{
		name = TextScanner::StripLeadingWhitespace(name_);
	}

	Sequence::Sequence(Sequence && rh)
		: name(rh.name), interior(rh.interior), exterior(rh.exterior)
	{
		nodes.swap(rh.nodes);
	}

	Sequence & Sequence::operator=(Sequence && rh)
	{
		interior = rh.interior;
		exterior = rh.exterior;
		name = rh.name;
		nodes.swap(rh.nodes);
		return *this;
	}

	std::string Sequence::as_string() const
	{
		std::string res;
		if (interior && exterior)
			res = "INT/EXT ";
		else if (interior)
			res = "INT. ";
		else if (exterior)
			res = "EXT. ";
		return res + name;
	}


	Script::Script(Script && rh) noexcept
		: title(std::move(rh.title))
		, characters(std::move(rh.characters))
		, sets(std::move(rh.sets))
		, sequences(std::move(rh.sequences))
	{
	}

#ifdef _MSC_VER
	inline FILE* open_file(const filesystem::path& p)
	{
		return _wfopen(p.c_str(), L"r");
	}
#else
	inline FILE* open_file(const filesystem::path& p)
	{
		return std::fopen(p.c_str(), "r");
	}
#endif


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

		size_t line = i + 1;
		while (isLineContinuation(lines[line])) {
			result += lines[line];
			++line;
		}
		i = line - 1;
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

#ifdef _MSC_VER
#define strnicmp _strnicmp
#endif


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
			"> CUT TO:",
			"> cut to:",
			"CUT TO BLACK",
			"CUT TO:",
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

	Script Script::parseFountain(const std::string& text)
	{
		const bool escapes = false;
		const bool emptyLines = true;
		std::vector<std::string> lines = TextScanner::SplitLines(text);

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

	Script Script::parseFountain(const filesystem::path& fountainFile)
	{
		FILE* f = open_file(fountainFile);
		if (!f) 
			throw std::runtime_error("Couldn't open file");

		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char* text = new char[len + 1];
		fread(text, 1, len, f);
		text[len] = '\0';
		fclose(f);
		char* end = text + len;
		std::string txt(text, end);
		delete[] text;
		return parseFountain(txt);
	}


}
