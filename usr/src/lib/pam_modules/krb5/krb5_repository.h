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
 * Copyright 2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_KRB5_REPOSITORY_H
#define	_KRB5_REPOSITORY_H

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#ifdef	__cplusplus
extern "C" {
#endif

#define	KRB5_REPOSITORY_NAME "KRB5"

enum krb5_rep_flags {
	SUNW_PAM_KRB5_ALREADY_AUTHENTICATED = 1,
	SUNW_PAM_KRB5_SECURE_CONNECTION = 2,
	SUNW_PAM_KRB5_LOCAL_CONNECTION = 3
};

typedef struct krb5_repository_data {
	char *principal;
	enum krb5_rep_flags flags;
}krb5_repository_data_t;

#ifdef	__cplusplus
}
#endif

#endif	/* _KRB5_REPOSITORY_H */