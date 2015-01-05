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



#ifndef __APP_SVC_DB_H__
#define __APP_SVC_DB_H__

#include <sqlite3.h>
#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

int _svc_db_add_app(const char *op, const char *mime_type, const char *uri, const char *pkg_name, uid_t uid);
int _svc_db_delete_with_pkgname(const char *pkg_name, uid_t uid);
char* _svc_db_get_app(const char *op, const char *mime_type, const char *uri, uid_t uid);
int _svc_db_is_defapp(const char *pkg_name, uid_t uid);
int _svc_db_get_list_with_collation(char *op, char *uri, char *mime, GSList **pkg_list, uid_t uid);



#ifdef __cplusplus
}
#endif

#endif		/* __APP_SVC_DB_H__ */

