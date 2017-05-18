<div align="left">
<a href="https://github.com/rryqszq4/ngx_php"><img width="320" src="https://raw.githubusercontent.com/rryqszq4/ngx_php/master/doc/ngx_php_logo.png"></a>
</div>

[![Build Status](https://travis-ci.org/rryqszq4/ngx_php.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php) 
[![Gitter](https://badges.gitter.im/rryqszq4/ngx_php.svg)](https://gitter.im/rryqszq4/ngx_php?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![GitHub release](https://img.shields.io/github/release/rryqszq4/ngx_php.svg)](https://github.com/rryqszq4/ngx_php/releases/latest)
[![license](https://img.shields.io/badge/license-BSD--2--Clause-blue.svg)](https://github.com/rryqszq4/ngx_php/blob/master/LICENSE)

[ngx_php](https://github.com/rryqszq4/ngx_php) - Embedded php script language for nginx-module. Another name is nginx-php5-module.   
QQ Group：558795330

Requirement
-----------
- PHP 5.5.* ~ PHP 5.6.*
- nginx-1.7.12 ~ nginx-1.11.8

Installation
-------
**build php**

```sh
$ wget 'http://php.net/distributions/php-5.6.30.tar.gz'
$ tar xf php-5.6.30.tar.gz
$ cd php-5.6.30

$ ./configure --prefix=/path/to/php \
$             --enable-maintainer-zts \
$             --enable-embed
$ make && make install
```

**build ngx_php**

```sh
$ git clone https://github.com/rryqszq4/ngx_php.git

$ wget 'http://nginx.org/download/nginx-1.7.12.tar.gz'
$ tar xf nginx-1.7.12.tar.gz
$ cd nginx-1.7.12

$ export PHP_BIN=/path/to/php/bin
$ export PHP_INC=/path/to/php/include/php
$ export PHP_LIB=/path/to/php/lib

$ ./configure --user=www --group=www \
$             --prefix=/path/to/nginx \
$             --with-ld-opt="-Wl,-rpath,$PHP_LIB" \
$             --add-module=/path/to/ngx_php/dev/ngx_devel_kit \
$             --add-module=/path/to/ngx_php
$ make && make install
```

Synopsis
--------

```nginx
user www www;
worker_processes  4;

events {
    worker_connections  1024;
}

http {
    include       mime.types;
    default_type  application/octet-stream;

    keepalive_timeout  65;
    
    client_max_body_size 10m;   
    client_body_buffer_size 4096k;

    php_ini_path /usr/local/php/etc/php.ini;

    server {
        listen       80;
        server_name  localhost;
    
        location /php {
            content_by_php '
                echo "hello ngx_php";
            ';
        }
    }
}
```

Test
----
Using the perl of [Test::Nginx](https://github.com/openresty/test-nginx) module to testing, searching and finding out problem in ngx_php. 

```sh
cd /path/to/ngx_php
export PATH=/path/to/nginx/sbin:$PATH
prove -r t
```
Test result:

```sh
t/001-hello.t ........... ok
t/008-error.t ........... ok
t/200-rewrite_by_php.t .. ok
t/202-access_by_php.t ... ok
t/204-content_by_php.t .. ok
All tests successful.
Files=5, Tests=22,  2 wallclock secs ( 0.04 usr  0.01 sys +  0.65 cusr  0.32 csys =  1.02 CPU)
Result: PASS
```

Directives
----------
* [php_ini_path](#php_ini_path)
* [init_by_php](#init_by_php)
* [init_by_php_file](#init_by_php_file)
* [rewrite_by_php](#rewrite_by_php)
* [rewrite_by_php_file](#rewrite_by_php_file)
* [access_by_php](#access_by_php)
* [access_by_php_file](#access_by_php_file)
* [content_by_php](#content_by_php)
* [content_by_php_file](#content_by_php_file)
* [log_by_php](#log_by_php)
* [log_by_php_file](#log_by_php_file)
* [set_by_php](#set_by_php)
* [set_run_by_php](#set_run_by_php)
* [set_by_php_file](#set_by_php_file)
* [set_run_by_php_file](#set_run_by_php_file)

php_ini_path
------------
* **syntax:** `php_ini_path &lt;php.ini file path&gt;`
* **context:** `http`
* **phase:** `loading-config`

Loading php configuration file in nginx configuration initialization.

```nginx
php_ini_path /usr/local/php/etc/php.ini;
```

init_by_php
-----------
* **syntax:** `init_by_php &lt;php script code&gt;`
* **context:** `http`
* **phase:** `loading-config`

In nginx configuration initialization or boot time, run some php scripts.

init_by_php_file
----------------
* **syntax:** `init_by_php_file &lt;php script file&gt;`
* **context:** `http`
* **phase:** `loading-config`

In nginx configuration initialization or boot time, run some php script file.

rewrite_by_php
--------------
* **syntax:** `rewrite_by_php &lt;php script code&gt;`
* **context:** `http, server, location, location if`
* **phase:** `rewrite`

Use php script redirect in nginx rewrite stage of.

```nginx
location /rewrite_by_php {
        rewrite_by_php "
            echo "rewrite_by_php";
            header('Location: http://www.baidu.com/');
        ";
    }
```

rewrite_by_php_file
-------------------
* **syntax:** `rewrite_by_php_file &lt;php script file&gt;`
* **context:** `http, server, location, location if`
* **phase:** `rewrite`

Use php script file, redirect in nginx rewrite stage of.

access_by_php
-------------
* **syntax:** `access_by_php &lt;php script code&gt;`
* **context:** `http, server, location, location if`
* **phase:** `access`

Nginx in the access phase, the php script determine access.

access_by_php_file
------------------
* **syntax:** `access_by_php_file &lt;php script file&gt;`
* **context:** `http, server, location, location if`
* **phase:** `access`

Nginx in the access phase, the php script file Analyzing access.

content_by_php
--------------
* **syntax:** `content_by_php &lt;php script code&gt;`
* **context:** `http, server, location, location if`
* **phase:** `content`

Most central command, run php script nginx stage of content.
```nginx
location /content_by_php {    
    content_by_php "
        header('Content-Type: text/html;charset=UTF-8');
    
        echo phpinfo();
    ";
        
}
```

content_by_php_file
-------------------
* **syntax:** `content_by_php_file &lt;php script file&gt;` 
* **context:** `http, server, location, location if`
* **phase:** `content`

Most central command, run php script file nginx stage of content.
```nginx
location /content_by_php_file {
        content_by_php_file /home/www/index.php;
}
```

log_by_php
----------
* **syntax:** `log_by_php &lt;php script code&gt;`
* **context:** `http, server, location, location if`
* **phase:** `log`

log_by_php_file
---------------
* **syntax:** `log_by_php_file &lt;php script file&gt;`
* **context:** `http, server, location, location if`
* **phase:** `log`

set_by_php
----------
* **syntax:** `set_by_php &lt;php script code&gt;`
* **context:** `server, server if, location, location if`
* **phase:** `content`

set_run_by_php
--------------
* **syntax:** `set_run_by_php &lt;php script code&gt;`
* **context:** `server, server if, location, location if`
* **phase:** `content`

set_by_php_file
---------------
* **syntax:** `set_by_php_file &lt;php script file&gt;`
* **context:** `server, server if, location, location if`
* **phase:** `content`

set_run_by_php_file
-------------------
* **syntax:** `set_run_by_php_file &lt;php script file&gt;`
* **context:** `server, server if, location, location if`
* **phase:** `content`


Nginx API for php
-----------------
* [ngx::_exit](#ngx_exit)
* [ngx::sleep](#ngxsleep)
* [ngx_generator::run]
* [ngx_php::main]
* [ngx_log::error](#ngx_logerror)

ngx::_exit
----------
* **syntax:** `ngx::_exit(int $status)`
* **context:** `content_by_php`

```php
echo "start\n";
ngx::_exit(200);
echo "end\n";
```

ngx::sleep
----------
* **syntax:** `ngx::sleep(int $time)`
* **context:** `content_by_php`

ngx_log::error
--------------
* **syntax:** `ngx_log::error(int level, string log)`
* **context:** `content_by_php`

Nginx log of level in php.
* NGX_LOG_STDERR
* NGX_LOG_EMERG
* NGX_LOG_ALERT
* NGX_LOG_CRIT
* NGX_LOG_ERR
* NGX_LOG_WARN
* NGX_LOG_NOTICE
* NGX_LOG_INFO
* NGX_LOG_DEBUG

```php
ngx_log::error(NGX_LOG_ERR, "test");

/*
 2016/10/06 22:10:19 [error] 51402#0: *1 test while reading response header from upstream, client: 192.168.80.1, 
 server: localhost, request: "GET /_mysql HTTP/1.1", upstream: "127.0.0.1:3306", host: "192.168.80.140"
*/
```

Question
--------
[issues #6](https://github.com/rryqszq4/ngx_php/issues/6) - Using in php-5.3.29, libxml2 2.7.6 not thread safety. Please disable xml in php install.
```sh
./configure --prefix=/usr/local/php5329 \
            --with-config-file-path=/usr/local/php5329/etc \
            --with-iconv=/usr/local/libiconv \
            --disable-xml \
            --disable-libxml \
            --disable-dom \
            --disable-simplexml \
            --disable-xmlreader \
            --disable-xmlwriter \
            --without-pear \
            --enable-maintainer-zts  \
            --enable-embed
```

Copyright and License
---------------------
Copyright (c) 2016-2017, rryqszq4 <rryqszq@gmail.com>  
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
