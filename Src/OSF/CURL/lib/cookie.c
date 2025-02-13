/***************************************************************************
*                                  _   _ ____  _
*  Project                     ___| | | |  _ \| |
*                             / __| | | | |_) | |
*                            | (__| |_| |  _ <| |___
*                             \___|\___/|_| \_\_____|
*
* Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
*
* This software is licensed as described in the file COPYING, which
* you should have received as part of this distribution. The terms
* are also available at https://curl.haxx.se/docs/copyright.html.
*
* You may opt to use, copy, modify, merge, publish, distribute and/or sell
* copies of the Software, and permit persons to whom the Software is
* furnished to do so, under the terms of the COPYING file.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
***************************************************************************/

/***


   RECEIVING COOKIE INFORMATION
   ============================

   struct CookieInfo *Curl_cookie_init(struct Curl_easy *data,
                    const char *file, struct CookieInfo *inc, bool newsession);

        Inits a cookie struct to store data in a local file. This is always
        called before any cookies are set.

   struct Cookie *Curl_cookie_add(struct Curl_easy *data,
                 struct CookieInfo *c, bool httpheader, char *lineptr,
                 const char *domain, const char *path);

        The 'lineptr' parameter is a full "Set-cookie:" line as
        received from a server.

        The function need to replace previously stored lines that this new
        line superceeds.

        It may remove lines that are expired.

        It should return an indication of success/error.


   SENDING COOKIE INFORMATION
   ==========================

   struct Cookies *Curl_cookie_getlist(struct CookieInfo *cookie,
                                    char *host, char *path, bool secure);

        For a given host and path, return a linked list of cookies that
        the client should send to the server if used now. The secure
        boolean informs the cookie if a secure connection is achieved or
        not.

        It shall only return cookies that haven't expired.


   Example set of cookies:

    Set-cookie: PRODUCTINFO=webxpress; domain=.fidelity.com; path=/; secure
    Set-cookie: PERSONALIZE=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/ftgw; secure
    Set-cookie: FidHist=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: FidOrder=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: DisPend=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: FidDis=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie:
    Session_Key@6791a9e0-901a-11d0-a1c8-9b012c88aa77=none;expires=Monday,
    13-Jun-1988 03:04:55 GMT; domain=.fidelity.com; path=/; secure
 ****/

#include "curl_setup.h"
#pragma hdrstop
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)

#ifdef USE_LIBPSL
	#include <libpsl.h>
#endif
#include "curl_printf.h"
#include "memdebug.h"

static void FASTCALL freecookie(struct Cookie * co)
{
	SAlloc::F(co->expirestr);
	SAlloc::F(co->domain);
	SAlloc::F(co->path);
	SAlloc::F(co->spath);
	SAlloc::F(co->name);
	SAlloc::F(co->value);
	SAlloc::F(co->maxage);
	SAlloc::F(co->version);
	SAlloc::F(co);
}

static bool tailmatch(const char * cooke_domain, const char * hostname)
{
	size_t cookie_domain_len = sstrlen(cooke_domain);
	size_t hostname_len = sstrlen(hostname);
	if(hostname_len < cookie_domain_len)
		return FALSE;
	if(!strcasecompare(cooke_domain, hostname+hostname_len-cookie_domain_len))
		return FALSE;
	/* A lead char of cookie_domain is not '.'.
	   RFC6265 4.1.2.3. The Domain Attribute says:
	     For example, if the value of the Domain attribute is
	     "example.com", the user agent will include the cookie in the Cookie
	     header when making HTTP requests to example.com, www.example.com, and
	     www.corp.example.com.
	 */
	if(hostname_len == cookie_domain_len)
		return TRUE;
	if('.' == *(hostname + hostname_len - cookie_domain_len - 1))
		return TRUE;
	return FALSE;
}

/*
 * matching cookie path and url path
 * RFC6265 5.1.4 Paths and Path-Match
 */
static bool pathmatch(const char * cookie_path, const char * request_uri)
{
	size_t uri_path_len;
	char * uri_path = NULL;
	char * pos;
	bool ret = FALSE;
	/* cookie_path must not have last '/' separator. ex: /sample */
	size_t cookie_path_len = sstrlen(cookie_path);
	if(1 == cookie_path_len) {
		/* cookie_path must be '/' */
		return TRUE;
	}
	uri_path = _strdup(request_uri);
	if(!uri_path)
		return FALSE;
	pos = sstrchr(uri_path, '?');
	if(pos)
		*pos = 0x0;
	/* #-fragments are already cut off! */
	if(0 == sstrlen(uri_path) || uri_path[0] != '/') {
		SAlloc::F(uri_path);
		uri_path = _strdup("/");
		if(!uri_path)
			return FALSE;
	}
	/* here, RFC6265 5.1.4 says
	   4. Output the characters of the uri-path from the first character up
	      to, but not including, the right-most %x2F ("/").
	   but URL path /hoge?fuga=xxx means /hoge/index.cgi?fuga=xxx in some site
	   without redirect.
	   Ignore this algorithm because /hoge is uri path for this case
	   (uri path is not /).
	 */
	uri_path_len = sstrlen(uri_path);
	if(uri_path_len < cookie_path_len) {
		ret = FALSE;
		goto pathmatched;
	}
	/* not using checkprefix() because matching should be case-sensitive */
	if(strncmp(cookie_path, uri_path, cookie_path_len)) {
		ret = FALSE;
		goto pathmatched;
	}
	/* The cookie-path and the uri-path are identical. */
	if(cookie_path_len == uri_path_len) {
		ret = TRUE;
		goto pathmatched;
	}
	/* here, cookie_path_len < url_path_len */
	if(uri_path[cookie_path_len] == '/') {
		ret = TRUE;
		goto pathmatched;
	}
	ret = FALSE;
pathmatched:
	SAlloc::F(uri_path);
	return ret;
}

/*
 * cookie path sanitize
 */
static char * sanitize_cookie_path(const char * cookie_path)
{
	char * new_path = _strdup(cookie_path);
	if(new_path) {
		// some stupid site sends path attribute with '"'. 
		size_t len = sstrlen(new_path);
		if(new_path[0] == '\"') {
			memmove((void *)new_path, (const void *)(new_path + 1), len);
			len--;
		}
		if(len && (new_path[len-1] == '\"')) {
			new_path[len-1] = 0x0;
			len--;
		}
		// RFC6265 5.2.4 The Path Attribute 
		if(new_path[0] != '/') { // Let cookie-path be the default-path. 
			SAlloc::F(new_path);
			new_path = _strdup("/");
		}
		else { // convert /hoge/ to /hoge 
			if(len && new_path[len-1] == '/') {
				new_path[len-1] = 0x0;
			}
		}
	}
	return new_path;
}
/*
 * Load cookies from all given cookie files (CURLOPT_COOKIEFILE).
 *
 * NOTE: OOM or cookie parsing failures are ignored.
 */
void Curl_cookie_loadfiles(struct Curl_easy * data)
{
	struct curl_slist * list = data->change.cookielist;
	if(list) {
		Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
		while(list) {
			struct CookieInfo * newcookies = Curl_cookie_init(data, list->data, data->cookies, data->set.cookiesession);
			if(!newcookies)
				// Failure may be due to OOM or a bad cookie; both are ignored but only the first should be
				infof(data, "ignoring failed cookie_init for %s\n", list->data);
			else
				data->cookies = newcookies;
			list = list->next;
		}
		curl_slist_free_all(data->change.cookielist); /* clean up list */
		data->change.cookielist = NULL; /* don't do this again! */
		Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
	}
}

/*
 * strstore() makes a _strdup() on the 'newstr' and if '*str' is non-NULL
 * that will be freed before the allocated string is stored there.
 *
 * It is meant to easily replace _strdup()
 */
static void strstore(char ** str, const char * newstr)
{
	SAlloc::F(*str);
	*str = _strdup(newstr);
}
/*
 * remove_expired() removes expired cookies.
 */
static void remove_expired(struct CookieInfo * cookies)
{
	curl_off_t now = (curl_off_t)time(NULL);
	struct Cookie * co = cookies->cookies;
	struct Cookie * pv = NULL;
	while(co) {
		struct Cookie * nx = co->next;
		if(co->expires && co->expires < now) {
			if(co == cookies->cookies) {
				cookies->cookies = co->next;
			}
			else {
				pv->next = co->next;
			}
			cookies->numcookies--;
			freecookie(co);
		}
		else {
			pv = co;
		}
		co = nx;
	}
}

/*
 * Return true if the given string is an IP(v4|v6) address.
 */
static bool isip(const char * domain)
{
	struct in_addr addr;

#ifdef ENABLE_IPV6
	struct in6_addr addr6;
#endif
	if(Curl_inet_pton(AF_INET, domain, &addr)
#ifdef ENABLE_IPV6
	    || Curl_inet_pton(AF_INET6, domain, &addr6)
#endif
	    ) {
		/* domain name given as IP address */
		return TRUE;
	}

	return FALSE;
}

/****************************************************************************
 *
 * Curl_cookie_add()
 *
 * Add a single cookie line to the cookie keeping object.
 *
 * Be aware that sometimes we get an IP-only host name, and that might also be
 * a numerical IPv6 address.
 *
 * Returns NULL on out of memory or invalid cookie. This is suboptimal,
 * as they should be treated separately.
 ***************************************************************************/

struct Cookie * Curl_cookie_add(struct Curl_easy * data,
    /* The 'data' pointer here may be NULL at times, and thus
       must only be used very carefully for things that can deal
       with data being NULL. Such as infof() and similar */

    struct CookieInfo * c,
    bool httpheader,             /* TRUE if HTTP header-style line */
    char * lineptr,              /* first character of the line */
    const char * domain,            /* default domain */
    const char * path)              /* full path used when this cookie is set,
                                       used to get default path for the cookie
                                       unless set */
{
	struct Cookie * clist;
	char name[MAX_NAME];
	struct Cookie * co;
	struct Cookie * lastc = NULL;
	time_t now = time(NULL);
	bool replace_old = FALSE;
	bool badcookie = FALSE; /* cookies are good by default. mmmmm yummy */
#ifdef USE_LIBPSL
	const psl_ctx_t * psl;
#endif
#ifdef CURL_DISABLE_VERBOSE_STRINGS
	(void)data;
#endif
	/* First, alloc and init a new struct for it */
	co = (struct Cookie *)SAlloc::C(1, sizeof(struct Cookie));
	if(!co)
		return NULL;  /* bail out if we're this low on memory */
	if(httpheader) {
		/* This line was read off a HTTP-header */
		const char * ptr;
		const char * semiptr;
		char * what = (char *)SAlloc::M(MAX_COOKIE_LINE);
		if(!what) {
			SAlloc::F(co);
			return NULL;
		}
		semiptr = sstrchr(lineptr, ';'); /* first, find a semicolon */
		while(*lineptr && ISBLANK(*lineptr))
			lineptr++;
		ptr = lineptr;
		do {
			/* we have a <what>=<this> pair or a stand-alone word here */
			name[0] = what[0] = 0; /* init the buffers */
			if(1 <= sscanf(ptr, "%" MAX_NAME_TXT "[^;\r\n=] =%" MAX_COOKIE_LINE_TXT "[^;\r\n]", name, what)) {
				/* Use strstore() below to properly deal with received cookie
				   headers that have the same string property set more than once,
				   and then we use the last one. */
				const char * whatptr;
				bool done = FALSE;
				bool sep;
				size_t len = sstrlen(what);
				size_t nlen = sstrlen(name);
				const char * endofn = &ptr[ nlen ];
				/* name ends with a '=' ? */
				sep = (*endofn == '=') ? TRUE : FALSE;
				if(nlen) {
					endofn--; /* move to the last character */
					if(ISBLANK(*endofn)) {
						/* skip trailing spaces in name */
						while(*endofn && ISBLANK(*endofn) && nlen) {
							endofn--;
							nlen--;
						}
						name[nlen] = 0; /* new end of name */
					}
				}

				/* Strip off trailing whitespace from the 'what' */
				while(len && ISBLANK(what[len-1])) {
					what[len-1] = 0;
					len--;
				}

				/* Skip leading whitespace from the 'what' */
				whatptr = what;
				while(*whatptr && ISBLANK(*whatptr))
					whatptr++;

				if(!co->name && sep) {
					/* The very first name/value pair is the actual cookie name */
					co->name = _strdup(name);
					co->value = _strdup(whatptr);
					if(!co->name || !co->value) {
						badcookie = TRUE;
						break;
					}
				}
				else if(!len) {
					/* this was a "<name>=" with no content, and we must allow
					   'secure' and 'httponly' specified this weirdly */
					done = TRUE;
					if(strcasecompare("secure", name))
						co->secure = TRUE;
					else if(strcasecompare("httponly", name))
						co->httponly = TRUE;
					else if(sep)
						/* there was a '=' so we're not done parsing this field */
						done = FALSE;
				}
				if(done)
					;
				else if(strcasecompare("path", name)) {
					strstore(&co->path, whatptr);
					if(!co->path) {
						badcookie = TRUE; /* out of memory bad */
						break;
					}
					co->spath = sanitize_cookie_path(co->path);
					if(!co->spath) {
						badcookie = TRUE; /* out of memory bad */
						break;
					}
				}
				else if(strcasecompare("domain", name)) {
					bool is_ip;

					/* Now, we make sure that our host is within the given domain,
					   or the given domain is not valid and thus cannot be set. */

					if('.' == whatptr[0])
						whatptr++;  /* ignore preceding dot */

#ifndef USE_LIBPSL
					/*
					 * Without PSL we don't know when the incoming cookie is set on a
					 * TLD or otherwise "protected" suffix. To reduce risk, we require a
					 * dot OR the exact host name being "localhost".
					 */
					{
						const char * dotp;
						/* check for more dots */
						dotp = sstrchr(whatptr, '.');
						if(!dotp && !strcasecompare("localhost", whatptr))
							domain = ":";
					}
#endif

					is_ip = isip(domain ? domain : whatptr);

					if(!domain
					    || (is_ip && !strcmp(whatptr, domain))
					    || (!is_ip && tailmatch(whatptr, domain))) {
						strstore(&co->domain, whatptr);
						if(!co->domain) {
							badcookie = TRUE;
							break;
						}
						if(!is_ip)
							co->tailmatch = TRUE; // we always do that if the domain name was given 
					}
					else {
						/* we did not get a tailmatch and then the attempted set domain
						   is not a domain to which the current host belongs. Mark as bad. */
						badcookie = TRUE;
						infof(data, "skipped cookie with bad tailmatch domain: %s\n", whatptr);
					}
				}
				else if(strcasecompare("version", name)) {
					strstore(&co->version, whatptr);
					if(!co->version) {
						badcookie = TRUE;
						break;
					}
				}
				else if(strcasecompare("max-age", name)) {
					/* Defined in RFC2109:

					   Optional.  The Max-Age attribute defines the lifetime of the
					   cookie, in seconds.  The delta-seconds value is a decimal non-
					   negative integer.  After delta-seconds seconds elapse, the
					   client should discard the cookie.  A value of zero means the
					   cookie should be discarded immediately.

					 */
					strstore(&co->maxage, whatptr);
					if(!co->maxage) {
						badcookie = TRUE;
						break;
					}
				}
				else if(strcasecompare("expires", name)) {
					strstore(&co->expirestr, whatptr);
					if(!co->expirestr) {
						badcookie = TRUE;
						break;
					}
				}
				// else this is the second (or more) name we don't know about! 
			}
			else {
				// this is an "illegal" <what>=<this> pair 
			}
			if(!semiptr || !*semiptr) {
				/* we already know there are no more cookies */
				semiptr = NULL;
				continue;
			}
			ptr = semiptr+1;
			while(*ptr && ISBLANK(*ptr))
				ptr++;
			semiptr = sstrchr(ptr, ';'); /* now, find the next semicolon */
			if(!semiptr && *ptr)
				// There are no more semicolons, but there's a final name=value pair coming up 
				semiptr = sstrchr(ptr, '\0');
		} while(semiptr);
		if(co->maxage) {
			co->expires = curlx_strtoofft((*co->maxage=='\"') ? &co->maxage[1] : &co->maxage[0], NULL, 10);
			if(CURL_OFF_T_MAX - now < co->expires)
				co->expires = CURL_OFF_T_MAX; // avoid overflow 
			else
				co->expires += now;
		}
		else if(co->expirestr) {
			/* Note that if the date couldn't get parsed for whatever reason,
			   the cookie will be treated as a session cookie */
			co->expires = curl_getdate(co->expirestr, 0);

			/* Session cookies have expires set to 0 so if we get that back
			   from the date parser let's add a second to make it a
			   non-session cookie */
			if(co->expires == 0)
				co->expires = 1;
			else if(co->expires < 0)
				co->expires = 0;
		}

		if(!badcookie && !co->domain) {
			if(domain) {
				/* no domain was given in the header line, set the default */
				co->domain = _strdup(domain);
				if(!co->domain)
					badcookie = TRUE;
			}
		}

		if(!badcookie && !co->path && path) {
			/* No path was given in the header line, set the default.
			   Note that the passed-in path to this function MAY have a '?' and
			   following part that MUST not be stored as part of the path. */
			const char * queryp = sstrchr(path, '?'); 
			/* queryp is where the interesting part of the path ends, so now we
			   want to the find the last */
			const char * endslash;
			if(!queryp)
				endslash = strrchr(path, '/');
			else
				endslash = (const char *)memrchr(path, '/', (size_t)(queryp - path));
			if(endslash) {
				size_t pathlen = (size_t)(endslash-path+1); /* include ending slash */
				co->path = (char *)SAlloc::M(pathlen+1); /* one extra for the zero byte */
				if(co->path) {
					memcpy(co->path, path, pathlen);
					co->path[pathlen] = 0; /* zero terminate */
					co->spath = sanitize_cookie_path(co->path);
					if(!co->spath)
						badcookie = TRUE;  /* out of memory bad */
				}
				else
					badcookie = TRUE;
			}
		}

		SAlloc::F(what);

		if(badcookie || !co->name) {
			/* we didn't get a cookie name or a bad one,
			   this is an illegal line, bail out */
			freecookie(co);
			return NULL;
		}
	}
	else {
		/* This line is NOT a HTTP header style line, we do offer support for
		   reading the odd netscape cookies-file format here */
		char * ptr;
		char * firstptr;
		char * tok_buf = NULL;
		int fields;

		/* IE introduced HTTP-only cookies to prevent XSS attacks. Cookies
		   marked with httpOnly after the domain name are not accessible
		   from javascripts, but since curl does not operate at javascript
		   level, we include them anyway. In Firefox's cookie files, these
		   lines are preceded with #HttpOnly_ and then everything is
		   as usual, so we skip 10 characters of the line..
		 */
		if(strncmp(lineptr, "#HttpOnly_", 10) == 0) {
			lineptr += 10;
			co->httponly = TRUE;
		}

		if(lineptr[0]=='#') {
			/* don't even try the comments */
			SAlloc::F(co);
			return NULL;
		}
		/* strip off the possible end-of-line characters */
		ptr = sstrchr(lineptr, '\r');
		ASSIGN_PTR(ptr, 0); // clear it 
		ptr = sstrchr(lineptr, '\n');
		ASSIGN_PTR(ptr, 0); // clear it 
		firstptr = strtok_r(lineptr, "\t", &tok_buf); /* tokenize it on the TAB */
		// Now loop through the fields and init the struct we already have allocated 
		for(ptr = firstptr, fields = 0; ptr && !badcookie;
		    ptr = strtok_r(NULL, "\t", &tok_buf), fields++) {
			switch(fields) {
				case 0:
				    if(ptr[0]=='.') /* skip preceding dots */
					    ptr++;
				    co->domain = _strdup(ptr);
				    if(!co->domain)
					    badcookie = TRUE;
				    break;
				case 1:
				    /* This field got its explanation on the 23rd of May 2001 by
				       Andr�s Garc�a:

				       flag: A TRUE/FALSE value indicating if all machines within a given
				       domain can access the variable. This value is set automatically by
				       the browser, depending on the value you set for the domain.

				       As far as I can see, it is set to true when the cookie says
				       .domain.com and to false when the domain is complete www.domain.com
				     */
				    co->tailmatch = strcasecompare(ptr, "TRUE") ? TRUE : FALSE;
				    break;
				case 2:
				    /* It turns out, that sometimes the file format allows the path
				       field to remain not filled in, we try to detect this and work
				       around it! Andr�s Garc�a made us aware of this... */
				    if(strcmp("TRUE", ptr) && strcmp("FALSE", ptr)) {
					    /* only if the path doesn't look like a boolean option! */
					    co->path = _strdup(ptr);
					    if(!co->path)
						    badcookie = TRUE;
					    else {
						    co->spath = sanitize_cookie_path(co->path);
						    if(!co->spath) {
							    badcookie = TRUE; /* out of memory bad */
						    }
					    }
					    break;
				    }
				    /* this doesn't look like a path, make one up! */
				    co->path = _strdup("/");
				    if(!co->path)
					    badcookie = TRUE;
				    co->spath = _strdup("/");
				    if(!co->spath)
					    badcookie = TRUE;
				    fields++; /* add a field and fall down to secure */
				// @fallthrough
				case 3:
				    co->secure = strcasecompare(ptr, "TRUE") ? TRUE : FALSE;
				    break;
				case 4:
				    co->expires = curlx_strtoofft(ptr, NULL, 10);
				    break;
				case 5:
				    co->name = _strdup(ptr);
				    if(!co->name)
					    badcookie = TRUE;
				    break;
				case 6:
				    co->value = _strdup(ptr);
				    if(!co->value)
					    badcookie = TRUE;
				    break;
			}
		}
		if(6 == fields) {
			/* we got a cookie with blank contents, fix it */
			co->value = _strdup("");
			if(!co->value)
				badcookie = TRUE;
			else
				fields++;
		}

		if(!badcookie && (7 != fields))
			/* we did not find the sufficient number of fields */
			badcookie = TRUE;

		if(badcookie) {
			freecookie(co);
			return NULL;
		}
	}

	if(!c->running && /* read from a file */
	    c->newsession && /* clean session cookies */
	    !co->expires) { /* this is a session cookie since it doesn't expire! */
		freecookie(co);
		return NULL;
	}

	co->livecookie = c->running;

	/* now, we have parsed the incoming line, we must now check if this
	   superceeds an already existing cookie, which it may if the previous have
	   the same domain and path as this */

	/* at first, remove expired cookies */
	remove_expired(c);
#ifdef USE_LIBPSL
	/* Check if the domain is a Public Suffix and if yes, ignore the cookie.
	   This needs a libpsl compiled with builtin data. */
	if(domain && co->domain && !isip(co->domain)) {
		psl = psl_builtin();
		if(psl && !psl_is_cookie_domain_acceptable(psl, domain, co->domain)) {
			infof(data, "cookie '%s' dropped, domain '%s' must not set cookies for '%s'\n", co->name, domain, co->domain);
			freecookie(co);
			return NULL;
		}
	}
#endif
	clist = c->cookies;
	replace_old = FALSE;
	while(clist) {
		if(strcasecompare(clist->name, co->name)) {
			/* the names are identical */
			if(clist->domain && co->domain) {
				if(strcasecompare(clist->domain, co->domain) && (clist->tailmatch == co->tailmatch))
					// The domains are identical 
					replace_old = TRUE;
			}
			else if(!clist->domain && !co->domain)
				replace_old = TRUE;

			if(replace_old) {
				// the domains were identical 
				if(clist->spath && co->spath)
					replace_old = strcasecompare(clist->spath, co->spath) ? TRUE : FALSE;
				else if(!clist->spath && !co->spath)
					replace_old = TRUE;
				else
					replace_old = FALSE;
			}
			if(replace_old && !co->livecookie && clist->livecookie) {
				/* Both cookies matched fine, except that the already present
				   cookie is "live", which means it was set from a header, while
				   the new one isn't "live" and thus only read from a file. We let
				   live cookies stay alive */

				/* Free the newcomer and get out of here! */
				freecookie(co);
				return NULL;
			}
			if(replace_old) {
				co->next = clist->next; /* get the next-pointer first */
				/* then free all the old pointers */
				SAlloc::F(clist->name);
				SAlloc::F(clist->value);
				SAlloc::F(clist->domain);
				SAlloc::F(clist->path);
				SAlloc::F(clist->spath);
				SAlloc::F(clist->expirestr);
				SAlloc::F(clist->version);
				SAlloc::F(clist->maxage);
				*clist = *co; /* then store all the new data */
				SAlloc::F(co); /* free the newly alloced memory */
				co = clist; /* point to the previous struct instead */
				/* We have replaced a cookie, now skip the rest of the list but
				   make sure the 'lastc' pointer is properly set */
				do {
					lastc = clist;
					clist = clist->next;
				} while(clist);
				break;
			}
		}
		lastc = clist;
		clist = clist->next;
	}
	if(c->running)
		/* Only show this when NOT reading the cookies from a file */
		infof(data, "%s cookie %s=\"%s\" for domain %s, path %s, expire %" CURL_FORMAT_CURL_OFF_T "\n", replace_old ? "Replaced" : "Added", co->name, co->value, co->domain, co->path, co->expires);

	if(!replace_old) {
		/* then make the last item point on this new one */
		if(lastc)
			lastc->next = co;
		else
			c->cookies = co;
		c->numcookies++; /* one more cookie in the jar */
	}
	return co;
}
/*
 * get_line() makes sure to only return complete whole lines that fit in 'len'
 * bytes and end with a newline.
 */
static char * get_line(char * buf, int len, FILE * input)
{
	bool partial = FALSE;
	while(1) {
		char * b = fgets(buf, len, input);
		if(b) {
			size_t rlen = sstrlen(b);
			if(rlen && (b[rlen-1] == '\n')) {
				if(partial) {
					partial = FALSE;
					continue;
				}
				return b;
			}
			/* read a partial, discard the next piece that ends with newline */
			partial = TRUE;
		}
		else
			break;
	}
	return NULL;
}

/*****************************************************************************
 *
 * Curl_cookie_init()
 *
 * Inits a cookie struct to read data from a local file. This is always
 * called before any cookies are set. File may be NULL.
 *
 * If 'newsession' is TRUE, discard all "session cookies" on read from file.
 *
 * Returns NULL on out of memory. Invalid cookies are ignored.
 ****************************************************************************/
struct CookieInfo * Curl_cookie_init(struct Curl_easy * data, const char * file, struct CookieInfo * inc, bool newsession)
{
	struct CookieInfo * c;
	FILE * fp = NULL;
	bool fromfile = TRUE;
	char * line = NULL;
	if(!inc) {
		// we didn't get a struct, create one 
		c = (struct CookieInfo *)SAlloc::C(1, sizeof(struct CookieInfo));
		if(!c)
			return NULL;  /* failed to get memory */
		c->filename = _strdup(file ? file : "none"); /* copy the name just in case */
		if(!c->filename)
			goto fail;  /* failed to get memory */
	}
	else {
		/* we got an already existing one, use that */
		c = inc;
	}
	c->running = FALSE; /* this is not running, this is init */
	if(file && !strcmp(file, "-")) {
		fp = stdin;
		fromfile = FALSE;
	}
	else if(file && !*file) {
		/* points to a "" string */
		fp = NULL;
	}
	else
		fp = file ? fopen(file, FOPEN_READTEXT) : NULL;
	c->newsession = newsession; /* new session? */
	if(fp) {
		char * lineptr;
		bool headerline;
		line = (char *)SAlloc::M(MAX_COOKIE_LINE);
		if(!line)
			goto fail;
		while(get_line(line, MAX_COOKIE_LINE, fp)) {
			if(checkprefix("Set-Cookie:", line)) {
				/* This is a cookie line, get it! */
				lineptr = &line[11];
				headerline = TRUE;
			}
			else {
				lineptr = line;
				headerline = FALSE;
			}
			while(*lineptr && ISBLANK(*lineptr))
				lineptr++;

			Curl_cookie_add(data, c, headerline, lineptr, 0, 0);
		}
		SAlloc::F(line); /* free the line buffer */

		if(fromfile)
			fclose(fp);
	}

	c->running = TRUE;    /* now, we're running */

	return c;

fail:
	SAlloc::F(line);
	if(!inc)
		/* Only clean up if we allocated it here, as the original could still be in
		 * use by a share handle */
		Curl_cookie_cleanup(c);
	if(fromfile && fp)
		fclose(fp);
	return NULL; /* out of memory */
}

/* sort this so that the longest path gets before the shorter path */
static int cookie_sort(const void * p1, const void * p2)
{
	struct Cookie * c1 = *(struct Cookie**)p1;
	struct Cookie * c2 = *(struct Cookie**)p2;
	// 1 - compare cookie path lengths 
	size_t l1 = sstrlen(c1->path);
	size_t l2 = sstrlen(c2->path);
	if(l1 != l2)
		return (l2 > l1) ? 1 : -1;  /* avoid size_t <=> int conversions */
	// 2 - compare cookie domain lengths 
	l1 = sstrlen(c1->domain);
	l2 = sstrlen(c2->domain);
	if(l1 != l2)
		return (l2 > l1) ? 1 : -1;  /* avoid size_t <=> int conversions */
	// 3 - compare cookie names 
	if(c1->name && c2->name)
		return strcmp(c1->name, c2->name);
	return 0; // sorry, can't be more deterministic 
}

#define CLONE(field)			 \
	do {				       \
		if(src->field) {		     \
			d->field = _strdup(src->field);	   \
			if(!d->field)			   \
				goto fail;			 \
		}				     \
	} while(0)

static struct Cookie * dup_cookie(const struct Cookie * src)
{
	struct Cookie * d = (struct Cookie *)SAlloc::C(sizeof(struct Cookie), 1);
	if(d) {
		CLONE(expirestr);
		CLONE(domain);
		CLONE(path);
		CLONE(spath);
		CLONE(name);
		CLONE(value);
		CLONE(maxage);
		CLONE(version);
		d->expires = src->expires;
		d->tailmatch = src->tailmatch;
		d->secure = src->secure;
		d->livecookie = src->livecookie;
		d->httponly = src->httponly;
	}
	return d;
fail:
	freecookie(d);
	return NULL;
}

/*****************************************************************************
 *
 * Curl_cookie_getlist()
 *
 * For a given host and path, return a linked list of cookies that the
 * client should send to the server if used now. The secure boolean informs
 * the cookie if a secure connection is achieved or not.
 *
 * It shall only return cookies that haven't expired.
 *
 ****************************************************************************/

struct Cookie * Curl_cookie_getlist(struct CookieInfo * c, const char * host, const char * path, bool secure)
{
	struct Cookie * newco;
	struct Cookie * co;
	time_t now = time(NULL);
	struct Cookie * mainco = NULL;
	size_t matches = 0;
	bool is_ip;
	if(!c || !c->cookies)
		return NULL;  /* no cookie struct or no cookies in the struct */
	/* at first, remove expired cookies */
	remove_expired(c);
	/* check if host is an IP(v4|v6) address */
	is_ip = isip(host);
	co = c->cookies;
	while(co) {
		/* only process this cookie if it is not expired or had no expire
		   date AND that if the cookie requires we're secure we must only
		   continue if we are! */
		if((!co->expires || (co->expires > now)) && (co->secure ? secure : TRUE)) {
			// now check if the domain is correct 
			if(!co->domain || (co->tailmatch && !is_ip && tailmatch(co->domain, host)) || ((!co->tailmatch || is_ip) && strcasecompare(host, co->domain)) ) {
				// the right part of the host matches the domain stuff in the cookie data 
				//
				// now check the left part of the path with the cookies path requirement 
				if(!co->spath || pathmatch(co->spath, path) ) {
					// and now, we know this is a match and we should create an entry for the return-linked-list 
					newco = dup_cookie(co);
					if(newco) {
						/* then modify our next */
						newco->next = mainco;
						/* point the main to us */
						mainco = newco;
						matches++;
					}
					else {
fail:
						/* failure, clear up the allocated chain and return NULL */
						Curl_cookie_freelist(mainco);
						return NULL;
					}
				}
			}
		}
		co = co->next;
	}
	if(matches) {
		// Now we need to make sure that if there is a name appearing more than
		// once, the longest specified path version comes first. To make this
		// the swiftest way, we just sort them all based on path length. 
		size_t i;
		// alloc an array and store all cookie pointers 
		struct Cookie ** array = (struct Cookie **)SAlloc::M(sizeof(struct Cookie *) * matches);
		if(!array)
			goto fail;
		co = mainco;
		for(i = 0; co; co = co->next)
			array[i++] = co;
		/* now sort the cookie pointers in path length order */
		qsort(array, matches, sizeof(struct Cookie *), cookie_sort);
		/* remake the linked list order according to the new order */
		mainco = array[0]; /* start here */
		for(i = 0; i<matches-1; i++)
			array[i]->next = array[i+1];
		array[matches-1]->next = NULL; /* terminate the list */
		SAlloc::F(array); /* remove the temporary data again */
	}
	return mainco; /* return the new list */
}

/*****************************************************************************
 *
 * Curl_cookie_clearall()
 *
 * Clear all existing cookies and reset the counter.
 *
 ****************************************************************************/
void Curl_cookie_clearall(struct CookieInfo * cookies)
{
	if(cookies) {
		Curl_cookie_freelist(cookies->cookies);
		cookies->cookies = NULL;
		cookies->numcookies = 0;
	}
}

/*****************************************************************************
 *
 * Curl_cookie_freelist()
 *
 * Free a list of cookies previously returned by Curl_cookie_getlist();
 *
 ****************************************************************************/

void Curl_cookie_freelist(struct Cookie * co)
{
	while(co) {
		struct Cookie * next = co->next;
		freecookie(co);
		co = next;
	}
}

/*****************************************************************************
 *
 * Curl_cookie_clearsess()
 *
 * Free all session cookies in the cookies list.
 *
 ****************************************************************************/
void Curl_cookie_clearsess(struct CookieInfo * cookies)
{
	if(cookies && cookies->cookies) {
		struct Cookie * first, * curr, * next, * prev = NULL;
		first = curr = prev = cookies->cookies;
		for(; curr; curr = next) {
			next = curr->next;
			if(!curr->expires) {
				if(first == curr)
					first = next;
				if(prev == curr)
					prev = next;
				else
					prev->next = next;
				freecookie(curr);
				cookies->numcookies--;
			}
			else
				prev = curr;
		}
		cookies->cookies = first;
	}
}

/*****************************************************************************
 *
 * Curl_cookie_cleanup()
 *
 * Free a "cookie object" previous created with Curl_cookie_init().
 *
 ****************************************************************************/
void Curl_cookie_cleanup(struct CookieInfo * c)
{
	if(c) {
		SAlloc::F(c->filename);
		Curl_cookie_freelist(c->cookies);
		SAlloc::F(c); /* free the base struct as well */
	}
}

/* get_netscape_format()
 *
 * Formats a string for Netscape output file, w/o a newline at the end.
 *
 * Function returns a char * to a formatted line. Has to be SAlloc::F()d
 */
static char * get_netscape_format(const struct Cookie * co)
{
	return aprintf(
	    "%s" /* httponly preamble */
	    "%s%s\t" /* domain */
	    "%s\t" /* tailmatch */
	    "%s\t" /* path */
	    "%s\t" /* secure */
	    "%" CURL_FORMAT_CURL_OFF_T "\t" /* expires */
	    "%s\t" /* name */
	    "%s", /* value */
	    co->httponly ? "#HttpOnly_" : "",
	    /* Make sure all domains are prefixed with a dot if they allow
	       tailmatching. This is Mozilla-style. */
	    (co->tailmatch && co->domain && co->domain[0] != '.') ? "." : "",
	    co->domain ? co->domain : "unknown",
	    co->tailmatch ? "TRUE" : "FALSE",
	    co->path ? co->path : "/",
	    co->secure ? "TRUE" : "FALSE",
	    co->expires,
	    co->name,
	    co->value ? co->value : "");
}
/*
 * cookie_output()
 *
 * Writes all internally known cookies to the specified file. Specify
 * "-" as file name to write to stdout.
 *
 * The function returns non-zero on write failure.
 */
static int cookie_output(struct CookieInfo * c, const char * dumphere)
{
	struct Cookie * co;
	FILE * out;
	bool use_stdout = FALSE;
	char * format_ptr;
	if(!c || !c->numcookies)
		return 0; // If there are no known cookies, we don't write or even create any destination file 
	// at first, remove expired cookies 
	remove_expired(c);
	if(!strcmp("-", dumphere)) {
		// use stdout 
		out = stdout;
		use_stdout = TRUE;
	}
	else {
		out = fopen(dumphere, FOPEN_WRITETEXT);
		if(!out)
			return 1;  /* failure */
	}
	fputs("# Netscape HTTP Cookie File\n# https://curl.haxx.se/docs/http-cookies.html\n# This file was generated by libcurl! Edit at your own risk.\n\n", out);
	for(co = c->cookies; co; co = co->next) {
		if(co->domain) {
			format_ptr = get_netscape_format(co);
			if(format_ptr == NULL) {
				fprintf(out, "#\n# Fatal libcurl error\n");
				if(!use_stdout)
					fclose(out);
				return 1;
			}
			fprintf(out, "%s\n", format_ptr);
			SAlloc::F(format_ptr);
		}
	}
	if(!use_stdout)
		fclose(out);
	return 0;
}

struct curl_slist * Curl_cookie_list(struct Curl_easy * data)
{
	struct curl_slist * list = NULL;
	if(data->cookies && data->cookies->numcookies) {
		for(struct Cookie * c = data->cookies->cookies; c; c = c->next) {
			if(c->domain) {
				char * line = get_netscape_format(c);
				if(!line) {
					curl_slist_free_all(list);
					return NULL;
				}
				else {
					struct curl_slist * beg = Curl_slist_append_nodup(list, line);
					if(!beg) {
						SAlloc::F(line);
						curl_slist_free_all(list);
						return NULL;
					}
					list = beg;
				}
			}
		}
	}
	return list;
}

void Curl_flush_cookies(struct Curl_easy * data, int cleanup)
{
	if(data->set.str[STRING_COOKIEJAR]) {
		if(data->change.cookielist) {
			/* If there is a list of cookie files to read, do it first so that
			   we have all the told files read before we write the new jar.
			   Curl_cookie_loadfiles() LOCKS and UNLOCKS the share itself! */
			Curl_cookie_loadfiles(data);
		}
		Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
		/* if we have a destination file for all the cookies to get dumped to */
		if(cookie_output(data->cookies, data->set.str[STRING_COOKIEJAR]))
			infof(data, "WARNING: failed to save cookies in %s\n", data->set.str[STRING_COOKIEJAR]);
	}
	else {
		if(cleanup && data->change.cookielist) {
			// since nothing is written, we can just free the list of cookie file names 
			curl_slist_free_all(data->change.cookielist); /* clean up list */
			data->change.cookielist = NULL;
		}
		Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
	}
	if(cleanup && (!data->share || (data->cookies != data->share->cookies))) {
		Curl_cookie_cleanup(data->cookies);
	}
	Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
}

#endif /* CURL_DISABLE_HTTP || CURL_DISABLE_COOKIES */
