/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * This file contains public functions for managing DHCP network
 * containers.  For the semantics of these functions, please see the
 * Enterprise DHCP Architecture Document.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include <dhcp_svc_public.h>
#include <dirent.h>
#include <libgen.h>
#include <libinetutil.h>
#include <sys/mman.h>

#include "dhcp_network.h"
#include "util.h"

static void net2path(char *, size_t, const char *, ipaddr_t, const char *);
static boolean_t record_match(char *[], dn_rec_t *, const dn_rec_t *, uint_t);
static int write_rec(int, dn_rec_t *, off_t);

/* ARGSUSED */
int
open_dn(void **handlep, const char *location, uint_t flags,
    const struct in_addr *netp, const struct in_addr *maskp)
{
	char		connet[INET_ADDRSTRLEN];
	char		dnpath[MAXPATHLEN];
	unsigned int	conver;
	dn_handle_t	*dhp;
	FILE		*fp;
	int		retval;
	int		i, nelems;
	char		nl;
	struct in_addr	net_nbo;
	int		fd;

	dhp = malloc(sizeof (dn_handle_t));
	if (dhp == NULL)
		return (DSVC_NO_MEMORY);

	dhp->dh_net = netp->s_addr;
	dhp->dh_oflags = flags;
	(void) strlcpy(dhp->dh_location, location, MAXPATHLEN);

	net2path(dnpath, MAXPATHLEN, location, netp->s_addr, "");
	retval = open_file(dnpath, flags, &fd);
	if (retval != DSVC_SUCCESS) {
		free(dhp);
		return (retval);
	}

	fp = fdopen(fd, flags & DSVC_WRITE ? "r+" : "r");
	if (fp == NULL) {
		(void) close(fd);
		free(dhp);
		return (DSVC_INTERNAL);
	}

	if (flags & DSVC_CREATE) {
		/*
		 * We just created the per-network container; put the
		 * header on for future use...
		 */
		net_nbo.s_addr = htonl(netp->s_addr);
		(void) inet_ntop(AF_INET, &net_nbo, connet, INET_ADDRSTRLEN);

		for (i = 0; connet[i] != '\0'; i++)
			if (connet[i] == '.')
				connet[i] = '_';

		retval = fprintf(fp, "# SUNWfiles%u_%s\n", DSVC_CONVER, connet);
		if (retval < 0 || fflush(fp) == EOF) {
			(void) fclose(fp);
			(void) free(dhp);
			return (DSVC_INTERNAL);
		}

		(void) fprintf(fp, "#\n# Do NOT edit this file by hand -- use");
		(void) fprintf(fp, " pntadm(1M) or dhcpmgr(1M) instead\n#\n");
	} else {
		/*
		 * Container already exists; sanity check against the
		 * header that's on-disk.
		 */
		nelems = fscanf(fp, "#%*1[ ]SUNWfiles%u_%15s%c", &conver,
		    connet, &nl);

		for (i = 0; connet[i] != '\0'; i++)
			if (connet[i] == '_')
				connet[i] = '.';

		if (nelems != 3 || inet_addr(connet) != htonl(netp->s_addr) ||
		    conver != DSVC_CONVER || nl != '\n') {
			(void) fclose(fp);
			(void) free(dhp);
			return (DSVC_INTERNAL);
		}
	}

	(void) fclose(fp);
	*handlep = dhp;
	return (DSVC_SUCCESS);
}

int
close_dn(void **handlep)
{
	free(*handlep);
	return (DSVC_SUCCESS);
}

int
remove_dn(const char *dir, const struct in_addr *netp)
{
	char dnpath[MAXPATHLEN];

	net2path(dnpath, MAXPATHLEN, dir, netp->s_addr, "");
	if (unlink(dnpath) == -1)
		return (syserr_to_dsvcerr(errno));

	return (DSVC_SUCCESS);
}

/*
 * Internal version lookup routine used by both lookup_dn() and
 * update_dn(); same semantics as lookup_dn() except that the `partial'
 * argument has been generalized into a `flags' field.
 */
static int
find_dn(int fd, uint_t flags, uint_t query, int count, const dn_rec_t *targetp,
    dn_rec_list_t **recordsp, uint_t *nrecordsp)
{
	int		retval = DSVC_SUCCESS;
	char		*fields[DNF_FIELDS];
	uint_t		nrecords;
	dn_rec_t	dn, *recordp;
	dn_rec_list_t	*records, *new_records;
	unsigned int	nfields;
	struct stat	st;
	struct in_addr	cip_nbo;
	char		*ent0, *ent, *entend;
	char		cip[INET_ADDRSTRLEN + 2];

	/*
	 * Page the whole container into memory via mmap() so we can scan it
	 * quickly; map it MAP_PRIVATE so that we can change newlines to
	 * NULs without changing the actual container itself.
	 */
	if (fstat(fd, &st) == -1 || st.st_size < 1)
		return (DSVC_INTERNAL);

	ent0 = mmap(0, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ent0 == MAP_FAILED)
		return (DSVC_INTERNAL);

	/*
	 * NUL-terminate the last byte (which should be a newline) so that
	 * we can safely use string functions on the mapped container.
	 */
	ent0[st.st_size - 1] = '\0';

	/*
	 * If we're searching by client IP address, then build a target
	 * string we can use to find it quickly.
	 */
	if (DSVC_QISEQ(query, DN_QCIP)) {
		cip[0] = '\n';
		cip_nbo.s_addr = htonl(targetp->dn_cip.s_addr);
		(void) inet_ntop(AF_INET, &cip_nbo, cip + 1, INET_ADDRSTRLEN);
		(void) strlcat(cip, "|", sizeof (cip));
	}

	records = NULL;
	ent = ent0;
	for (nrecords = 0; count < 0 || nrecords < count; ent = entend + 1) {
		/*
		 * Bail if we've reached the end of the container.
		 */
		if (ent - ent0 >= st.st_size)
			break;

		/*
		 * If we're searching by client IP address, locate it
		 * quickly using strstr(3C); if we can't find it by this
		 * technique then it's not in the container.
		 */
		if (DSVC_QISEQ(query, DN_QCIP)) {
			/*
			 * If we've already found the DN_QCIP record, bail.
			 */
			if (nrecords > 0)
				break;

			ent = strstr(ent, cip);
			if (ent == NULL)
				break;
			ent++;
		}

		/*
		 * Find the end of the record and change it a NUL byte so
		 * that it is interpreted correctly with field_split() and
		 * record_match() below.  If we can't find a trailing
		 * newline, then it must be the last record (whose newline
		 * we already changed to a NUL above).
		 */
		entend = strchr(ent, '\n');
		if (entend != NULL)
			*entend = '\0';
		else
			entend = &ent0[st.st_size - 1];

		/*
		 * Skip pure comment lines; for now this just skips the
		 * header information at the top of the container.
		 */
		if (ent[0] == DNF_COMMENT_CHAR)
			continue;

		/*
		 * Split the buffer up into DNF_FIELDS fields.
		 */
		nfields = field_split(ent, DNF_FIELDS, fields, "|");
		if (nfields < DNF_FIELDS)
			continue;

		/*
		 * See if we've got a match, filling in dnf.dnf_rec as
		 * we go.  If record_match() succeeds, dnf.dnf_rec will
		 * be completely filled in.
		 */
		if (!record_match(fields, &dn, targetp, query))
			continue;

		/*
		 * Caller just wants a count of the number of matching
		 * records, not the records themselves; continue.
		 */
		if (recordsp == NULL) {
			nrecords++;
			continue;
		}

		/*
		 * Allocate record; if FIND_POSITION flag is set, then
		 * we need to allocate an extended (dn_recpos_t) record.
		 */
		if (flags & FIND_POSITION)
			recordp = malloc(sizeof (dn_recpos_t));
		else
			recordp = malloc(sizeof (dn_rec_t));

		if (recordp == NULL) {
			if ((flags & FIND_PARTIAL) == 0)
				retval = DSVC_NO_MEMORY;
			break;
		}

		/*
		 * Fill in record; do a structure copy from our automatic
		 * dn.  If FIND_POSITION flag is on, pass back additional
		 * position information.
		 */
		*recordp = dn;
		if (flags & FIND_POSITION) {
			((dn_recpos_t *)recordp)->dnp_off = ent - ent0;
			((dn_recpos_t *)recordp)->dnp_size = entend - ent + 1;
		}

		/*
		 * Chuck the record on the list; up the counter.
		 */
		new_records = add_dnrec_to_list(recordp, records);
		if (new_records == NULL) {
			free(recordp);
			if ((flags & FIND_PARTIAL) == 0)
				retval = DSVC_NO_MEMORY;
			break;
		}

		records = new_records;
		nrecords++;
	}

	(void) munmap(ent0, st.st_size);

	if (retval == DSVC_SUCCESS) {
		*nrecordsp = nrecords;
		if (recordsp != NULL)
			*recordsp = records;
		return (DSVC_SUCCESS);
	}

	if (records != NULL)
		free_dnrec_list(records);

	return (retval);
}

int
lookup_dn(void *handle, boolean_t partial, uint_t query, int count,
    const dn_rec_t *targetp, dn_rec_list_t **recordsp, uint_t *nrecordsp)
{
	int		retval;
	char		dnpath[MAXPATHLEN];
	int		fd;
	dn_handle_t	*dhp = (dn_handle_t *)handle;

	if ((dhp->dh_oflags & DSVC_READ) == 0)
		return (DSVC_ACCESS);

	net2path(dnpath, MAXPATHLEN, dhp->dh_location, dhp->dh_net, "");
	fd = open(dnpath, O_RDONLY);
	if (fd == -1)
		return (syserr_to_dsvcerr(errno));

	retval = find_dn(fd, partial ? FIND_PARTIAL : 0, query, count, targetp,
	    recordsp, nrecordsp);

	(void) close(fd);
	return (retval);
}

/*
 * Compares the fields in fields[] agains the fields in target `targetp',
 * using `query' to decide what fields to compare.  Returns B_TRUE if `dnp'
 * matches `targetp', B_FALSE if not.  On success, `dnp' is completely
 * filled in.
 */
static boolean_t
record_match(char *fields[], dn_rec_t *dnp, const dn_rec_t *targetp,
    uint_t query)
{
	unsigned int	qflags[] = { DN_QFDYNAMIC, DN_QFAUTOMATIC, DN_QFMANUAL,
				    DN_QFUNUSABLE, DN_QFBOOTP_ONLY };
	unsigned int	flags[]  = { DN_FDYNAMIC, DN_FAUTOMATIC, DN_FMANUAL,
				    DN_FUNUSABLE, DN_FBOOTP_ONLY };
	unsigned int	i;
	uint_t		dn_cid_len;

	dnp->dn_cip.s_addr = ntohl(inet_addr(fields[DNF_CIP]));
	if (DSVC_QISEQ(query, DN_QCIP) &&
	    dnp->dn_cip.s_addr != targetp->dn_cip.s_addr)
		return (B_FALSE);
	if (DSVC_QISNEQ(query, DN_QCIP) &&
	    dnp->dn_cip.s_addr == targetp->dn_cip.s_addr)
		return (B_FALSE);

	dnp->dn_lease = atoi(fields[DNF_LEASE]);
	if (DSVC_QISEQ(query, DN_QLEASE) && targetp->dn_lease != dnp->dn_lease)
		return (B_FALSE);
	if (DSVC_QISNEQ(query, DN_QLEASE) && targetp->dn_lease == dnp->dn_lease)
		return (B_FALSE);

	/*
	 * We use dn_cid_len since dnp->dn_cid_len is of type uchar_t but
	 * hexascii_to_octet() expects an uint_t *
	 */
	dn_cid_len = DN_MAX_CID_LEN;
	if (hexascii_to_octet(fields[DNF_CID], strlen(fields[DNF_CID]),
	    dnp->dn_cid, &dn_cid_len) != 0)
		return (B_FALSE);

	dnp->dn_cid_len = dn_cid_len;
	if (DSVC_QISEQ(query, DN_QCID) &&
	    (dnp->dn_cid_len != targetp->dn_cid_len ||
	    (memcmp(dnp->dn_cid, targetp->dn_cid, dnp->dn_cid_len) != 0)))
		return (B_FALSE);
	if (DSVC_QISNEQ(query, DN_QCID) &&
	    (dnp->dn_cid_len == targetp->dn_cid_len &&
	    (memcmp(dnp->dn_cid, targetp->dn_cid, dnp->dn_cid_len) == 0)))
		return (B_FALSE);

	dnp->dn_sip.s_addr = ntohl(inet_addr(fields[DNF_SIP]));
	if (DSVC_QISEQ(query, DN_QSIP) &&
	    dnp->dn_sip.s_addr != targetp->dn_sip.s_addr)
		return (B_FALSE);
	if (DSVC_QISNEQ(query, DN_QSIP) &&
	    dnp->dn_sip.s_addr == targetp->dn_sip.s_addr)
		return (B_FALSE);

	unescape('|', fields[DNF_MACRO], dnp->dn_macro, sizeof (dnp->dn_macro));
	if (DSVC_QISEQ(query, DN_QMACRO) &&
	    strcmp(targetp->dn_macro, dnp->dn_macro) != 0)
		return (B_FALSE);
	if (DSVC_QISNEQ(query, DN_QMACRO) &&
	    strcmp(targetp->dn_macro, dnp->dn_macro) == 0)
		return (B_FALSE);

	dnp->dn_flags = atoi(fields[DNF_FLAGS]);
	for (i = 0; i < sizeof (qflags) / sizeof (unsigned int); i++) {
		if (DSVC_QISEQ(query, qflags[i]) &&
		    (dnp->dn_flags & flags[i]) !=
		    (targetp->dn_flags & flags[i]))
			return (B_FALSE);
		if (DSVC_QISNEQ(query, qflags[i]) &&
		    (dnp->dn_flags & flags[i]) ==
		    (targetp->dn_flags & flags[i]))
			return (B_FALSE);
	}

	dnp->dn_sig = atoll(fields[DNF_SIG]);
	unescape('|', fields[DNF_COMMENT], dnp->dn_comment,
	    sizeof (dnp->dn_comment));

	return (B_TRUE);
}

/*
 * Internal dhcp_network record update routine, used to factor out the
 * common code between add_dn(), delete_dn(), and modify_dn().  If
 * `origp' is NULL, then act like add_dn(); if `newp' is NULL, then
 * act like delete_dn(); otherwise act like modify_dn().
 */
static int
update_dn(const dn_handle_t *dhp, const dn_rec_t *origp, dn_rec_t *newp)
{
	char		dnpath[MAXPATHLEN], newpath[MAXPATHLEN];
	int		retval = DSVC_SUCCESS;
	off_t		recoff, recnext;
	dn_rec_list_t	*reclist;
	int		fd, newfd;
	uint_t		found;
	int		query;
	struct stat	st;


	if ((dhp->dh_oflags & DSVC_WRITE) == 0)
		return (DSVC_ACCESS);

	/*
	 * Open the container to update and a new container file which we
	 * will store the updated version of the container in.  When the
	 * update is done, rename the new file to be the real container.
	 */
	net2path(dnpath, MAXPATHLEN, dhp->dh_location, dhp->dh_net, "");
	fd = open(dnpath, O_RDONLY);
	if (fd == -1)
		return (syserr_to_dsvcerr(errno));

	net2path(newpath, MAXPATHLEN, dhp->dh_location, dhp->dh_net, ".new");
	newfd = open(newpath, O_CREAT|O_TRUNC|O_WRONLY, 0644);
	if (newfd == -1) {
		(void) close(fd);
		return (syserr_to_dsvcerr(errno));
	}

	DSVC_QINIT(query);
	DSVC_QEQ(query, DN_QCIP);

	/*
	 * If we're changing the key for this record, make sure the key
	 * we're changing to doesn't already exist.
	 */
	if (origp != NULL && newp != NULL) {
		if (origp->dn_cip.s_addr != newp->dn_cip.s_addr) {
			retval = find_dn(fd, 0, query, 1, newp, NULL, &found);
			if (retval != DSVC_SUCCESS)
				goto out;
			if (found != 0) {
				retval = DSVC_EXISTS;
				goto out;
			}
		}
	}

	/*
	 * If we're adding a new record, make sure the record doesn't
	 * already exist.
	 */
	if (newp != NULL && origp == NULL) {
		retval = find_dn(fd, 0, query, 1, newp, NULL, &found);
		if (retval != DSVC_SUCCESS)
			goto out;
		if (found != 0) {
			retval = DSVC_EXISTS;
			goto out;
		}
	}

	/*
	 * If we're deleting or modifying record, make sure the record
	 * still exists and that our copy isn't stale.  Note that we don't
	 * check signatures if we're deleting the record and origp->dn_sig
	 * is zero, so that records that weren't looked up can be deleted.
	 */
	if (origp != NULL) {
		retval = find_dn(fd, FIND_POSITION, query, 1, origp, &reclist,
		    &found);
		if (retval != DSVC_SUCCESS)
			goto out;
		if (found == 0) {
			retval = DSVC_NOENT;
			goto out;
		}

		if (reclist->dnl_rec->dn_sig != origp->dn_sig) {
			if (newp != NULL || origp->dn_sig != 0) {
				free_dnrec_list(reclist);
				retval = DSVC_COLLISION;
				goto out;
			}
		}

		/*
		 * Note the offset of the record we're modifying or deleting
		 * for use down below.
		 */
		recoff  = ((dn_recpos_t *)reclist->dnl_rec)->dnp_off;
		recnext = recoff + ((dn_recpos_t *)reclist->dnl_rec)->dnp_size;

		free_dnrec_list(reclist);
	} else {
		/*
		 * No record to modify or delete, so set `recoff' and
		 * `recnext' appropriately.
		 */
		recoff = 0;
		recnext = 0;
	}

	/*
	 * Make a new copy of the container.  If we're deleting or
	 * modifying a record, don't copy that record to the new container.
	 */
	if (fstat(fd, &st) == -1) {
		retval = DSVC_INTERNAL;
		goto out;
	}

	retval = copy_range(fd, 0, newfd, 0, recoff);
	if (retval != DSVC_SUCCESS)
		goto out;

	retval = copy_range(fd, recnext, newfd, recoff, st.st_size - recnext);
	if (retval != DSVC_SUCCESS)
		goto out;

	/*
	 * If there's a new/modified record, append it to the new container.
	 */
	if (newp != NULL) {
		if (origp == NULL)
			newp->dn_sig = gensig();
		else
			newp->dn_sig = origp->dn_sig + 1;

		retval = write_rec(newfd, newp, recoff + st.st_size - recnext);
		if (retval != DSVC_SUCCESS)
			goto out;
	}

	/*
	 * Note: we close these descriptors before the rename(2) (rather
	 * than just having the `out:' label clean them up) to save NFS
	 * some work (otherwise, NFS has to save `dnpath' to an alternate
	 * name since its vnode would still be active).
	 */
	(void) close(fd);
	(void) close(newfd);

	if (rename(newpath, dnpath) == -1)
		retval = syserr_to_dsvcerr(errno);

	return (retval);
out:
	(void) close(fd);
	(void) close(newfd);
	(void) unlink(newpath);
	return (retval);
}

int
add_dn(void *handle, dn_rec_t *addp)
{
	return (update_dn((dn_handle_t *)handle, NULL, addp));
}

int
modify_dn(void *handle, const dn_rec_t *origp, dn_rec_t *newp)
{
	return (update_dn((dn_handle_t *)handle, origp, newp));
}

int
delete_dn(void *handle, const dn_rec_t *delp)
{
	return (update_dn((dn_handle_t *)handle, delp, NULL));
}

int
list_dn(const char *location, char ***listppp, uint_t *countp)
{
	char		ipaddr[INET_ADDRSTRLEN];
	struct dirent	*result;
	DIR		*dirp;
	unsigned int	i, count = 0;
	char		*re, **new_listpp, **listpp = NULL;
	char		conver[4];
	int		error;

	dirp = opendir(location);
	if (dirp == NULL) {
		switch (errno) {
		case EACCES:
		case EPERM:
			return (DSVC_ACCESS);
		case ENOENT:
			return (DSVC_NO_LOCATION);
		default:
			break;
		}
		return (DSVC_INTERNAL);
	}

	/*
	 * Compile a regular expression matching "SUNWfilesX_" (where X is
	 * a container version number) followed by an IP address (roughly
	 * speaking).  Note that the $N constructions allow us to get the
	 * container version and IP address when calling regex(3C).
	 */
	re = regcmp("^SUNWfiles([0-9]{1,3})$0_"
	    "(([0-9]{1,3}_){3}[0-9]{1,3})$1$", (char *)0);
	if (re == NULL)
		return (DSVC_NO_MEMORY);

	while ((result = readdir(dirp)) != NULL) {
		if (regex(re, result->d_name, conver, ipaddr) != NULL) {
			if (atoi(conver) != DSVC_CONVER)
				continue;

			for (i = 0; ipaddr[i] != '\0'; i++)
				if (ipaddr[i] == '_')
					ipaddr[i] = '.';

			new_listpp = realloc(listpp,
			    (sizeof (char **)) * (count + 1));
			if (new_listpp == NULL) {
				error = DSVC_NO_MEMORY;
				goto fail;
			}
			listpp = new_listpp;
			listpp[count] = strdup(ipaddr);
			if (listpp[count] == NULL) {
				error = DSVC_NO_MEMORY;
				goto fail;
			}
			count++;
		}
	}
	free(re);
	(void) closedir(dirp);

	*countp = count;
	*listppp = listpp;
	return (DSVC_SUCCESS);

fail:
	free(re);
	(void) closedir(dirp);

	for (i = 0; i < count; i++)
		free(listpp[i]);
	free(listpp);
	return (error);
}

/*
 * Given a buffer `path' of `pathlen' bytes, fill it in with a path to the
 * DHCP Network table for IP network `ip' located in directory `dir' with a
 * suffix of `suffix'.
 */
static void
net2path(char *path, size_t pathlen, const char *dir, ipaddr_t ip,
    const char *suffix)
{
	(void) snprintf(path, pathlen, "%s/SUNWfiles%u_%d_%d_%d_%d%s", dir,
	    DSVC_CONVER, ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff,
	    ip & 0xff, suffix);
}

/*
 * Write the dn_rec_t `recp' into the open container `fd' at offset
 * `recoff'.  Returns DSVC_* error code.
 */
static int
write_rec(int fd, dn_rec_t *recp, off_t recoff)
{
	char		entbuf[1024], *ent = entbuf;
	size_t		entsize = sizeof (entbuf);
	int		entlen;
	dn_filerec_t	dnf;
	struct in_addr	nip;
	unsigned int	cid_len = sizeof (dnf.dnf_cid);

	/*
	 * Copy data into a dn_filerec_t, since that's what we can
	 * actually put on disk.
	 */
	if (octet_to_hexascii(recp->dn_cid, recp->dn_cid_len, dnf.dnf_cid,
	    &cid_len) != 0)
		return (DSVC_INTERNAL);

	nip.s_addr = htonl(recp->dn_cip.s_addr);
	(void) inet_ntop(AF_INET, &nip, dnf.dnf_cip, sizeof (dnf.dnf_cip));
	nip.s_addr = htonl(recp->dn_sip.s_addr);
	(void) inet_ntop(AF_INET, &nip, dnf.dnf_sip, sizeof (dnf.dnf_cip));

	dnf.dnf_sig	= recp->dn_sig;
	dnf.dnf_flags	= recp->dn_flags;
	dnf.dnf_lease	= recp->dn_lease;

	escape('|', recp->dn_macro, dnf.dnf_macro, sizeof (dnf.dnf_macro));
	escape('|', recp->dn_comment, dnf.dnf_comment,
	    sizeof (dnf.dnf_comment));
again:
	entlen = snprintf(ent, entsize, "%s|%s|%02hu|%s|%u|%llu|%s|%s\n",
	    dnf.dnf_cip, dnf.dnf_cid, dnf.dnf_flags, dnf.dnf_sip,
	    dnf.dnf_lease, dnf.dnf_sig, dnf.dnf_macro, dnf.dnf_comment);
	if (entlen == -1)
		return (syserr_to_dsvcerr(errno));

	if (entlen > entsize) {
		entsize = entlen;
		ent = alloca(entlen);
		goto again;
	}

	if (pnwrite(fd, ent, entlen, recoff) == -1)
		return (syserr_to_dsvcerr(errno));

	return (DSVC_SUCCESS);
}
