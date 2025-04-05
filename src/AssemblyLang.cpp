#include "TextEditor.h"
TextEditor::LanguageDefinition AssemblyLangDef()
{
	static bool inited = false;
	static TextEditor::LanguageDefinition langDef;
	if (!inited)
	{
		static const char* const keywords[] = {
			".enabl", ".dsabl", ".page", ".title", ".sbttl", ".module", ".include", ".incbin", ".area", ".psharea", ".poparea", ".bank",
            ".org", ".radix", ".globl", ".local", ".if", ".iff", ".ift", ".iftf", ".ifdef", ".ifndef", ".ifgt", ".iflt", ".ifge", ".ifle",
            ".ifeq", ".ifne", ".ifb", ".ifnb", ".ifidn", ".ifdif", ".iif", ".iiff", ".iift", ".iiftf", ".iifdef", ".iifndef", ".iifgt", ".iiflt",
            ".iifge", ".iifle", ".iifeq", ".iifne", ".iifb", ".iifnb", ".iifidn", ".iifdif", ".else", ".endif", ".list", ".nlist", ".equ",
            ".gblequ", ".lclequ", ".byte", ".db", ".fcb", ".word", ".dw", ".fdb", ".blkb", ".ds", ".rmb", ".rs", ".blkw", ".ascii", ".ascis",
            ".asciz", ".str", ".strs", ".strz", ".fcc", ".define", ".undefine", ".even", ".odd", ".bndry", ".msg", ".assume", ".error", ".trace",
            ".ntrace", ".end", ".macro", ".endm", ".mexit", ".narg", ".nchr", ".ntyp", ".irp", ".irpc", ".rept", ".nval", ".mdelete", ".b865",
            "adc", "add", "and", "cmp", "nand", "nor", "or", "sbc", "sub", "xnor", "xor", "bcc", "bcs", "beq", "bmi", "bne", "bpl", "clc",
            "stc", "hlt", "nop", "dex", "inx", "rts", "jmp", "jsr", "mov", "dec", "inc", "phr", "plr", "rol", "ror", "rsh", "lsh",
		};

		for (auto& k : keywords)
			langDef.mKeywords.insert(k);

		static const char* const identifiers[] = {
			"a", "x", "y", "sp", "r0", "r1", "r2", "r3",
		};
		for (auto& k : identifiers)
		{
			TextEditor::Identifier id;
			id.mDeclaration = "Register";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		static const char* const identifiers2[] = {
			"ABS", "REL", "OVR", "CON", "NOPAG", "PAG",
		};
		for (auto& k : identifiers2)
		{
			TextEditor::Identifier id;
			id.mDeclaration = "Area type";
			langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
		}

		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("\\\"[^\\\"]*\\\"", TextEditor::PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("\\\'[^\\\']*\\\'", TextEditor::PaletteIndex::String));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("#?(0[xX][0-9a-fA-F]*)|([0-9]+)", TextEditor::PaletteIndex::Number));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[a-zA-Z][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("_[a-zA-Z][a-zA-Z0-9_]*", TextEditor::PaletteIndex::KnownIdentifier));
		langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", TextEditor::PaletteIndex::Punctuation));

		langDef.mCommentStart = "/*";
		langDef.mCommentEnd = "*/";
		langDef.mSingleLineComment = ";";
        langDef.mPreprocChar = '.';

		langDef.mCaseSensitive = true;
		langDef.mAutoIndentation = true;

		langDef.mName = "b865_Assembly";

		inited = true;
	}
	return langDef;
}