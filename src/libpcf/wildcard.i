/**
 * @file wildcard.i
 * @author Daniel Starke
 * @copyright Copyright 2014-2017 Daniel Starke
 * @see wildcards.h
 * @see wildcardus.h
 * @date 2014-11-05
 * @version 2016-05-01
 * @internal This file is never used or compiled directly but only included.
 * @remarks Define CHAR_T to the character type before including this file.
 * @remarks See WILDCARD_FUNC() for further notes.
 */


/**
 * Returns 1 if text matches pattern, else 0.
 * Valid characters for the matching string are:
 * @li * matches any character 0 to unlimited times
 * @li ? matches any character exactly once
 * @li # matches any digit exactly once
 * Any other character will be expected to be the same as in text to match.
 *
 * @param[in] text - text string
 * @param[in] pattern - wildcard matching string
 * @return 1 on match, else 0
 * @remarks Define WILDCARD_NUMBER_FUNC for the digit comparison function.
 * @remarks Define WILDCARD_FUNC for name of the wildcard matching function.
 */
int WILDCARD_FUNC(const CHAR_T * text, const CHAR_T * pattern) {
	const CHAR_T * curText; /* current text */
	const CHAR_T * curPat; /* current pattern */
	const CHAR_T * laText; /* look ahead text */
	const CHAR_T * laPat; /* look ahead pattern */
	const CHAR_T * sPat; /* saved pattern */
	if (text == NULL || pattern == NULL) return 0;
	curText = text;
	curPat = pattern;
	do {
		switch (*curPat) {
		case '*':
			laText = curText;
			laPat = curPat + 1;
			if (*laPat == 0) return 1;
			if (*laText == 0) {
				curPat = laPat;
				continue;
			}
			sPat = laPat;
			while (*sPat == '*') {
				curPat = sPat;
				sPat++;
			}
			if (*sPat == *curText || *sPat == '?' || (*sPat == '#' && WILDCARD_NUMBER_FUNC(*curText))) {
				laPat = sPat;
				while (*laPat != '*' && *laPat != 0) {
					if (*laText == *laPat || *laPat == '?' || (*laPat == '#' && WILDCARD_NUMBER_FUNC(*laText))) {
						laText++;
						laPat++;
					} else {
						curText++;
						break;
					}
					if (*laText == 0) {
						while (*laPat == '*') {
							laPat++;
						}
						if (*laPat == 0) {
							return 1;
						} else {
							curText++;
							break;
						}
					}
				}
				if (*laPat == '*') {
					curText = laText;
					sPat = laPat;
				} else {
					if (*laText == *laPat && *laPat == 0) {
						/* this point will never be reached (see previous return) */
						return 1;
					} else if (*laText == 0) {
						continue;
					} else if (*laPat == 0) {
						curText++;
						sPat = curPat;
					} else {
						sPat = curPat;
					}
				}
				curPat = sPat;
			} else {
				curText++;
			}
			break;
		case '?':
			if (*curText != 0) {
				curText++;
				curPat++;
				if (*curText != 0 && *curPat == 0) {
					return 0;
				}
			} else {
				return 0;
			}
			break;
		case '#':
			if ( WILDCARD_NUMBER_FUNC(*curText) ) {
				curText++;
				curPat++;
				if (*curText != 0 && *curPat == 0) {
					return 0;
				}
			} else {
				return 0;
			}
			break;
		default:
			if (*curText == *curPat) {
				curText++;
				curPat++;
			} else {
				return 0;
			}
			break;
		}
	} while (*curPat != 0);
	if (*curText != 0) return 0;
	return 1;
}
