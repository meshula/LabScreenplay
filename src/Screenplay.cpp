
// License: BSD 3-clause
// Copyright: Nick Porcino, 2017

#include "Screenplay.h"
#include <LabText/TextScanner.hpp>

namespace lab
{
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



}
