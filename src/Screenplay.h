// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <filesystem>

namespace lab
{

	namespace filesystem = std::experimental::filesystem;

enum class NodeKind
{
	KeyValue,
	Divider,
	Character, Action, Location, Dialog, Direction, Transition,
	Unknown
};

struct ScriptNode
{
	ScriptNode() = default;
	~ScriptNode() = default;

    ScriptNode(NodeKind kind, const std::string& content) : kind(kind), content(content) {}
	ScriptNode(NodeKind kind, const std::string& key, const std::string& content) : kind(kind), key(key), content(content) {}
	ScriptNode(const ScriptNode & rh) : kind(rh.kind), key(rh.key), content(rh.content) {}
    ScriptNode(ScriptNode && rh) : kind(rh.kind), key(rh.key), content(rh.content) {}
	ScriptNode& operator=(ScriptNode && rh)
	{
		kind = rh.kind;
		key = rh.key;
		content = rh.content;
		return *this;
	}
	ScriptNode& operator=(const ScriptNode & rh)
	{
		kind = rh.kind;
		key = rh.key;
		content = rh.content;
		return *this;
	}

	NodeKind kind = NodeKind::Unknown;
	std::string key;
    std::string content;

	std::string as_string() const;
};

struct Sequence
{
	Sequence() = default;
	Sequence(const std::string & name_, const std::string & location_, bool interior, bool exterior);
	Sequence(Sequence && rh);
	Sequence & operator=(Sequence && rh);

	std::string as_string() const;

	std::string name;
	std::string location;
	bool interior = false;
	bool exterior = false;
	std::vector<ScriptNode> nodes;
};

struct Script
{
	Script() = default;
	Script(Script && rh) noexcept;

	Sequence title;
	std::set<std::string> characters;
	std::set<std::string> sets;
	std::vector<Sequence> sequences;
	std::map<std::string, int> sequence_index;

	static Script parseFountain(const std::string& fountainFile);
	static Script parseFountain(const filesystem::path& fountainFile);
};

struct ScriptMeta
{
	ScriptMeta(const Script&);
	std::map<std::string, std::set<std::string>> sequence_characters;
	std::map<std::string, std::vector<std::string>> character_dialog;
};

} // lab
