// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#pragma once

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
	ScriptNode & operator=(ScriptNode && rh)
	{
		kind = rh.kind;
		key = rh.key;
		content = rh.content;
	}

	NodeKind kind = NodeKind::Unknown;
	std::string key;
    std::string content;

	std::string as_string() const;
};

struct Sequence
{
	Sequence() = default;
	Sequence(const std::string & name_, bool interior, bool exterior);
	Sequence(Sequence && rh);
	Sequence & operator=(Sequence && rh);

	std::string as_string() const;

	std::string name;
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

	static Script parseFountain(const std::string& fountainFile);
	static Script parseFountain(const filesystem::path& fountainFile);
};

} // lab
