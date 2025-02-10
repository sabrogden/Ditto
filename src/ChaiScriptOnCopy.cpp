#include "stdafx.h"
#include "ChaiScriptOnCopy.h"
#include "..\Shared\TextConvert.h"

#include "chaiscript/chaiscript.hpp"

using namespace chaiscript;


ChaiScriptOnCopy::ChaiScriptOnCopy()
{
}


ChaiScriptOnCopy::~ChaiScriptOnCopy()
{
}

std::string FormatCurrentTime(const std::string &format) {
	CTime t = CTime::GetCurrentTime();
	CStringA x = t.Format(format.c_str());
	return x;
}

bool ChaiScriptOnCopy::ProcessScript(CDittoChaiScript &clipData, std::string script)
{
	m_lastError = _T("");
	bool continueCopy = true;
	 
	try
	{
		ChaiScript chai;

		chai.add(chaiscript::fun(&CDittoChaiScript::GetClipMD5), "GetClipMD5");
		chai.add(chaiscript::fun(&CDittoChaiScript::GetClipSize), "GetClipSize");
		chai.add(chaiscript::fun(&CDittoChaiScript::GetAsciiString), "GetAsciiString");
		chai.add(chaiscript::fun(&CDittoChaiScript::SetAsciiString), "SetAsciiString");
		chai.add(chaiscript::fun(&CDittoChaiScript::GetActiveApp), "GetActiveApp");

		chai.add(chaiscript::fun(&CDittoChaiScript::GetActiveAppTitle), "GetActiveAppTitle");

		chai.add(chaiscript::fun(&CDittoChaiScript::SetMakeTopSticky), "SetMakeTopSticky");
		chai.add(chaiscript::fun(&CDittoChaiScript::SetMakeLastSticky), "SetMakeLastSticky");
		chai.add(chaiscript::fun(&CDittoChaiScript::SetReplaceTopSticky), "SetReplaceTopSticky");

		chai.add(chaiscript::fun(&CDittoChaiScript::FormatExists), "FormatExists");
		chai.add(chaiscript::fun(&CDittoChaiScript::RemoveFormat), "RemoveFormat");
		chai.add(chaiscript::fun(&CDittoChaiScript::SetParentId), "SetParentId");
		chai.add(chaiscript::fun(&CDittoChaiScript::AsciiTextMatchesRegex), "AsciiTextMatchesRegex");
		chai.add(chaiscript::fun(&CDittoChaiScript::AsciiTextReplaceRegex), "AsciiTextReplaceRegex");

		chai.add(chaiscript::fun(&CDittoChaiScript::DescriptionMatchesRegex), "DescriptionMatchesRegex");
		chai.add(chaiscript::fun(&CDittoChaiScript::DescriptionReplaceRegex), "DescriptionReplaceRegex");

		chai.add(chaiscript::fun(&FormatCurrentTime), "FormatCurrentTime");

		chai.add(chaiscript::var(&clipData), "clip");

		//loop over all scripts
		/*std::string script = R""(	
									var md5 = clip.GetClipMD5(13)
									return md5 == "4FF8DB22A28559FB23AB9EB90523AE3E"
								)"";*/


		Boxed_Value bv = chai.eval(script);
		if (chaiscript::boxed_cast<bool> (bv) == true)
		{
			m_lastError = _T("Script returned true, canceling copy");
			continueCopy = false;
		}
	}
	catch (const chaiscript::exception::eval_error &ee)
	{
		std::string eString;
		eString += ee.pretty_print();

		m_lastError = CTextConvert::AnsiToUnicode(eString.c_str());
	}
	catch (std::exception &e)
	{
		std::string eString;
		eString += e.what();

		m_lastError = CTextConvert::AnsiToUnicode(eString.c_str());
	}
	catch (CException *ex)
	{
		TCHAR szCause[255];
		ex->GetErrorMessage(szCause, 255);
		m_lastError.Format(_T("ProcessScript exception: %s"), szCause);
	}
	catch (...)
	{
		m_lastError.Format(_T("ProcessScript exception"));
	}

	return continueCopy;
}
