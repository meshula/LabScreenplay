
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
		case NodeKind::Transition: return ToUpper(content) + "\n";
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

		string s = StripLeadingWhitespace(input);
		size_t len = s.length() - 1;
		if (len < 4)
			return false;

		if (s[0] == '>')
			return true;

		if (!lineIsUpperCase(input.c_str(), input.c_str() + input.length()))
			return false;

		const char * transitions[] = 
		{
			"CUT TO BLACK:",
			"CUT TO:",
			"INTERCUT WITH:",
			"DISSOLVE:",
			"WIPE:",
			"FADE IN:",
			"FADE OUT:",
			"TITLE OVER:",
			"SPLIT SCREEN:",
			"OPENING CREDITS:",
			"END CREDITS:"
		};

		for (auto str : transitions)
			if (s.find(str) != string::npos)
				return true;

		string end = s.substr(len - 3, len - 2);
		return beginsWith(end, " TO") || beginsWith(end, " IN");
	}

	string parseTransition(const std::string & input)
	{
		string r = StripLeadingWhitespace(input);
		if (r[0] == '>')
			r = r.substr(1);

		return StripTrailingWhitespace(StripLeadingWhitespace(r));
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

	struct ScriptEdit
	{
		Script* script = nullptr;
		Sequence* curr_sequence = nullptr;
		ScriptNode curr_node;

		void start_node(NodeKind kind, const std::string& value)
		{
			finalize_current_node();
			curr_node = { kind, value, "" };

			if (kind == NodeKind::Dialog)
			{
				/// @TODO how to interpret a value with parentheses? What does the spec say...?
				script->characters.insert(value);
			}
		}
		void finalize_current_node()
		{
			if (curr_node.kind != NodeKind::Unknown)
			{
				curr_sequence->nodes.push_back(curr_node);
				curr_node.kind = NodeKind::Unknown;
				curr_node.key = "";
				curr_node.content = "";
			}
		}
		void append_text(const std::string& s)
		{
			if (curr_node.kind == NodeKind::Unknown)
				curr_node.kind = NodeKind::Action;
			if (curr_node.content.size())
				curr_node.content += '\n';
			curr_node.content += s;
		}

		void start_sequence(const string& name, bool interior, bool exterior)
		{
			finalize_current_sequence();
			script->sequences.emplace_back(Sequence(name, interior, exterior));
			script->sets.insert(script->sequences.back().as_string());
			curr_sequence = &script->sequences.back();
		}
		void finalize_current_sequence()
		{
			finalize_current_node();
			curr_sequence = nullptr;
		}
	};


	Script Script::parseFountain(const std::string& text)
	{
		Script script;
		ScriptEdit edit = { &script, &script.title };
		std::vector<std::string> lines = TextScanner::SplitLines(text);

		const char* title_page_tags[] = 
		{
			"Title:", "Credit:", "Author:", "Source:", "Draft Date:",
			"Notes:", "Contact:", "Copyright:"
		};

		for (auto& line : lines)
		{
			auto s = StripLeadingWhitespace(line);

			if (beginsWith(s, "==="))
			{
				edit.start_node(NodeKind::Divider, s);
				edit.finalize_current_node();
				continue;
			}

			bool titled = false;
			for (auto t : title_page_tags)
			{
				if (beginsWith(s, t))
				{
					edit.start_node(NodeKind::KeyValue, std::string(t, strlen(t) - 1));
					titled = true;
					break;
				}
			}
			if (titled)
				continue;

			if (isShot(s))
			{
				bool interior, exterior;
				string shot_name = parseShot(s, interior, exterior);
				edit.start_sequence(shot_name, interior, exterior);
				continue;
			}

			if (isTransition(s))
			{
				edit.start_node(NodeKind::Transition, ToUpper(parseTransition(s)));
				edit.finalize_current_node();
				continue;
			}

			if (isDialog(s))
			{
				if (s[0] == '@')
					s = s.substr(1);
				s = StripLeadingWhitespace(s);

				edit.start_node(NodeKind::Dialog, s);
				continue;
			}

			edit.append_text(s);
		}

		edit.finalize_current_sequence();
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
