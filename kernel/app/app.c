/*
  +----------------------------------------------------------------------+
  | Xannotations---PHP Version 7                                         |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Josin                                                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "kernel/app/app.h"
#include "kernel/app/net.h"
#include "php_xannotation.h"
#include "ext/standard/info.h"
#include "kernel/app/dispatch.h"
#include "ext/standard/php_string.h"


/**
 * {{{
 * All arg_infos for the Xan\App class
 */
ZEND_BEGIN_ARG_INFO_EX(ARGINFO(xan_app_construct), 0, 0, 0)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(xan_app_run), 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(xan_app_get), 0, 0, 1)
    ZEND_ARG_INFO(0, objName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ARGINFO(xan_app_set), 0, 0, 2)
    ZEND_ARG_INFO(0, objName)
    ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

/*}}}*/

/**
 * {{{
 * proto App::__construct($options=[
 *  'urlPattern'        => 'AUTO',
 *  'defaultModule'     => 'index',
 *  'urlGetStr'         => '_xurl',
 *  'defaultController' => 'index',
 *  'defaultAction'     => 'index',
 *  'viewSuffix'        => 'html',
 *  'urlSuffix'         => 'html',
 *  'allowModules'      => 'index,home',
 *  'applicationDir'    => './../apps',
 *  'defaultNamespace'  => 'app'
 * ])
 */
XAN_METHOD(App, __construct)
{
    zval *options = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &options) == FAILURE)
    {
        return ;
    }

    if ( !options || ( options && (Z_TYPE_P(options) != IS_ARRAY) ) ||
       ( options && (Z_TYPE_P(options) == IS_ARRAY) && !zend_hash_num_elements(Z_ARRVAL_P(options)) ) )
    {
        return ;
    }

    /* Parsing the urlPattern */
    zval *url_pattern = STRING_FIND_P(options, "urlPattern");
    if ( url_pattern && !ZVAL_IS_NULL(url_pattern) && (Z_TYPE_P(url_pattern) == IS_STRING) )
    {
        if ( zend_string_equals_literal( zend_string_tolower(Z_STR_P(url_pattern)), "auto") )
        {
            XAN_G(url_pattern) = 0;
            zval *url_get_str = STRING_FIND_P(options, "urlGetStr");
            if ( url_get_str )
                ZVAL_COPY(&XAN_G(url_get_str), url_get_str);
        }
        else if ( zend_string_equals_literal( zend_string_tolower(Z_STR_P(url_pattern)), "get") )
        {
            XAN_G(url_pattern) = 1;
            zval *url_get_str = STRING_FIND_P(options, "urlGetStr");
            if ( url_get_str )
                ZVAL_COPY(&XAN_G(url_get_str), url_get_str);
        }
        else if ( zend_string_equals_literal( zend_string_tolower(Z_STR_P(url_pattern)), "path") )
        {
            XAN_G(url_pattern) = 2;
        }
    }

    /* Parsing the defaultModule */
    zval *default_module = STRING_FIND_P(options, "defaultModule");
    if ( default_module && !ZVAL_IS_NULL(default_module) && (Z_TYPE_P(default_module) == IS_STRING) )
    {
        if (Z_STRVAL_P(default_module)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(default_module), zend_string_tolower(Z_STR_P(default_module)));
        }
    }

    /* Parsing the applicationDir */
    zval *application_dir = STRING_FIND_P(options, "applicationDir");
    if ( application_dir && !ZVAL_IS_NULL(application_dir) && (Z_TYPE_P(application_dir) == IS_STRING) )
    {
        if (Z_STRVAL_P(application_dir)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(application_dir), zend_string_tolower(Z_STR_P(application_dir)));
        }
    }

    /* Parsing the defaultNamespace */
    zval *default_namespace = STRING_FIND_P(options, "defaultNamespace");
    if ( default_namespace && !ZVAL_IS_NULL(default_namespace) && (Z_TYPE_P(default_namespace) == IS_STRING) )
    {
        if (Z_STRVAL_P(default_namespace)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(default_namespace), zend_string_tolower(Z_STR_P(default_namespace)));
        }

        if (ZVAL_IS_NULL(&XAN_G(aliases)))
        {
            array_init(&XAN_G(aliases));
        }

        add_assoc_string(&XAN_G(aliases), Z_STRVAL_P(default_namespace), Z_STRVAL(XAN_G(application_dir)) );
    }

    /* Parsing the defaultController */
    zval *default_controller = STRING_FIND_P(options, "defaultController");
    if ( default_controller && !ZVAL_IS_NULL(default_controller) && (Z_TYPE_P(default_controller) == IS_STRING) )
    {
        if (Z_STRVAL_P(default_controller)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(default_controller), zend_string_tolower(Z_STR_P(default_controller)));
        }
    }

    /* Parsing the defaultAction */
    zval *default_action = STRING_FIND_P(options, "defaultAction");
    if ( default_action && !ZVAL_IS_NULL(default_action) && (Z_TYPE_P(default_action) == IS_STRING) )
    {
        if (Z_STRVAL_P(default_action)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(default_action), zend_string_tolower(Z_STR_P(default_action)));
        }
    }

    /* Parsing the viewSuffix */
    zval *view_suffix = STRING_FIND_P(options, "viewSuffix");
    if ( view_suffix && !ZVAL_IS_NULL(view_suffix) && (Z_TYPE_P(view_suffix) == IS_STRING) )
    {
        if (Z_STRVAL_P(view_suffix)[0] != '\0')
        {
            ZVAL_STR(&XAN_G(view_suffix), zend_string_tolower(Z_STR_P(view_suffix)));
        }
    }

    /* Parsing the urlSuffix */
    zval *url_suffix = STRING_FIND_P(options, "urlSuffix");
    if ( url_suffix && !ZVAL_IS_NULL(url_suffix) && (Z_TYPE_P(url_suffix) == IS_STRING) )
    {
        if (Z_STRVAL_P(url_suffix)[0] != '\0')
        {
            XAN_G(must_url_suffix) = 1;
            ZVAL_STR(&XAN_G(url_suffix), zend_string_tolower(Z_STR_P(url_suffix)));
        }
    }

    /* Parsing the allowModules */
    zval *allow_modules = STRING_FIND_P(options, "allowModules");
    if ( allow_modules && !ZVAL_IS_NULL(allow_modules) && (Z_TYPE_P(allow_modules) == IS_STRING) )
    {
        zend_array_destroy(Z_ARRVAL(XAN_G(allow_modules)));
        array_init(&XAN_G(allow_modules));
        php_explode(strpprintf(0, "%s", ","), Z_STR_P(allow_modules), &XAN_G(allow_modules), ZEND_LONG_MAX);
    }

    ZVAL_COPY(&XAN_G(app_di), getThis());
    zend_update_static_property(Z_OBJCE_P(getThis()), XAN_STRL("app"), getThis());
}/*}}}*/


/**
 * {{{
 * proto App::run()
 */
XAN_METHOD(App, run)
{
    zval *url = NULL;

    if ( XAN_G(url_pattern) == 0 )
    {
        /** 
         * auto mode, step:
         * 1, find the value from the get str
         * 2, find the value from $_SERVER['PATH_INFO']
         */
        if ( !ZVAL_IS_NULL(&XAN_G(url_get_str)) )
        {
            url = xan_get_get_vars(Z_STRVAL_P(&XAN_G(url_get_str)));
        }

        if ( !url || ZVAL_IS_NULL(url) )
        {
            url = xan_get_server_var("PATH_INFO");
        }
    }
    else if ( XAN_G(url_pattern) == 1 )
    {
        if (ZVAL_IS_NULL(&XAN_G(url_get_str)))
        {
            XAN_INFO(E_ERROR, "URL in `GET` mode must provide the `$_GET` value.");
        }
        url = xan_get_get_vars(Z_STRVAL_P(&XAN_G(url_get_str)));
    }
    else if ( XAN_G(url_pattern) == 2)
    {
        url = xan_get_server_var("PATH_INFO");
    }

    /* Dispatching the url to the dispatch engine to do the url routing job. */
    xan_dispatch_url( !url ? NULL : Z_STR_P(url) );

    /* After dispatching the url, you can do the job in the following place, if you like. */
    /* TODO... */
}/*}}}*/


/**
 * {{{
 * proto App::__get($name)
 */
XAN_METHOD(App, __get)
{
    zend_string *obj_name;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &obj_name) == FAILURE)
    {
        return ;
    }
    zval *obj = zend_hash_str_find(Z_ARRVAL(XAN_G(class_di)), ZSTR_VAL(obj_name), ZSTR_LEN(obj_name));
    if ( !obj || ZVAL_IS_NULL(obj) ) RETURN_NULL();
    RETURN_ZVAL(obj, 1, NULL);
}/*}}}*/

/**
 * {{{ 
 * proto App::__set($obj_name, $object)
 */
XAN_METHOD(App, __set)
{
    zval *object;
    zend_string *obj_name;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sz", &obj_name, &object) == FAILURE)
    {
        return ;
    }
    if (ZSTR_VAL(obj_name)[0] == '\0')
    {
        return ;
    }
    add_assoc_zval(&XAN_G(class_di), ZSTR_VAL(obj_name), object);
}/*}}}*/

/**
 * {{{
 * proto All function for Xan\App class
 */
XAN_FUNCTIONS(app)
    XAN_ME(App, __construct, arginfo_xan_app_construct, ZEND_ACC_PUBLIC)
    XAN_ME(App, run, arginfo_xan_app_run, ZEND_ACC_PUBLIC)
    XAN_ME(App, __set, arginfo_xan_app_set, ZEND_ACC_PUBLIC)
    XAN_ME(App, __get, arginfo_xan_app_get, ZEND_ACC_PUBLIC)
XAN_FUNCTIONS_END()
/*}}}*/

/**
 * {{{
 * proto app_init()
 */
XAN_INIT(app)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Xan", "Apps", app_functions);
    xan_app_ce = zend_register_internal_class(&ce);

    XAN_PR_NULL(xan_app_ce, "app", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
}/*}}}*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */


