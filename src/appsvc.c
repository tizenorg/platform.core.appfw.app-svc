/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <aul.h>
#include <aul_svc.h>
#include <Ecore.h>

#ifndef WAYLAND
#include <Ecore_X.h>
#endif

#include "appsvc.h"

#ifndef SLPAPI
#define SLPAPI __attribute__ ((visibility("default")))
#endif

SLPAPI int appsvc_set_operation(bundle *b, const char *operation)
{
	return aul_svc_set_operation(b, operation);
}

SLPAPI int appsvc_set_uri(bundle *b, const char *uri)
{
	return aul_svc_set_uri(b, uri);
}

SLPAPI int appsvc_set_mime(bundle *b, const char *mime)
{
	return aul_svc_set_mime(b, mime);
}

SLPAPI int appsvc_add_data(bundle *b, const char *key, const char *val)
{
	return aul_svc_add_data(b, key, val);
}

SLPAPI int appsvc_add_data_array(bundle *b, const char *key,  const char **val_array, int len)
{
	return aul_svc_add_data_array(b, key, val_array, len);
}

SLPAPI int appsvc_set_pkgname(bundle *b, const char *pkg_name)
{
	return aul_svc_set_pkgname(b, pkg_name);
}

SLPAPI int appsvc_set_appid(bundle *b, const char *appid)
{
	return aul_svc_set_appid(b, appid);
}

SLPAPI int appsvc_set_category(bundle *b, const char *category)
{
	return aul_svc_set_category(b, category);
}

SLPAPI int appsvc_set_launch_mode(bundle *b, const char *mode)
{
	return aul_svc_set_launch_mode(b, mode);
}

SLPAPI int appsvc_run_service(bundle *b, int request_code, appsvc_res_fn cbfunc, void *data)
{
	return aul_svc_run_service(b, request_code, (aul_svc_res_fn)cbfunc, data);
}

SLPAPI int appsvc_usr_run_service(bundle *b, int request_code, appsvc_res_fn cbfunc, void *data, uid_t uid)
{
	return aul_svc_run_service_with_uid(b, request_code, (aul_svc_res_fn)cbfunc, data, uid);
}

SLPAPI int appsvc_get_list(bundle *b, appsvc_info_iter_fn iter_fn, void *data)
{
	return aul_svc_get_list(b, (aul_svc_info_iter_fn)iter_fn, data);
}

SLPAPI int appsvc_usr_get_list(bundle *b, appsvc_info_iter_fn iter_fn, void *data, uid_t uid)
{
	return aul_svc_get_list_with_uid(b, (aul_svc_info_iter_fn)iter_fn, data, uid);
}

SLPAPI int appsvc_get_all_defapps(appsvc_info_iter_fn iter_fn, void *data)
{
	return aul_svc_get_all_defapps((aul_svc_info_iter_fn)iter_fn, data);
}

SLPAPI const char *appsvc_get_operation(bundle *b)
{
	return aul_svc_get_operation(b);
}

SLPAPI const char *appsvc_get_uri(bundle *b)
{
	return aul_svc_get_uri(b);
}

SLPAPI const char *appsvc_get_mime(bundle *b)
{
	return aul_svc_get_mime(b);
}

SLPAPI const char *appsvc_get_data(bundle *b, const char *key)
{
	return aul_svc_get_data(b, key);
}

SLPAPI const char **appsvc_get_data_array(bundle *b, const char *key, int *len)
{
	return aul_svc_get_data_array(b, key, len);
}

SLPAPI const char *appsvc_get_pkgname(bundle *b)
{
	return aul_svc_get_pkgname(b);
}

SLPAPI const char *appsvc_get_appid(bundle *b)
{
	return aul_svc_get_appid(b);
}

SLPAPI const char *appsvc_get_category(bundle *b)
{
	return aul_svc_get_category(b);
}

SLPAPI const char *appsvc_get_launch_mode(bundle *b)
{
	return aul_svc_get_launch_mode(b);
}

SLPAPI int appsvc_create_result_bundle(bundle *inb, bundle **outb)
{
	return aul_svc_create_result_bundle(inb, outb);
}

SLPAPI int appsvc_send_result(bundle *b, appsvc_result_val result)
{
	return aul_svc_send_result(b, (aul_svc_result_val)result);
}

SLPAPI int appsvc_set_defapp(const char *op, const char *mime_type, const char *uri,
					const char *defapp, uid_t uid)
{
	return aul_svc_set_defapp_with_uid(op, mime_type, uri, defapp, uid);
}

SLPAPI int appsvc_unset_defapp(const char *defapp, uid_t uid)
{
	return aul_svc_unset_defapp_with_uid(defapp, uid);
}

SLPAPI int appsvc_unset_all_defapps()
{
	return aul_svc_unset_all_defapps();
}

SLPAPI int appsvc_is_defapp(const char *pkg_name, uid_t uid)
{
	return aul_svc_is_defapp_with_uid(pkg_name, uid);
}

SLPAPI int appsvc_data_is_array(bundle *b, const char *key)
{
	return aul_svc_data_is_array(b, key);
}

#ifndef WAYLAND
SLPAPI int appsvc_allow_transient_app(bundle *b, Ecore_X_Window id)
{
	return aul_svc_allow_transient_app(b, id);
}
#else
SLPAPI int appsvc_allow_transient_app(bundle *b, unsigned int id)
{
	return aul_svc_allow_transient_app(b, id);
}
#endif

#ifndef WAYLAND
SLPAPI int appsvc_request_transient_app(bundle *b, Ecore_X_Window callee_id, appsvc_host_res_fn cbfunc, void *data)
{
	return aul_svc_request_transient_app(b, callee_id, (aul_svc_host_res_fn)cbfunc, data);
}
#else
SLPAPI int appsvc_request_transient_app(bundle *b, unsigned int callee_id, appsvc_host_res_fn cbfunc, void *data)
{
	return aul_svc_request_transient_app(b, callee_id, (aul_svc_host_res_fn)cbfunc, data);
}
#endif

SLPAPI int appsvc_subapp_terminate_request_pid(int pid)
{
	return aul_svc_subapp_terminate_request_pid(pid);
}
