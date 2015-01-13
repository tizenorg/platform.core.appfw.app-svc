/*
 *  app-svc
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jayoun Lee <airjany@samsung.com>, Sewook Park <sewook7.park@samsung.com>, Jaeho Lee <jaeho81.lee@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>
#include <tzplatform_config.h>

#include "appsvc_db.h"
#include "internal.h"

#define SVC_DB_PATH	tzplatform_mkpath(TZ_SYS_DB, ".appsvc.db")
#define APP_INFO_DB_PATH	tzplatform_mkpath(TZ_SYS_DB, ".app_info.db")

#define QUERY_MAX_LEN	8192
#define URI_MAX_LEN	4096
#define BUF_MAX_LEN	1024
#define BUFSIZE	4096
#define ROOT_UID	0

#define APPSVC_COLLATION "appsvc_collation"

#define QUERY_CREATE_TABLE_APPSVC "create table if not exists appsvc " \
            "(operation text, " \
            "mime_type text, " \
            "uri text, " \
            "pkg_name text, " \
            "PRIMARY KEY(pkg_name)) "

static sqlite3 *svc_db = NULL;
static sqlite3 *app_info_db = NULL;

static int _mkdir(const char *dir, mode_t mode)
{
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;
	int ret;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if(tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for(p = tmp + 1; *p; p++) {
		if(*p == '/') {
			*p = 0;
			ret = mkdir(tmp, mode);
			if (ret && errno != EEXIST)
				return ret;
		*p = '/';
		}
	}
	return mkdir(tmp, mode);
}

static void _mkdir_for_user(const char* dir, uid_t uid, gid_t gid) {
  int ret = 0;

  ret = _mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH);
  if (ret == -1 && errno != EEXIST) {
    _E("FAIL : to create directory %s %d", dir, errno);
  } else if (getuid() == ROOT_UID) {
    ret = chown(dir, uid, gid);
    if (ret == -1)
      _E("FAIL : chown %s %d.%d, because %s", dir, uid, gid, strerror(errno));
  }
}

static char* getUserSvcDB(uid_t uid)
{
	const char *appsvc_db = NULL;
	const char *db_path = NULL;
	uid_t uid_caller = getuid();
	gid_t gid = ROOT_UID;

	if (uid == ROOT_UID) {
		_E("FAIL : Root is not allowed user! please fix it replacing with DEFAULT_USER");
		return NULL;
	}

	if (uid != tzplatform_getuid(TZ_SYS_GLOBALAPP_USER)) {
		tzplatform_set_user(uid);
		appsvc_db = tzplatform_mkpath(TZ_USER_DB, ".appsvc.db");
		db_path = tzplatform_getenv(TZ_USER_DB);
		gid = tzplatform_getgid(TZ_SYS_USER_GROUP);
		tzplatform_reset_user();
	} else {
		appsvc_db = tzplatform_mkpath(TZ_SYS_DB, ".appsvc.db");
		db_path = tzplatform_getenv(TZ_SYS_DB);
	}

	// just allow certain users to create missing directory.
	if (uid_caller == ROOT_UID || uid_caller == uid)
		_mkdir_for_user (db_path, uid, gid);

	return appsvc_db;
}


static char* getUserAppDB(uid_t uid)
{
	const char *app_info_db = NULL;
	const char *db_path = NULL;
	uid_t uid_caller = getuid();
	gid_t gid = ROOT_UID;

	if (uid == ROOT_UID) {
		_E("FAIL : Root is not allowed user! please fix it replacing with DEFAULT_USER");
		return NULL;
	}

	if (uid != tzplatform_getuid(TZ_SYS_GLOBALAPP_USER)) {
		tzplatform_set_user(uid);
		app_info_db = tzplatform_mkpath(TZ_USER_DB, ".app_info.db");
		db_path = tzplatform_getenv(TZ_USER_DB);
		gid = tzplatform_getgid(TZ_SYS_USER_GROUP);
		tzplatform_reset_user();
	} else {
		app_info_db = tzplatform_mkpath(TZ_SYS_DB, ".app_info.db");
		db_path = tzplatform_getenv(TZ_SYS_DB);
	}

	// just allow certain users to create the missing directory.
	if (uid_caller == ROOT_UID || uid_caller == uid)
		_mkdir_for_user (db_path, uid, gid);

	return app_info_db;
}
/**
 * db initialize
 */
static int __init(uid_t uid)
{
	int rc;

	if (svc_db) {
		_D("Already initialized\n");
		return 0;
	}

	rc = sqlite3_open(getUserSvcDB(uid), &svc_db);
	if(rc) {
		_E("Can't open database: %s", sqlite3_errmsg(svc_db));
		goto err;
	}

	// Enable persist journal mode
	rc = sqlite3_exec(svc_db, "PRAGMA journal_mode = PERSIST", NULL, NULL, NULL);
	if(SQLITE_OK!=rc){
		_D("Fail to change journal mode\n");
		goto err;
	}
	rc = sqlite3_exec(svc_db, QUERY_CREATE_TABLE_APPSVC, NULL, NULL, NULL);
	if(SQLITE_OK!=rc){
		_D("Fail to create tables\n");
		goto err;
	}

	return 0;
err:
	sqlite3_close(svc_db);
	return -1;
}

static int __collate_appsvc(void *ucol, int str1_len, const void *str1, int str2_len, const void *str2)
{
	char *saveptr1 = NULL;
	char *saveptr2 = NULL;
	char *dup_str1;
	char *dup_str2;
	char *token;
	char *in_op;
	char *in_uri;
	char *in_mime;
	char *op;
	char *uri;
	char *mime;
	int i;

	if(str1 == NULL || str2 == NULL)
		return -1;

	dup_str1 = strdup(str1);
	dup_str2 = strdup(str2);

	in_op = strtok_r(dup_str2, "|", &saveptr1);
	in_uri = strtok_r(NULL, "|", &saveptr1);
	in_mime = strtok_r(NULL, "|", &saveptr1);

	token = strtok_r(dup_str1, ";", &saveptr1);

	if(token == NULL) {
		free(dup_str1);
		free(dup_str2);
		return -1;
	}

	do {
		//_D("token : %s", token);
		op = strtok_r(token, "|", &saveptr2);
		uri = strtok_r(NULL, "|", &saveptr2);
		mime = strtok_r(NULL, "|", &saveptr2);

		if( (strcmp(op, in_op) == 0) && (strcmp(mime, in_mime) == 0) ) {
			_D("%s %s %s %s %s %s", op, in_op, mime, in_mime, uri, in_uri);
			if(strcmp(uri, in_uri) == 0) {
				free(dup_str1);
				free(dup_str2);
				return 0;
			} else {
				for(i=0; uri[i]!=0; i++) {
					if(uri[i] == '*') {
						uri[i] = 0;
						if(strstr(in_uri, uri)) {
							_D("in_uri : %s | uri : %s", in_uri, uri);
							free(dup_str1);
							free(dup_str2);
							return 0;
						}
					}
				}
			}
		}
	} while(token = strtok_r(NULL, ";", &saveptr1));

	free(dup_str1);
	free(dup_str2);

	return -1;
}

static int __init_app_info_db(uid_t uid)
{
	int rc;

	if (app_info_db) {
		_D("Already initialized\n");
		return 0;
	}

	rc = sqlite3_open(getUserAppDB(uid), &app_info_db);
	if(rc) {
		_E("Can't open database: %s", sqlite3_errmsg(app_info_db));
		goto err;
	}

	// Enable persist journal mode
	rc = sqlite3_exec(app_info_db, "PRAGMA journal_mode = PERSIST", NULL, NULL, NULL);
	if(SQLITE_OK!=rc){
		_D("Fail to change journal mode\n");
		goto err;
	}

	sqlite3_create_collation(app_info_db, APPSVC_COLLATION, SQLITE_UTF8, NULL,
                __collate_appsvc);

	return 0;
err:
	sqlite3_close(app_info_db);
	return -1;
}


static int __fini(void)
{
	if (svc_db) {
		sqlite3_close(svc_db);
		svc_db = NULL;
	}
	return 0;
}


int _svc_db_add_app(const char *op, const char *mime_type, const char *uri, const char *pkg_name, uid_t uid)
{
	char m[BUF_MAX_LEN];
	char u[URI_MAX_LEN];
	char query[QUERY_MAX_LEN];
	char* error_message = NULL;

	if(__init(uid)<0)
		return -1;

	if(op == NULL )
		return -1;

	if(mime_type==NULL)
		strncpy(m,"NULL",BUF_MAX_LEN-1);
	else
		strncpy(m,mime_type,BUF_MAX_LEN-1);

	if(uri==NULL)
		strncpy(u,"NULL",URI_MAX_LEN-1);
	else
		strncpy(u,uri,URI_MAX_LEN-1);

	snprintf(query, QUERY_MAX_LEN, "insert into appsvc( operation, mime_type, uri, pkg_name) \
		values('%s','%s','%s','%s')",op,m,u,pkg_name);

	if (SQLITE_OK != sqlite3_exec(svc_db, query, NULL, NULL, &error_message))
	{
	 	_E("Don't execute query = %s, error message = %s\n", query, error_message);
		return -1;
	}

	__fini();
	return 0;
}

int _svc_db_delete_with_pkgname(const char *pkg_name, uid_t uid)
{
	char query[QUERY_MAX_LEN];
	char* error_message = NULL;

	if(pkg_name == NULL) {
		_E("Invalid argument: data to delete is NULL\n");
		return -1;
	}

	if(__init(uid)<0)
		return -1;

	snprintf(query, QUERY_MAX_LEN, "delete from appsvc where pkg_name = '%s';", pkg_name);

	if (SQLITE_OK != sqlite3_exec(svc_db, query, NULL, NULL, &error_message))
	{
	 	_E("Don't execute query = %s, error message = %s\n", query, error_message);
		return -1;
	}

	__fini();

	return 0;
}

int _svc_db_is_defapp(const char *pkg_name, uid_t uid)
{
	char query[QUERY_MAX_LEN];
	sqlite3_stmt *stmt;
	int cnt = 0;
	int ret = -1;

	if(pkg_name == NULL) {
		_E("Invalid argument: data to delete is NULL\n");
		return 0;
	}

	if(__init(uid)<0)
		return 0;

	snprintf(query, QUERY_MAX_LEN,
		"select count(*) from appsvc where pkg_name = '%s';", pkg_name);

	ret = sqlite3_prepare(svc_db, query, sizeof(query), &stmt, NULL);
	if (ret != SQLITE_OK) {
		return -1;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		cnt = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);

	__fini();

	if(cnt < 1) return 0;

	return 1;
}

char* _svc_db_get_app(const char *op, const char *mime_type, const char *uri, uid_t uid)
{
	char m[BUF_MAX_LEN];
	char u[URI_MAX_LEN];
	char query[QUERY_MAX_LEN];
	sqlite3_stmt* stmt;
	int ret;
	char* pkgname;
	char* ret_val = NULL;

	if(op == NULL )
		return NULL;

	if(mime_type==NULL)
		strncpy(m,"NULL",BUF_MAX_LEN-1);
	else
		strncpy(m,mime_type,BUF_MAX_LEN-1);

	if(uri==NULL)
		strncpy(u,"NULL",URI_MAX_LEN-1);
	else
		strncpy(u,uri,URI_MAX_LEN-1);

//	if(doubt_sql_injection(mime_type))
//		return NULL;

	if(__init(uid) < 0)
		return NULL;


	snprintf(query, QUERY_MAX_LEN, "select pkg_name from appsvc where operation='%s' and mime_type='%s' and uri='%s'",\
				op,m,u);

	_D("query : %s\n",query);

	ret = sqlite3_prepare(svc_db, query, strlen(query), &stmt, NULL);

	if ( ret != SQLITE_OK) {
		_E("prepare error(%d)\n", ret);
		goto db_fini;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		_D("no result");
		goto stmt_finialize;
	}

	pkgname = (char*) sqlite3_column_text(stmt, 0);
	if(pkgname) {
		ret_val = malloc(BUF_MAX_LEN);
		strncpy(ret_val, (const char *)sqlite3_column_text(stmt, 0),BUF_MAX_LEN-1);
	}

	_D("pkgname : %s\n",pkgname);

stmt_finialize :
	ret = sqlite3_finalize(stmt);
	if ( ret != SQLITE_OK) {
		_D("finalize error(%d)", ret);
	}

db_fini :
	__fini();

	return ret_val;
}

int _svc_db_get_list_with_collation(char *op, char *uri, char *mime, GSList **pkg_list, uid_t uid)
{
	char query[QUERY_MAX_LEN];
	sqlite3_stmt* stmt;
	int ret;
	GSList *iter = NULL;
	char *str = NULL;
	char *pkgname = NULL;
	int found;

	if(__init_app_info_db(uid)<0)
		return 0;

	snprintf(query, QUERY_MAX_LEN, "select package from app_info where x_slp_svc like '%%%s|%s|%s%%'", op, uri ? uri : "NULL", mime);
	_D("query : %s\n",query);

	ret = sqlite3_prepare(app_info_db, query, strlen(query), &stmt, NULL);

	if ( ret != SQLITE_OK) {
		_E("prepare error\n");
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		str = sqlite3_column_text(stmt, 0);
		found = 0;
		for (iter = *pkg_list; iter != NULL; iter = g_slist_next(iter)) {
			pkgname = (char *)iter->data;
			if (strncmp(str,pkgname, MAX_PACKAGE_STR_SIZE-1) == 0) {
				found = 1;
				break;
			}
		}
		if(found == 0) {
			pkgname = strdup(str);
			*pkg_list = g_slist_append(*pkg_list, (void *)pkgname);
			_D("%s is added",pkgname);
		}
	}

	ret = sqlite3_finalize(stmt);

	return 0;
}


