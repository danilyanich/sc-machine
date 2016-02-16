/*
* This source file is part of an OSTIS project. For the latest info, see http://ostis.net
* Distributed under the MIT License
* (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
*/

#include "iotSpeech.hpp"
#include "iotKeynodes.hpp"
#include "iotUtils.hpp"

#include "wrap/sc_memory.hpp"
#include "wrap/sc_stream.hpp"

namespace iot
{

	IMPLEMENT_AGENT(GenerateSpeechText, COMMAND_AGENT)
	{
		sc::Iterator5Ptr itCmd = mMemoryCtx.iterator5(
			requestAddr,
			SC_TYPE(sc_type_arc_pos_const_perm),
			SC_TYPE(sc_type_node | sc_type_const),
			SC_TYPE(sc_type_arc_pos_const_perm),
			Keynodes::rrel_1
			);

		if (!itCmd->next())
			return SC_RESULT_ERROR_INVALID_PARAMS;

		// got command addr
		sc::Addr commandAddr;
		sc::Addr const commandInstAddr = itCmd->value(2);
		sc::Iterator3Ptr itCommandClass = mMemoryCtx.iterator3(
			SC_TYPE(sc_type_node | sc_type_const | sc_type_node_class),
			SC_TYPE(sc_type_arc_pos_const_perm),
			commandInstAddr);
		while (itCommandClass->next())
		{
			if (mMemoryCtx.helperCheckArc(Keynodes::command, itCommandClass->value(0), SC_TYPE(sc_type_arc_pos_const_perm)))
			{
				commandAddr = itCommandClass->value(0);
				break;
			}
		}

		if (!commandAddr.isValid())
			return SC_RESULT_ERROR_INVALID_STATE;

		sc::Iterator5Ptr itLang = mMemoryCtx.iterator5(
			requestAddr,
			SC_TYPE(sc_type_arc_pos_const_perm),
			SC_TYPE(sc_type_node | sc_type_const),
			SC_TYPE(sc_type_arc_pos_const_perm),
			Keynodes::rrel_2
			);
		if (!itLang->next())
			return SC_RESULT_ERROR_INVALID_PARAMS;

		sc::Addr const langAddr = itLang->value(2);

		sc::Iterator5Ptr itAttr = mMemoryCtx.iterator5(
			requestAddr,
			SC_TYPE(sc_type_arc_pos_const_perm),
			SC_TYPE(sc_type_node | sc_type_const),
			SC_TYPE(sc_type_arc_pos_const_perm),
			Keynodes::rrel_3
			);
		if (!itAttr->next())
			return SC_RESULT_ERROR_INVALID_PARAMS;
		sc::Addr const attrAddr = itAttr->value(2);

		/// TODO: make commond method to get arguments with custom role

		// check if there are speech templates for a specified command
		sc::Iterator5Ptr itTemplatesSet = mMemoryCtx.iterator5(
			SC_TYPE(sc_type_node | sc_type_const | sc_type_node_tuple),
			SC_TYPE(sc_type_arc_common | sc_type_const),
			commandAddr,
			SC_TYPE(sc_type_arc_pos_const_perm),
			Keynodes::nrel_speech_templates
			);

		
		if (!itTemplatesSet->next())
			return SC_RESULT_ERROR_INVALID_PARAMS;
		// got templates set
		sc::Addr const templatesAddr = itTemplatesSet->value(0);
		
		// try to find template for a specified language
		sc::Iterator5Ptr itTempl = mMemoryCtx.iterator5(
			templatesAddr,
			SC_TYPE(sc_type_arc_pos_const_perm),
			SC_TYPE(sc_type_link),
			SC_TYPE(sc_type_arc_pos_const_perm),
			attrAddr
			);

		/// TODO: possible select random template from a set (more then one template for language and result attr)
		while (itTempl->next())
		{
			sc::Addr const linkAddr = itTempl->value(2);
			if (mMemoryCtx.helperCheckArc(langAddr, linkAddr, sc_type_arc_pos_const_perm))
			{
				sc::Stream stream;
				if (mMemoryCtx.getLinkContent(linkAddr, stream))
				{
					std::string strTemplate;
					if (sc::StreamConverter::streamToString(stream, strTemplate))
					{
						std::string resultText;
						TextTemplateProcessor processor(mMemoryCtx, strTemplate, langAddr);
						if (processor.generateOutputText(resultText))
						{
							sc::Addr const resultLink = mMemoryCtx.createLink();
							assert(resultLink.isValid());

							sc::Stream resultStream(resultText.c_str(), (sc_uint32)resultText.size(), SC_STREAM_FLAG_READ | SC_STREAM_FLAG_SEEK);
							
							bool const res = mMemoryCtx.setLinkContent(resultLink, resultStream);
							assert(res);

							sc::Addr const edge = mMemoryCtx.createArc(sc_type_arc_pos_const_perm, resultAddr, resultLink);
							assert(edge.isValid());
							
						}						
					}
				}
				else
				{
					/// TODO: generate default text

					// for a fast test, just use template as an answer
					sc::Addr const edge = mMemoryCtx.createArc(sc_type_arc_pos_const_perm, resultAddr, linkAddr);
					assert(edge.isValid());
				}

				return SC_RESULT_OK;
			}
		}


		return SC_RESULT_ERROR;
	}

	sc_result handler_generate_text_command(sc_event const * event, sc_addr arg)
	{
		RUN_AGENT(GenerateSpeechText, Keynodes::command_generate_text_from_template, sc_access_lvl_make_min, sc::Addr(arg));
	}





// ----------------- Template processor ---------------

	TextTemplateProcessor::TextTemplateProcessor(sc::MemoryContext & memoryCtx, std::string const & str, sc::Addr const & langAddr)
		: mMemoryCtx(memoryCtx)
		, mInputTextTemplate(str)
		, mLanguageAddr(langAddr)
	{
	}

	TextTemplateProcessor::~TextTemplateProcessor()
	{
	}

	bool TextTemplateProcessor::generateOutputText(std::string & outText)
	{
		/// TODO: make complex template language parser
		
		// for that moment we will parse just on command ($main_idtf)
		// syntax: $main_idtf(<sysIdtf>);
		size_t currentChar = 0, prevChar = 0;
		while (1)
		{
			currentChar = mInputTextTemplate.find_first_of("$", currentChar);
			if (currentChar == std::string::npos)
				break;

			outText += mInputTextTemplate.substr(prevChar, currentChar - prevChar);

			// determine command name
			size_t bracketStart = mInputTextTemplate.find_first_of("(", currentChar);
			if (bracketStart != std::string::npos)
			{
				std::string commandName = mInputTextTemplate.substr(currentChar + 1, bracketStart - currentChar - 1);
				
				// determine arguments end
				size_t bracketEnd = mInputTextTemplate.find_first_of(")", bracketStart);
				if (bracketEnd != std::string::npos)
				{
					prevChar = bracketEnd;
					std::string arguments = mInputTextTemplate.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

					/// TODO: parse arguments list 

					std::string replacement;
					if (commandName == "main_idtf")
					{
						replacement = processMainIdtfCmd(arguments);
					}

					// replace command by result value
					outText += replacement;
				}
			}

			++currentChar;
		};

		if (currentChar != std::string::npos)
			outText += mInputTextTemplate.substr(prevChar);

		return true;
	}


	std::string TextTemplateProcessor::processMainIdtfCmd(std::string & arguments)
	{
		std::string result;
		sc::Addr elAddr;
		if (mMemoryCtx.helperFindBySystemIdtf(arguments, elAddr))
		{
			sc::Addr linkIdtf = Utils::findMainIdtf(mMemoryCtx, elAddr, mLanguageAddr);
			if (linkIdtf.isValid())
			{
				sc::Stream stream;
				if (mMemoryCtx.getLinkContent(linkIdtf, stream))
				{
					sc::StreamConverter::streamToString(stream, result);
				}
			}
		}
		return result;
	}

}