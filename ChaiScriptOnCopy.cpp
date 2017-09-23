#include "stdafx.h"
#include "ChaiScriptOnCopy.h"
#include "DittoChaiScript.h"
#include "Shared\TextConvert.h"

#include "chaiscript/chaiscript.hpp"

using namespace chaiscript;


ChaiScriptOnCopy::ChaiScriptOnCopy()
{
}


ChaiScriptOnCopy::~ChaiScriptOnCopy()
{
}


bool ChaiScriptOnCopy::ProcessScript(IClip *pClip, std::string script, std::string activeApp)
{
	m_lastError = _T("");
	bool continueCopy = true;
	
	try
	{
		ChaiScript chai;

		chai.add(chaiscript::fun(&CDittoChaiScript::GetClipMD5), "GetClipMD5");

		CDittoChaiScript clipData(pClip, activeApp);
		chai.add(chaiscript::var(&clipData), "clip");

		//loop over all scripts
		/*std::string script = R""(	
									var md5 = clip.GetClipMD5(13)
									return md5 == "4FF8DB22A28559FB23AB9EB90523AE3E"
								)"";*/


		Boxed_Value bv = chai.eval(script);
		if (chaiscript::boxed_cast<bool> (bv) == true)
		{
			m_lastError = _T("Script return true, to cancel copy");
			continueCopy = false;
		}
	}
	catch (const chaiscript::exception::eval_error &ee)
	{
		std::string eString;
		eString += ee.pretty_print();

		m_lastError = CTextConvert::MultiByteToUnicodeString(eString.c_str());
	}
	catch (std::exception &e)
	{
		std::string eString;
		eString += e.what();

		m_lastError = CTextConvert::MultiByteToUnicodeString(eString.c_str());
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
