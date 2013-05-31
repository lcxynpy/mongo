/*-
 * Copyright (c) 2008-2013 WiredTiger, Inc.
 *	All rights reserved.
 *
 * See the file LICENSE for redistribution information.
 */

#include "wt_internal.h"
/* I'm sure we need to config this */
#include <dirent.h>

/*
 * __wt_dirlist --
 *	Get a list of files from a directory, optionally filtered by
 *	a given prefix.
 */
int
__wt_dirlist(WT_SESSION_IMPL *session, const char *dir, const char *prefix,
    uint32_t flags, char ***dirlist, int *countp)
{
	WT_DECL_RET;
	struct dirent *dp;
	DIR *dirp;
	char **entries;
	size_t dirallocsz;
	int count, dirsz, match;

	*countp = 0;
	entries = NULL;
	dirallocsz = 0;
	if (flags == 0)
		LF_SET(WT_DIRLIST_INCLUDE);
	WT_VERBOSE_RET(session, fileops, "wt_dirlist of %s %s prefix %s",
	    dir, LF_ISSET(WT_DIRLIST_INCLUDE) ? "include" : "exclude",
	    prefix == NULL ? "all" : prefix);
fprintf(stderr, "wt_dirlist of %s %s prefix %s\n",
	    dir, LF_ISSET(WT_DIRLIST_INCLUDE) ? "include" : "exclude",
	    prefix == NULL ? "all" : prefix);

	WT_SYSCALL_RETRY(((dirp = opendir(dir)) == NULL ? 1 : 0), ret);
	*dirlist = NULL;
	for (dirsz = count = 0; (dp = readdir(dirp)) != NULL;) {
		/*
		 * Skip . and ..
		 */
		if (strcmp(dp->d_name, ".") == 0 ||
		    strcmp(dp->d_name, "..") == 0)
			continue;
		match = 0;
		if (prefix != NULL &&
		    ((LF_ISSET(WT_DIRLIST_INCLUDE) &&
		    WT_PREFIX_MATCH(dp->d_name, prefix)) ||
		    (LF_ISSET(WT_DIRLIST_EXCLUDE) &&
		    !WT_PREFIX_MATCH(dp->d_name, prefix))))
			match = 1;
		if (prefix == NULL || match) {
			/*
			 * We have a file name we want to return.
			 */
			count++;
			if (count > dirsz) {
				dirsz += WT_DIR_ENTRY;
				WT_ERR(__wt_realloc(session, &dirallocsz,
				    sizeof(entries[0]) * dirsz, &entries));
			}
			WT_ERR(__wt_strdup(
			    session, dp->d_name, &entries[count-1]));
		}
	}
	if (count > 0)
		*dirlist = entries;
	*countp = count;
err:
	(void)closedir(dirp);

	if (ret == 0)
		return (0);

	if (dirlist != NULL) {
		for (count = dirsz; count > 0; count--)
			__wt_free(session, entries[count]);
		__wt_free(session, entries);
	}
	WT_RET_MSG(session, ret, "dirlist %s prefix %s", dir, prefix);
}
