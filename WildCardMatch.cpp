#include "stdafx.h"
#include "WildCardMatch.h"


CWildCardMatch::CWildCardMatch(void)
{
}


CWildCardMatch::~CWildCardMatch(void)
{
}

BOOL CWildCardMatch::WildMatch(CString sWild, CString sString, CString sLimitChar)
{
	BOOL bAny = FALSE;
	BOOL bNextIsOptional = FALSE;
	BOOL bAutorizedChar = TRUE;

	int i=0;
	int j=0;

	// Check all the string char by char
	while (i < sString.GetLength()) 
	{
		// Check index for array overflow
		if (j < sWild.GetLength())
		{
			// Manage '*' in the wildcard
			if (sWild[j]=='*') 
			{
				// Go to next character in the wildcard
				j++;

				// Enf of the string and wildcard end 
				// with *, only test string validity
				if (j >= sWild.GetLength()) 
				{
					// Check end of the string
					while (!sLimitChar.IsEmpty() && i < sString.GetLength()) 
					{
						// If this char is not ok, return false
						if (sLimitChar.Find(sString[i]) < 0)
							return FALSE;

						i++;
					}

					return TRUE;
				}

				bAny = TRUE;
				bNextIsOptional = FALSE;
			} 
			else 
			{
				// Optional char in the wildcard
				if (sWild[j] == '^')
				{
					// Go to next char in the wildcard and indicate 
					// that the next is optional
					j++;
					bNextIsOptional = TRUE;
				}
				else
				{
					bAutorizedChar = ((sLimitChar.IsEmpty()) || (sLimitChar.Find(sString[i])>=0));

					// IF :
					// Current char match the wildcard
					// '?' is used and current char is in authorized char list
					// Char is optional and it's not in the string
					// and it's necessary to test if '*' make any 
					// char browsing

					if (sWild[j] == sString[i] || 
						(sWild[j] == '?' && bAutorizedChar) || 
						(bNextIsOptional && !(bAny && bAutorizedChar))) 
					{
						// If current char match wildcard, 
						// we stop for any char browsing
						if (sWild[j] == sString[i])
							bAny = FALSE;

						// If it's not an optional char who is not present,
						// go to next
						if (sWild[j] == sString[i] || sWild[j] == '?')
							i++;

						j++;

						bNextIsOptional = FALSE;
					} 
					else
					{
						// If we are in any char browsing ('*') 
						// and current char is authorized
						if (bAny && bAutorizedChar)
						{
							// Go to next
							i++;
						}
						else
						{
							return FALSE;
						}
					}
				}
			}
		}
		else
		{
			// End of the wildcard but not the 
			// end of the string => 
			// not matching
			return FALSE;
		}
	}

	if (j < sWild.GetLength() && sWild[j] == '^')
	{
		bNextIsOptional = TRUE;
		j++;
	}


	// If the string is shorter than wildcard 
	// we test end of the 
	// wildcard to check matching
	while ((j < sWild.GetLength() && sWild[j] == '*') || bNextIsOptional)
	{
		j++;
		bNextIsOptional = FALSE;

		if (j < sWild.GetLength() && sWild[j] == '^')
		{
			bNextIsOptional = TRUE;
			j++;
		}
	}

	return j >= sWild.GetLength();
}